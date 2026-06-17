#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CHAR 256

typedef struct HuffNode {
    unsigned char ch;
    int freq;
    struct HuffNode *left, *right;
} HuffNode;

typedef struct {
    HuffNode *nodes[MAX_CHAR * 2];
    int size;
} MinHeap;

HuffNode *new_node(unsigned char ch, int freq) {
    HuffNode *n = malloc(sizeof(HuffNode));
    n->ch = ch; n->freq = freq;
    n->left = n->right = NULL;
    return n;
}

void heap_push(MinHeap *h, HuffNode *node) {
    int i = h->size++;
    h->nodes[i] = node;
    while (i > 0) {
        int parent = (i - 1) / 2;
        if (h->nodes[parent]->freq > h->nodes[i]->freq) {
            HuffNode *tmp = h->nodes[parent];
            h->nodes[parent] = h->nodes[i];
            h->nodes[i] = tmp;
            i = parent;
        } else break;
    }
}

HuffNode *heap_pop(MinHeap *h) {
    HuffNode *top = h->nodes[0];
    h->nodes[0] = h->nodes[--h->size];
    int i = 0;
    while (1) {
        int left = 2*i+1, right = 2*i+2, smallest = i;
        if (left < h->size && h->nodes[left]->freq < h->nodes[smallest]->freq)
            smallest = left;
        if (right < h->size && h->nodes[right]->freq < h->nodes[smallest]->freq)
            smallest = right;
        if (smallest == i) break;
        HuffNode *tmp = h->nodes[i];
        h->nodes[i] = h->nodes[smallest];
        h->nodes[smallest] = tmp;
        i = smallest;
    }
    return top;
}

char codes[MAX_CHAR][MAX_CHAR];

void build_codes(HuffNode *root, char *buf, int depth) {
    if (!root->left && !root->right) {
        buf[depth] = '\0';
        strncpy(codes[root->ch], buf, MAX_CHAR - 1);
        return;
    }
    if (root->left) {
        buf[depth] = '0';
        build_codes(root->left, buf, depth + 1);
    }
    if (root->right) {
        buf[depth] = '1';
        build_codes(root->right, buf, depth + 1);
    }
}

void free_tree(HuffNode *root) {
    if (!root) return;
    free_tree(root->left);
    free_tree(root->right);
    free(root);
}

HuffNode *build_huffman_tree(MinHeap *heap) {
    if (heap->size == 0) return NULL;
    while (heap->size > 1) {
        HuffNode *l = heap_pop(heap);
        HuffNode *r = heap_pop(heap);
        HuffNode *parent = new_node(0, l->freq + r->freq);
        parent->left = l;
        parent->right = r;
        heap_push(heap, parent);
    }
    HuffNode *root = heap_pop(heap);
    if (root && !root->left && !root->right) {
        HuffNode *parent = new_node(0, root->freq);
        parent->left = root;
        root = parent;
    }
    return root;
}

void write_bits(FILE *f, const char *bits, unsigned char *buf, int *bit_pos) {
    for (int i = 0; bits[i]; i++) {
        if (bits[i] == '1')
            *buf |= (1 << (7 - *bit_pos));
        (*bit_pos)++;
        if (*bit_pos == 8) {
            fwrite(buf, 1, 1, f);
            *buf = 0;
            *bit_pos = 0;
        }
    }
}

int compress(const char *infile) {
    FILE *fin = fopen(infile, "rb");
    if (!fin) { printf("[Error] Cannot open %s\n", infile); return 0; }

    int freq[MAX_CHAR] = {0};
    unsigned char c;
    long original_size = 0;
    while (fread(&c, 1, 1, fin) == 1) { freq[c]++; original_size++; }
    fclose(fin);

    if (original_size == 0) { printf("[Error] File is empty\n"); return 0; }

    MinHeap heap = { .size = 0 };
    for (int i = 0; i < MAX_CHAR; i++)
        if (freq[i]) heap_push(&heap, new_node((unsigned char)i, freq[i]));

    HuffNode *root = build_huffman_tree(&heap);
    memset(codes, 0, sizeof(codes));
    char buf[MAX_CHAR];
    build_codes(root, buf, 0);

    FILE *fout = fopen("telemetry.huff", "wb");
    if (!fout) { printf("[Error] Cannot create output file\n"); free_tree(root); return 0; }

    fwrite(&original_size, sizeof(long), 1, fout);
    for (int i = 0; i < MAX_CHAR; i++)
        fwrite(&freq[i], sizeof(int), 1, fout);

    fin = fopen(infile, "rb");
    unsigned char byte = 0;
    int bit_pos = 0;
    while (fread(&c, 1, 1, fin) == 1)
        write_bits(fout, codes[c], &byte, &bit_pos);
    if (bit_pos > 0) fwrite(&byte, 1, 1, fout);
    fclose(fin);

    long compressed_size = ftell(fout);
    fclose(fout);
    free_tree(root);

    printf("Original size:    %ld bytes\n", original_size);
    printf("Compressed size:  %ld bytes\n", compressed_size);
    printf("Compression ratio: %.2f%%\n", (1.0 - (double)compressed_size / original_size) * 100);
    printf("Saved to telemetry.huff\n");
    return 1;
}

int decompress() {
    FILE *fin = fopen("telemetry.huff", "rb");
    if (!fin) { printf("[Error] telemetry.huff not found\n"); return 0; }

    long original_size;
    fread(&original_size, sizeof(long), 1, fin);
    int freq[MAX_CHAR];
    for (int i = 0; i < MAX_CHAR; i++)
        fread(&freq[i], sizeof(int), 1, fin);

    MinHeap heap = { .size = 0 };
    for (int i = 0; i < MAX_CHAR; i++)
        if (freq[i]) heap_push(&heap, new_node((unsigned char)i, freq[i]));

    HuffNode *root = build_huffman_tree(&heap);
    FILE *fout = fopen("telemetry_restored.txt", "wb");
    if (!fout) { printf("[Error] Cannot create output\n"); free_tree(root); fclose(fin); return 0; }

    HuffNode *cur = root;
    long decoded = 0;
    unsigned char byte;
    while (decoded < original_size && fread(&byte, 1, 1, fin) == 1) {
        for (int bit = 7; bit >= 0 && decoded < original_size; bit--) {
            cur = (byte >> bit & 1) ? cur->right : cur->left;
            if (!cur->left && !cur->right) {
                fwrite(&cur->ch, 1, 1, fout);
                decoded++;
                cur = root;
            }
        }
    }
    fclose(fin);
    fclose(fout);
    free_tree(root);
    printf("Decompressed to telemetry_restored.txt (%ld bytes)\n", decoded);
    return 1;
}

int verify(const char *original) {
    FILE *f1 = fopen(original, "rb");
    FILE *f2 = fopen("telemetry_restored.txt", "rb");
    if (!f1 || !f2) {
        if (f1) fclose(f1);
        if (f2) fclose(f2);
        return 0;
    }
    unsigned char c1, c2;
    int ok = 1;
    while (fread(&c1, 1, 1, f1) == 1) {
        if (fread(&c2, 1, 1, f2) != 1 || c1 != c2) { ok = 0; break; }
    }
    if (ok && fread(&c2, 1, 1, f2) == 1) ok = 0;
    fclose(f1); fclose(f2);
    return ok;
}

int main(int argc, char *argv[]) {
    const char *infile = argc > 1 ? argv[1] : "telemetry.txt";

    printf("=== Telemetry Huffman Compression Utility ===\n\n");

    printf("[Compressing %s...]\n", infile);
    if (!compress(infile)) return 1;

    printf("\n[Decompressing telemetry.huff...]\n");
    if (!decompress()) return 1;

    printf("\n[Verifying...]\n");
    if (verify(infile))
        printf("[PASS] Restored file matches original exactly.\n");
    else
        printf("[FAIL] Files differ — check implementation.\n");

    return 0;
}
