#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PROC 50
#define PROC_LEN 64

typedef struct BSTNode {
    char procedure[PROC_LEN];
    struct BSTNode *left;
    struct BSTNode *right;
} BSTNode;

BSTNode *bst_root = NULL;

BSTNode *new_node(const char *proc) {
    BSTNode *node = malloc(sizeof(BSTNode));
    strncpy(node->procedure, proc, PROC_LEN - 1);
    node->procedure[PROC_LEN - 1] = '\0';
    node->left = node->right = NULL;
    return node;
}

BSTNode *insert(BSTNode *root, const char *proc) {
    if (!root) return new_node(proc);
    int cmp = strcmp(proc, root->procedure);
    if (cmp < 0)
        root->left = insert(root->left, proc);
    else if (cmp > 0)
        root->right = insert(root->right, proc);
    return root;
}

BSTNode *search(BSTNode *root, const char *proc) {
    if (!root) return NULL;
    int cmp = strcmp(proc, root->procedure);
    if (cmp == 0) return root;
    if (cmp < 0) return search(root->left, proc);
    return search(root->right, proc);
}

int levenshtein(const char *a, const char *b) {
    int la = strlen(a), lb = strlen(b);
    int dp[la + 1][lb + 1];
    for (int i = 0; i <= la; i++) dp[i][0] = i;
    for (int j = 0; j <= lb; j++) dp[0][j] = j;
    for (int i = 1; i <= la; i++) {
        for (int j = 1; j <= lb; j++) {
            if (a[i-1] == b[j-1])
                dp[i][j] = dp[i-1][j-1];
            else
                dp[i][j] = 1 + (dp[i-1][j] < dp[i][j-1] ? (dp[i-1][j] < dp[i-1][j-1] ? dp[i-1][j] : dp[i-1][j-1]) : (dp[i][j-1] < dp[i-1][j-1] ? dp[i][j-1] : dp[i-1][j-1]));
        }
    }
    return dp[la][lb];
}

char best_match[PROC_LEN];
int best_dist;

void find_closest(BSTNode *root, const char *input) {
    if (!root) return;
    int d = levenshtein(input, root->procedure);
    if (d < best_dist) {
        best_dist = d;
        strncpy(best_match, root->procedure, PROC_LEN - 1);
    }
    find_closest(root->left, input);
    find_closest(root->right, input);
}

void log_unknown(const char *proc) {
    FILE *f = fopen("audit.log", "a");
    if (f) {
        fprintf(f, "[REJECTED] Unknown procedure attempt: %s\n", proc);
        fclose(f);
    }
}

void free_tree(BSTNode *root) {
    if (!root) return;
    free_tree(root->left);
    free_tree(root->right);
    free(root);
}

int load_procedures(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        printf("[Error] Could not open %s\n", filename);
        return 0;
    }
    char line[PROC_LEN];
    int count = 0;
    while (fgets(line, sizeof(line), f) && count < MAX_PROC) {
        line[strcspn(line, "\r\n")] = '\0';
        if (strlen(line) > 0) {
            bst_root = insert(bst_root, line);
            count++;
        }
    }
    fclose(f);
    printf("[%d procedures loaded]\n", count);
    return count;
}

int main() {
    if (!load_procedures("procedures.txt")) {
        bst_root = insert(bst_root, "LOCK_PANEL");
        bst_root = insert(bst_root, "RESET_SENSOR");
        bst_root = insert(bst_root, "CALIBRATE_ARM");
        bst_root = insert(bst_root, "RESTART_LINE");
        bst_root = insert(bst_root, "FLUSH_VALVE");
        bst_root = insert(bst_root, "POWER_CYCLE");
        printf("[Loaded default procedures]\n");
    }

    char input[PROC_LEN];
    printf("=== Maintenance Procedure Validator ===\n");
    printf("Enter 'exit' to quit.\n\n");

    while (1) {
        printf("Enter procedure: ");
        if (!fgets(input, sizeof(input), stdin)) break;
        input[strcspn(input, "\r\n")] = '\0';
        if (strcmp(input, "exit") == 0) break;
        if (strlen(input) == 0) continue;

        BSTNode *found = search(bst_root, input);
        if (found) {
            printf("[APPROVED] Procedure '%s' is authorized.\n\n", input);
        } else {
            best_dist = 9999;
            best_match[0] = '\0';
            find_closest(bst_root, input);

            if (best_dist <= 4 && best_match[0] != '\0') {
                printf("[SUGGESTION] '%s' not recognized. Did you mean '%s'?\n\n", input, best_match);
            } else {
                printf("[REJECTED] '%s' is not an approved procedure.\n", input);
                log_unknown(input);
                printf("[Logged to audit.log]\n\n");
            }
        }
    }

    free_tree(bst_root);
    return 0;
}
