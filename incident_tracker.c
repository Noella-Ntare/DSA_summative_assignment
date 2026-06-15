#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define MAX_INCIDENTS 25
#define DESC_LEN 100

typedef struct Incident {
    int id;
    char description[DESC_LEN];
    char timestamp[30];
    struct Incident *prev;
    struct Incident *next;
} Incident;

typedef struct {
    Incident *head;
    Incident *tail;
    Incident *current;
    int count;
    int next_id;
    int live;
    pthread_mutex_t lock;
} Tracker;

Tracker tracker;

void get_timestamp(char *buf) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(buf, 30, "%Y-%m-%d %H:%M:%S", tm_info);
}

void add_incident(const char *desc) {
    pthread_mutex_lock(&tracker.lock);

    if (tracker.count == MAX_INCIDENTS) {
        Incident *old = tracker.head;
        tracker.head = old->next;
        if (tracker.head)
            tracker.head->prev = NULL;
        if (tracker.current == old)
            tracker.current = tracker.head;
        free(old);
        tracker.count--;
    }

    Incident *node = malloc(sizeof(Incident));
    node->id = tracker.next_id++;
    strncpy(node->description, desc, DESC_LEN - 1);
    node->description[DESC_LEN - 1] = '\0';
    get_timestamp(node->timestamp);
    node->next = NULL;
    node->prev = tracker.tail;

    if (tracker.tail)
        tracker.tail->next = node;
    else
        tracker.head = node;

    tracker.tail = node;
    tracker.count++;

    if (tracker.current == NULL)
        tracker.current = node;

    pthread_mutex_unlock(&tracker.lock);
}

void display_current() {
    pthread_mutex_lock(&tracker.lock);
    if (tracker.current == NULL) {
        printf("[No incidents recorded]\n");
    } else {
        printf("\n[Incident #%d] %s\n  %s\n",
               tracker.current->id,
               tracker.current->timestamp,
               tracker.current->description);
        printf("  (Position: older <-- | --> newer)\n");
    }
    pthread_mutex_unlock(&tracker.lock);
}

void move_forward() {
    pthread_mutex_lock(&tracker.lock);
    if (tracker.current && tracker.current->next) {
        tracker.current = tracker.current->next;
        pthread_mutex_unlock(&tracker.lock);
        display_current();
    } else {
        printf("[Already at the newest incident]\n");
        pthread_mutex_unlock(&tracker.lock);
    }
}

void move_backward() {
    pthread_mutex_lock(&tracker.lock);
    if (tracker.current && tracker.current->prev) {
        tracker.current = tracker.current->prev;
        pthread_mutex_unlock(&tracker.lock);
        display_current();
    } else {
        printf("[Already at the oldest incident]\n");
        pthread_mutex_unlock(&tracker.lock);
    }
}

void delete_all() {
    pthread_mutex_lock(&tracker.lock);
    Incident *cur = tracker.head;
    while (cur) {
        Incident *next = cur->next;
        free(cur);
        cur = next;
    }
    tracker.head = tracker.tail = tracker.current = NULL;
    tracker.count = 0;
    printf("[All incidents deleted]\n");
    pthread_mutex_unlock(&tracker.lock);
}

void save_session() {
    pthread_mutex_lock(&tracker.lock);
    FILE *f = fopen("session.log", "w");
    if (!f) {
        printf("[Error saving session]\n");
        pthread_mutex_unlock(&tracker.lock);
        return;
    }
    Incident *cur = tracker.head;
    while (cur) {
        fprintf(f, "#%d [%s] %s\n", cur->id, cur->timestamp, cur->description);
        cur = cur->next;
    }
    fclose(f);
    printf("[Session saved to session.log]\n");
    pthread_mutex_unlock(&tracker.lock);
}

const char *sample_incidents[] = {
    "Fire reported at 5th Avenue",
    "Traffic collision on Highway 3",
    "Medical emergency at City Hospital",
    "Burglary at Downtown Mall",
    "Gas leak reported near Station 7",
    "Flooding on River Road",
    "Power outage in North District"
};

void *live_monitor(void *arg) {
    int idx = 0;
    while (tracker.live) {
        sleep(3);
        if (!tracker.live) break;
        const char *msg = sample_incidents[idx % 7];
        idx++;
        add_incident(msg);
        printf("\n[LIVE] New incident added: %s\n> ", msg);
        fflush(stdout);
    }
    return NULL;
}

int main() {
    tracker.head = tracker.tail = tracker.current = NULL;
    tracker.count = 0;
    tracker.next_id = 1;
    tracker.live = 0;
    pthread_mutex_init(&tracker.lock, NULL);

    add_incident("Unit 1 responding to disturbance on Main St");
    add_incident("Ambulance dispatched to 12 Oak Lane");
    add_incident("Fire alarm triggered at Central Library");
    tracker.current = tracker.head;

    pthread_t thread;
    char cmd;

    printf("=== Emergency Dispatch Incident Tracker ===\n");
    printf("Commands: f=forward  b=backward  l=live  s=stop  d=delete  q=quit\n");
    display_current();

    while (1) {
        printf("> ");
        fflush(stdout);
        if (scanf(" %c", &cmd) != 1) break;

        if (cmd == 'f') {
            move_forward();
        } else if (cmd == 'b') {
            move_backward();
        } else if (cmd == 'l') {
            if (!tracker.live) {
                tracker.live = 1;
                pthread_create(&thread, NULL, live_monitor, NULL);
                printf("[Live monitoring started]\n");
            } else {
                printf("[Live monitoring already active]\n");
            }
        } else if (cmd == 's') {
            if (tracker.live) {
                tracker.live = 0;
                pthread_join(thread, NULL);
                printf("[Live monitoring stopped]\n");
            } else {
                printf("[Live monitoring is not active]\n");
            }
        } else if (cmd == 'd') {
            delete_all();
        } else if (cmd == 'q') {
            if (tracker.live) {
                tracker.live = 0;
                pthread_join(thread, NULL);
            }
            save_session();
            break;
        } else {
            printf("[Unknown command]\n");
        }
    }

    delete_all();
    pthread_mutex_destroy(&tracker.lock);
    return 0;
}
