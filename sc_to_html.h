#pragma once
#ifndef SC_TO_HTML_H
#define SC_TO_HTML_H
#include "common.h"
#include "slice.h"
#include "arena.h"

#define SC_HTML_MAX_TAG_DEPTH 128

// Converts the input sc file text into an html file.
// This does not apply things like headers, footer, navigation.
// This just generates the raw html for the article, similar to
// the result of running markdown on a md file.
//
// On success:
// Returns true
// HTML text is allocated in the arena, and pointed to by
// out_slice.
//
// On failure:
// Returns false
// An error message is allocated in the arena, and pointed to
// by out_slice
//
// path and file are just strings used when making an error
// message, they are not opened/read
int SCToHTML(Slice sc, const char *path, const char *file, Arena *arena, Slice *out_slice) {

// IMPORTANT NOTE(eric): This escape function only supports characters I've
// actually used.  
//
// Writes text to the arena, escaping html special characters
void HTMLWriteEscapedText(Slice text, Arena *arena);

#ifndef NDEBUG
void TEST_SCToHTML(void);
#endif

#endif

