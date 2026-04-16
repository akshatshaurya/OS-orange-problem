#include "index.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

// from object.c
int object_write(ObjectType type, const void *data, size_t len, ObjectID *id_out);

// ─── FIND ─────────────────────────

IndexEntry* index_find(Index *index, const char *path) {
    for (int i = 0; i < index->count; i++) {
        if (strcmp(index->entries[i].path, path) == 0)
            return &index->entries[i];
    }
    return NULL;
}

// ─── LOAD ─────────────────────────

int index_load(Index *index) {
    index->count = 0;

    FILE *f = fopen(INDEX_FILE, "r");
    if (!f) return 0;

    char line[1024];

    while (fgets(line, sizeof(line), f)) {
        if (index->count >= MAX_INDEX_ENTRIES) break;

        IndexEntry *e = &index->entries[index->count];
        char hex[HASH_HEX_SIZE + 1];

        if (sscanf(line, "%o %64s %lu %u %[^\n]",
                   &e->mode,
                   hex,
                   &e->mtime_sec,
                   &e->size,
                   e->path) != 5) {
            fclose(f);
            return -1;
        }

        if (hex_to_hash(hex, &e->hash) != 0) {
            fclose(f);
            return -1;
        }

        index->count++;
    }

    fclose(f);
    return 0;
}

// ─── SAVE (FINAL FIXED) ─────────────────────────

int index_save(const Index *index) {
    // ensure .pes exists
    mkdir(PES_DIR, 0755);

    FILE *f = fopen(INDEX_FILE, "w");
    if (!f) return -1;

    for (int i = 0; i < index->count; i++) {
        char hex[HASH_HEX_SIZE + 1];
        hash_to_hex(&index->entries[i].hash, hex);

        fprintf(f, "%o %s %lu %u %s\n",
                index->entries[i].mode,
                hex,
                index->entries[i].mtime_sec,
                index->entries[i].size,
                index->entries[i].path);
    }

    fclose(f);
    return 0;
}

// ─── ADD ─────────────────────────

int index_add(Index *index, const char *path) {
    struct stat st;
    if (stat(path, &st) != 0) {
        printf("error: cannot access %s\n", path);
        return -1;
    }

    FILE *f = fopen(path, "rb");
    if (!f) return -1;

    size_t size = st.st_size;

    char *data = NULL;
    if (size > 0) {
        data = malloc(size);
        if (!data) {
            fclose(f);
            return -1;
        }

        fread(data, 1, size, f);
    }

    fclose(f);

    ObjectID id;
    if (object_write(OBJ_BLOB, data, size, &id) != 0) {
        free(data);
        return -1;
    }

    free(data);

    IndexEntry *e = index_find(index, path);

    if (!e) {
        if (index->count >= MAX_INDEX_ENTRIES) return -1;
        e = &index->entries[index->count++];
    }

    e->mode = st.st_mode;
    e->hash = id;
    e->mtime_sec = st.st_mtime;
    e->size = st.st_size;

    strcpy(e->path, path);

    return index_save(index);
}

// ─── STATUS ─────────────────────────

int index_status(const Index *index) {
    printf("Staged changes:\n");

    if (index->count == 0)
        printf("  (nothing to show)\n");

    for (int i = 0; i < index->count; i++) {
        printf("  staged:     %s\n", index->entries[i].path);
    }

    printf("\nUnstaged changes:\n  (nothing to show)\n\n");

    printf("Untracked files:\n");

    DIR *d = opendir(".");
    struct dirent *ent;

    while ((ent = readdir(d))) {
        if (strcmp(ent->d_name, ".") == 0 ||
            strcmp(ent->d_name, "..") == 0 ||
            strcmp(ent->d_name, ".pes") == 0)
            continue;

        int found = 0;

        for (int i = 0; i < index->count; i++) {
            if (strcmp(index->entries[i].path, ent->d_name) == 0) {
                found = 1;
                break;
            }
        }

        if (!found) {
            printf("  untracked:  %s\n", ent->d_name);
        }
    }

    closedir(d);
    printf("\n");
    return 0;
}
