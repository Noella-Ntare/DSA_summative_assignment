#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define MAX_NODES 20
#define NAME_LEN 32

typedef struct EdgeNode {
    int dest;
    int weight;
    struct EdgeNode *next;
} EdgeNode;

typedef struct {
    char name[NAME_LEN];
    EdgeNode *edges;
} Node;

Node graph[MAX_NODES];
int node_count = 0;

int find_node(const char *name) {
    for (int i = 0; i < node_count; i++)
        if (strcmp(graph[i].name, name) == 0)
            return i;
    return -1;
}

int add_node(const char *name) {
    int idx = find_node(name);
    if (idx != -1) return idx;
    strncpy(graph[node_count].name, name, NAME_LEN - 1);
    graph[node_count].edges = NULL;
    return node_count++;
}

void add_edge(int from, int to, int w) {
    EdgeNode *e = malloc(sizeof(EdgeNode));
    e->dest = to; e->weight = w;
    e->next = graph[from].edges;
    graph[from].edges = e;
}

void add_path(const char *a, const char *b, int w) {
    int u = add_node(a), v = add_node(b);
    add_edge(u, v, w);
    add_edge(v, u, w);
}

void dijkstra(int src, int dest) {
    int dist[MAX_NODES], visited[MAX_NODES], prev[MAX_NODES];
    for (int i = 0; i < node_count; i++) {
        dist[i] = INT_MAX;
        visited[i] = 0;
        prev[i] = -1;
    }
    dist[src] = 0;

    for (int i = 0; i < node_count; i++) {
        int u = -1;
        for (int j = 0; j < node_count; j++) {
            if (!visited[j] && (u == -1 || dist[j] < dist[u]))
                u = j;
        }
        if (u == -1 || dist[u] == INT_MAX) break;
        visited[u] = 1;

        EdgeNode *e = graph[u].edges;
        while (e) {
            int v = e->dest;
            if (!visited[v] && dist[u] + e->weight < dist[v]) {
                dist[v] = dist[u] + e->weight;
                prev[v] = u;
            }
            e = e->next;
        }
    }

    if (dist[dest] == INT_MAX) {
        printf("[No path found from %s to %s]\n", graph[src].name, graph[dest].name);
        return;
    }

    int path[MAX_NODES], len = 0, cur = dest;
    while (cur != -1) {
        path[len++] = cur;
        cur = prev[cur];
    }

    printf("\nRoute: ");
    for (int i = len - 1; i >= 0; i--) {
        printf("%s", graph[path[i]].name);
        if (i > 0) printf(" --> ");
    }
    printf("\nTotal distance: %d units\n", dist[dest]);
}

int main() {
    add_path("Library", "Cafeteria", 6);
    add_path("Library", "Engineering", 15);
    add_path("Cafeteria", "Science Block", 4);
    add_path("Science Block", "Dormitory", 8);
    add_path("Engineering", "Administration", 5);
    add_path("Administration", "Dormitory", 3);
    add_path("Cafeteria", "Charging Station", 2);
    add_path("Charging Station", "Administration", 4);

    int dest = find_node("Dormitory");

    printf("=== Campus Delivery Robot Navigation ===\n");
    printf("Available buildings:\n");
    for (int i = 0; i < node_count; i++)
        printf("  - %s\n", graph[i].name);

    char start[NAME_LEN];
    while (1) {
        printf("\nEnter starting building (or 'quit'): ");
        if (!fgets(start, sizeof(start), stdin)) break;
        start[strcspn(start, "\r\n")] = '\0';
        if (strcmp(start, "quit") == 0) break;

        int src = find_node(start);
        if (src == -1) {
            printf("[Error] Building '%s' not found. Please check the name.\n", start);
            continue;
        }
        if (src == dest) {
            printf("[Already at Dormitory]\n");
            continue;
        }
        dijkstra(src, dest);
    }

    for (int i = 0; i < node_count; i++) {
        EdgeNode *e = graph[i].edges;
        while (e) {
            EdgeNode *tmp = e->next;
            free(e);
            e = tmp;
        }
    }
    return 0;
}
