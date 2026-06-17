#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_AIRPORTS 20
#define CODE_LEN 4

typedef struct EdgeNode {
    int dest;
    struct EdgeNode *next;
} EdgeNode;

typedef struct {
    char code[CODE_LEN + 1];
    EdgeNode *edges;
} Airport;

Airport airports[MAX_AIRPORTS];
int airport_count = 0;

int find_airport(const char *code) {
    for (int i = 0; i < airport_count; i++)
        if (strcmp(airports[i].code, code) == 0)
            return i;
    return -1;
}

int add_airport(const char *code) {
    int idx = find_airport(code);
    if (idx != -1) return idx;
    if (airport_count >= MAX_AIRPORTS) {
        printf("[Max airports reached]\n");
        return -1;
    }
    strncpy(airports[airport_count].code, code, CODE_LEN);
    airports[airport_count].code[CODE_LEN] = '\0';
    airports[airport_count].edges = NULL;
    return airport_count++;
}

void add_route(const char *from, const char *to) {
    int f = add_airport(from);
    int t = add_airport(to);
    if (f == -1 || t == -1) return;

    EdgeNode *cur = airports[f].edges;
    while (cur) {
        if (cur->dest == t) {
            printf("[Route %s -> %s already exists]\n", from, to);
            return;
        }
        cur = cur->next;
    }

    EdgeNode *node = malloc(sizeof(EdgeNode));
    node->dest = t;
    node->next = airports[f].edges;
    airports[f].edges = node;
    printf("[Route added: %s -> %s]\n", from, to);
}

void remove_route(const char *from, const char *to) {
    int f = find_airport(from);
    int t = find_airport(to);
    if (f == -1 || t == -1) {
        printf("[Route not found]\n");
        return;
    }
    EdgeNode **prev = &airports[f].edges;
    while (*prev) {
        if ((*prev)->dest == t) {
            EdgeNode *tmp = *prev;
            *prev = tmp->next;
            free(tmp);
            printf("[Route removed: %s -> %s]\n", from, to);
            return;
        }
        prev = &(*prev)->next;
    }
    printf("[Route %s -> %s not found]\n", from, to);
}

void remove_airport(const char *code) {
    int idx = find_airport(code);
    if (idx == -1) {
        printf("[Airport not found]\n");
        return;
    }

    EdgeNode *e = airports[idx].edges;
    while (e) {
        EdgeNode *tmp = e->next;
        free(e);
        e = tmp;
    }

    for (int i = 0; i < airport_count; i++) {
        if (i == idx) continue;
        EdgeNode **prev = &airports[i].edges;
        while (*prev) {
            if ((*prev)->dest == idx) {
                EdgeNode *tmp = *prev;
                *prev = tmp->next;
                free(tmp);
            } else {
                if ((*prev)->dest > idx)
                    (*prev)->dest--;
                prev = &(*prev)->next;
            }
        }
    }

    for (int i = idx; i < airport_count - 1; i++)
        airports[i] = airports[i + 1];
    airport_count--;
    printf("[Airport %s removed]\n", code);
}

void query_airport(const char *code) {
    int idx = find_airport(code);
    if (idx == -1) {
        printf("[Airport %s not found]\n", code);
        return;
    }

    printf("\nAirport: %s\n", code);
    printf("  Outgoing (direct flights from %s): ", code);
    EdgeNode *e = airports[idx].edges;
    if (!e) printf("none");
    while (e) {
        printf("%s ", airports[e->dest].code);
        e = e->next;
    }
    printf("\n");

    printf("  Incoming (direct flights into %s): ", code);
    int found = 0;
    for (int i = 0; i < airport_count; i++) {
        if (i == idx) continue;
        EdgeNode *cur = airports[i].edges;
        while (cur) {
            if (cur->dest == idx) {
                printf("%s ", airports[i].code);
                found = 1;
            }
            cur = cur->next;
        }
    }
    if (!found) printf("none");
    printf("\n");
}

void print_matrix() {
    printf("\nAdjacency Matrix:\n     ");
    for (int j = 0; j < airport_count; j++)
        printf("%-5s", airports[j].code);
    printf("\n");
    for (int i = 0; i < airport_count; i++) {
        printf("%-5s", airports[i].code);
        for (int j = 0; j < airport_count; j++) {
            int connected = 0;
            EdgeNode *e = airports[i].edges;
            while (e) {
                if (e->dest == j) { connected = 1; break; }
                e = e->next;
            }
            printf("%-5d", connected);
        }
        printf("\n");
    }
}

void list_airports() {
    printf("Airports (%d): ", airport_count);
    for (int i = 0; i < airport_count; i++)
        printf("%s ", airports[i].code);
    printf("\n");
}

int main() {
    add_airport("KGL"); add_airport("NBO"); add_airport("EBB");
    add_airport("JNB"); add_airport("ADD"); add_airport("CAI");
    add_airport("CPT");

    add_route("KGL", "NBO"); add_route("KGL", "EBB");
    add_route("NBO", "JNB"); add_route("EBB", "ADD");
    add_route("ADD", "CAI"); add_route("JNB", "CPT");

    char cmd[10], a1[8], a2[8];
    printf("=== Airline Route Analyzer ===\n");
    printf("Commands: query <airport> | matrix | list | add_airport <code> |\n");
    printf("          add_route <from> <to> | del_route <from> <to> | del_airport <code> | quit\n\n");

    while (1) {
        printf("> ");
        if (scanf("%9s", cmd) != 1) break;

        if (strcmp(cmd, "query") == 0) {
            scanf("%7s", a1);
            query_airport(a1);
        } else if (strcmp(cmd, "matrix") == 0) {
            print_matrix();
        } else if (strcmp(cmd, "list") == 0) {
            list_airports();
        } else if (strcmp(cmd, "add_airport") == 0) {
            scanf("%7s", a1);
            int idx = add_airport(a1);
            if (idx != -1) printf("[Airport %s added]\n", a1);
        } else if (strcmp(cmd, "add_route") == 0) {
            scanf("%7s %7s", a1, a2);
            add_route(a1, a2);
        } else if (strcmp(cmd, "del_route") == 0) {
            scanf("%7s %7s", a1, a2);
            remove_route(a1, a2);
        } else if (strcmp(cmd, "del_airport") == 0) {
            scanf("%7s", a1);
            remove_airport(a1);
        } else if (strcmp(cmd, "quit") == 0) {
            break;
        } else {
            printf("[Unknown command]\n");
        }
    }

    for (int i = 0; i < airport_count; i++) {
        EdgeNode *e = airports[i].edges;
        while (e) {
            EdgeNode *tmp = e->next;
            free(e);
            e = tmp;
        }
    }
    return 0;
}
