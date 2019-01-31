#include "slice.h"
#include <stdio.h>

void SliceFPrint(Slice s, FILE *out) { 
    fwrite(s.begin, sizeof(char), SliceLength(s), out); 
}

void SlicePrint(Slice s) { 
    fwrite(s.begin, sizeof(char), SliceLength(s), stdout); 
}

int SliceCmp(Slice a, Slice b) {
    memsize alen = (memsize)(a.end - a.begin);
    memsize blen = (memsize)(b.end - b.begin);
    memsize min_len = alen < blen ? alen : blen;
    int result = memcmp(a.begin, b.begin, min_len);

    if (result == 0 && alen != blen) {
        return blen - alen;
    }

    return result;
}

int SliceEqCStr(Slice a, const char *b) {
    memsize alen = (memsize)(a.end - a.begin);
    memsize blen = strlen(b);
    if (blen != alen) { return 0; }
    return memcmp(a.begin, b, alen) == 0;
}

int SliceStartsWithCStr(Slice a, const char *b) {
    memsize alen = (memsize)(a.end - a.begin);
    memsize blen = strlen(b);
    if (blen > alen) { return 0; }
    return memcmp(a.begin, b, blen) == 0;
}

int SliceEndsWithCStr(Slice a, const char *b) {
    memsize alen = (memsize)(a.end - a.begin);
    memsize blen = strlen(b);
    if (blen > alen) { return 0; }
    return memcmp(a.begin + (alen - blen), b, blen) == 0;
}

