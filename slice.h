#pragma once
#ifndef SLICE_H
#define SLICE_H
#include "common.h"
#include <stdio.h>

// A Slice is a range of chars
// Used to represent strings most of the time.
// Because we know the full range, there is no null termination
typedef struct Slice {
    char *begin;
    char *end;
} Slice;

static inline
Slice MakeSlice(char *buffer, memsize size) { 
    return (Slice) {buffer, buffer + size}; 
}

static inline
Slice NullSlice(void) { 
    return (Slice) {0, 0};
}

static inline
int IsNullSlice(Slice s) { 
    return s.begin == NULL; 
}

static inline
memsize SliceLength(Slice s) { 
    return (memsize)(s.end - s.begin); 
}

// "CStr" is the name in the project for a null terminated string (ie, a
// C-style String -> CStr)
Slice SliceFromCStr(const char *cstr); 

// Write slice to file
void SliceFPrint(Slice s, FILE *out); 

// Write slice to stdout
void SlicePrint(Slice s); 

// Compare two slices lexicographically, returns:
// >  0 -> a >  b
// == 0 -> a == b
// <  0 -> a <  b
int SliceCmp(Slice a, Slice b); 

// Returns true if the slice equals the given
// null terminated string
int SliceEqCStr(Slice a, const char *b); 
int SliceStartsWithCStr(Slice a, const char *b); 
int SliceEndsWithCStr(Slice a, const char *b); 

#endif
