#include "arena.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

char *RawArenaPush(Arena *a, memsize size) {
    assert((memsize)(a->end - a->current) >= size);

    if ((memsize)(a->end - a->current) < size) {
        fprintf(stderr, "Need more memory! Arena is full.\n");
        exit(-1);
    }

    char *buf = a->current;
    a->current += size;
    return buf;
}

Arena AllocArena(memsize size) {
    char *buffer = malloc(size);

    if (!buffer) {
        fprintf(stderr, "Could not allocate memory arena, size was %d", (int)size);
        exit(-1);
    }

    return MakeArena(buffer, size);
}

void FreeArena(Arena *a) { 
    free(a->begin); 
}


// Arena string building functions

void ArenaPushData(Arena *a, char *data, memsize data_count) {
    char *buf = RawArenaPush(a, data_count);
    memcpy(buf, data, data_count);
}

void ArenaPushCStr(Arena *a, const char *cstr) {
    while (*cstr && a->current < a->end) {
        *a->current++ = *cstr++;
    }
}

Slice ArenaPushSlice(Arena *a, Slice s) {
    char *begin = a->current;
    memsize len = (memsize)(s.end - s.begin);
    ArenaPushData(a, s.begin, len);
    return (Slice) {begin, a->current};
}

void ArenaPushfv(Arena *a, const char *fmt, va_list ap) {
    int amt = vsnprintf(a->current, ArenaSpace(a), fmt, ap);
    if (amt > ArenaSpace(a)-1) { amt = (int)ArenaSpace(a)-1; }
    a->current += amt;
}

void ArenaPushf(Arena *a, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    ArenaPushfv(a, fmt, ap);
    va_end(ap);
}

Slice ArenaPrintf(Arena *a, const char *fmt, ...) {
    char *begin = a->current;

    va_list ap;
    va_start(ap, fmt);
    ArenaPushfv(a, fmt, ap);
    va_end(ap);

    return (Slice) {begin, a->current};
}

const char *ArenaPrintfCStr(Arena *a, const char *fmt, ...) {
    char *begin = a->current;
    va_list ap;
    va_start(ap, fmt);
    ArenaPushfv(a, fmt, ap);
    va_end(ap);
    *RawArenaPush(a, 1) = 0;
    return begin;
}

const char *ArenaCloneCStr(Arena *a, const char *str) {
    char *begin = a->current;
    ArenaPushCStr(a, str);
    *RawArenaPush(a, 1) = 0;
    return begin;
}

