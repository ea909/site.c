#pragma once
#ifndef ARENA_H
#define ARENA_H
#include "common.h"
#include "slice.h"

// An Arena is a basic linear allocator
// All memory is grabbed from an arena, and all strings
// are constructed by pushing onto it too.
typedef struct Arena {
    char *current;
    char *begin;
    char *end;
} Arena;

typedef void* ArenaPos;

static inline
Arena MakeArena(char *buffer, memsize size) {
    return (Arena) {buffer, buffer, buffer + size};
}

// Allocates `size` bytes from the arena
char *RawArenaPush(Arena *a, memsize size); 

// These macros are used for most arena allocations. They take a type
// argument, generate a call to RawPushArena with the correct byte count, and
// cast the result to the requested type.
#define  ArenaPush(a, T) (T*)RawArenaPush((a), sizeof(T))
#define  ArenaPushMany(a, T, count) (T*)RawArenaPush((a), sizeof(T) * count)

static inline
void ArenaReset(Arena *a) { 
    a->current = a->begin; 
}

static inline
memsize ArenaSpace(Arena *a) { 
    return (memsize)(a->end - a->current); 
}

// Save and Restore allow arena space to be reused. You can mark a position in
// the arena, do a ton of work that involves allocation from the arena, and
// roll back in one instruction once its done.
static inline
ArenaPos ArenaSave(Arena *a) { 
    return (void*)a->current; 
}

static inline
void ArenaRestore(Arena *a, ArenaPos pos) { 
    a->current = pos; 
}

// BeginString/EndString marks the start of a string and
// then produces a slice after building it with the Push functions below
typedef char* ArenaString;

static inline
ArenaString ArenaBeginString(Arena *a) { 
    return a->current; 
}

static inline
Slice ArenaEndString(Arena *a, ArenaString s) {
    return (Slice) {s, a->current};
}

Arena AllocArena(memsize size); 
void FreeArena(Arena *a); 

////////////////////////////////////////////////
// Arena String Building functions.
// Use between ArenaBeginString/ArenaEndString
////////////////////////////////////////////////

static inline
void ArenaPushChar(Arena *a, char value) {
    *RawArenaPush(a, 1) = value;
}

void ArenaPushData(Arena *a, char *data, memsize data_count); 
void ArenaPushCStr(Arena *a, const char *cstr); 
Slice ArenaPushSlice(Arena *a, Slice s);

void ArenaPushfv(Arena *a, const char *fmt, va_list ap); 

// Push the result of an sprintf into the arena
void ArenaPushf(Arena *a, const char *fmt, ...); 

// Allocates the result of a sprintf in the arena, returning
// a Slice of the result
Slice ArenaPrintf(Arena *a, const char *fmt, ...); 

// Like the above, but makes a null terminated c string
const char *ArenaPrintfCStr(Arena *a, const char *fmt, ...); 

const char *ArenaCloneCStr(Arena *a, const char *str); 

#endif

