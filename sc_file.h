#pragma once
#ifndef SC_FILE_H
#define SC_FILE_H
#include "common.h"
#include "slice.h"
#include "arena.h"

// site.c File Format:
//
// site.c uses a format similar to latex. There is an older version of site.c
// that used markdown, but as you add stuff to markdown it:
// 1. Deviates more and more from the standard
// 2. Requires an accumulation of more and more varieties of syntax,
//    complicating parsing.
// Additionally, most existing markdown implementations output awful HTML.
//
// The goals of this format are:
// 1. Invert the structure of HTML - the text encapsulates some commands,
// instead of a bunch of tags encapsulating the text. The text should be front
// and center.
// 2. Output good HTML5 with semantic tags
// 3. Be more easily extensible than markdown parsers. 
//
// All regular text in a file is interpreted as plain text, and is escaped by
// a backslash character. To include a backslash in the plain text, use a
// double backslash. After the backslash, you have a function call syntax with
// keyword arguments. After the function call, plain text
// resumes.
// 
// Example:
/*
Herp derp derp derp
Herp derp \functioncall(arg1="value", herp=212, derp="value", wah=blah) Herp derp derp
derp derp derp
Here is a backslash or \\ 
Not all function have arguments: \noargfunc
Functions can have blocks: 
\herp{
    HASERQW
    QWERQWE
    ZXCVZXCV
    QWERQEWr
}

\foof(arg1="This is an arg"){But there is also a block}
*/

#define SC_MAX_ARGS     32

// This format does not require an AST, since you cannot nest functions.
// Instead, it produces an object stream/list.
typedef enum SCObjectType {
    SCObjectType_Text,       // Just some plain text
    SCObjectType_Func,       // Function 
    SCObjectType_End,        // End of the file
    SCObjectType_Backslash,  // An escaped backslash
    SCObjectType_Error,      // Something went wrong
    SCObjectType_COUNT
} SCObjectType;

static const char *g_sc_object_type_name[SCObjectType_COUNT] = {
    "Text",
    "Func",
    "End",
    "Backslash",
    "Error",
};

typedef struct SCObject {
    SCObjectType type;

    // Position of the start of full_text
    int          line_no;
    int          column_no;

    // Position of the end of full_text
    int          end_line_no;
    int          end_column_no;

    Slice        full_text;

    // Function object data:
    Slice  function_name;
    Slice  keys  [SC_MAX_ARGS];
    Slice  values[SC_MAX_ARGS];
    int    args_count;
    int    has_block;
    Slice  block;

    // Error string for SCObjectType_Error
    const char *error_text;
    const char *path;
    const char *file_name;
} SCObject;

// SC Lexer + Parser mashed togther. This file format is actually a regular
// language so you don't really need to separate lexing from parsing. It also
// makes it easier to grab all the plain text out in big chunks.
typedef struct SCReader {
    int   line_no;
    int   column_no;
    char *begin;
    char *current;
    char *end;
    const char *error;

    // Only used when printing error messages
    const char *path;
    const char *file_name;
} SCReader;

SCReader MakeSCReader(Slice text, const char *path, const char *file);

// Read the next object from the SC file
//
// If an error occurs, an object of SCObjectType_Error is returned, and the
// same object is returned on all subsequent calls
//
// Once the end of the file is reached, an SCObjectType_End object is
// returned, and will be returned on all subsequent calls
void SCRead(SCReader *r, SCObject *out);

// Convenience function to construct an error message with line number info. 
// If error_text is null, uses the error from the SCObject.
Slice SCMakeErrorString(SCObject *obj, Arena *arena, const char *error_text);

// Convenience macro to handle the SCObjectType_Error case when reading
// an sc file.
//
// Produces an error message, assigned it to the slice pointed to
// by error_text_ptr, and returns 0.
#define R_HandleSCObjectTypeError(obj, arena_ptr, error_text_ptr) \
    case SCObjectType_Error: \
    { \
        *(error_text_ptr) = SCMakeErrorString(&(obj), (arena_ptr), 0); \
        return 0; \
    } break

// Convenience macro for handling cases when parsing an sc file
// Checks to ensure that a command has a block, returns with error if not
#define R_CheckSCObjectHasBlock(obj, name, arena, error_out_slice_ptr) \
    if (!(obj.has_block)) {                                           \
        *(error_out_slice_ptr) = SCMakeErrorString(&(obj), (arena),   \
                                                   name " commands require a block"); \
        return 0;                                                     \
    }

// Prints the sc object to stdout
void PrintSCObject(SCObject *obj);

// isspace(ch) for all ch in text
int IsAllWhitespace(Slice text);

#ifndef NDEBUG
void TEST_SCReader(void);
#endif

#endif

