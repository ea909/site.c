#include "sc_file.h"
#include <ctype.h>
#include <string.h>

// Valid characters for function names or argument keys
static int IsNameOrKeyChar(int c);

// Matches chars that make up a decimal number
static int IsDigitChar(int ch);

// Fill out an SCObjectType_Error object with the given error message.
static void SCError(SCReader *r, const char *str, SCObject *out);

// Consume characters from the input, updating the line/col numbers
static void SCConsume(SCReader *r, memsize chars);

// Checks for a character. Consumes if present and fails otherwise
static int SCExpect(SCReader *r, char c, const char *err, SCObject *out); 

// Consume until char is reached
static void SCConsumeUntil(SCReader *r, char c);

// Consume while predicate function is true
static void SCConsumeWhileFunc(SCReader *r, int (*func)(int ch));

// Consume whitespace
static void SCConsumeWhitespace(SCReader *r);

// Reads an argument.
// An argument is a key value pair with a 
// bareword key and either a string or number value.
//
// Ex:
// this_is_a_key = "This is a value"
// this_is_a_number = 22.3
//
// In a function:
// \example(arg1=22, arg2="string")
static void SCReadArgument(SCReader *r, SCObject *out);

// Read an argument list
// An argument list is a list of key value pairs (see SCReadArgument) wrapped
// in parens, ex:
// (arg1 = "this is a value", herparg = 33.6)
static void SCReadArgumentList(SCReader *r, SCObject *out);

// A block is a potentially multiline chunk of text wrapped in curly braces,
// ex:
// { 
//    This
//    is
//    a
//    block
// }
//
// In a function:
// \blockfunc{Here's a block too}
//
// The block reader counts and balances curly braces, so as long as the curly
// braces are balanced, you can include them in the block. This allows you
// to paste c-style source without having to add escape characters.
static void SCReadBlock(SCReader *r, SCObject *out);

// A function is the only non text item in the file.
// It is marked by a backslash, following by the function name, ex:
// \functioncall
// \herp
// \derp
//
// A function may optionally have an argument list:
// \function(arg1 = "QWER", arg2 = 44, foo = "bar")
//
// A function may optionally have a block, which is a single, possibly
// multiline argument:
// \function{
//     This
//     is 
//     a
//     block
// }
//
// A function may have arguments and a block:
// \function(arg1=22, arg2=33){This is the block}
static void SCReadFunction(SCReader *r, SCObject *out);

SCReader MakeSCReader(Slice text, const char *path, const char *file) {
    return (SCReader) {
        .line_no    = 1, 
        .column_no  = 1, 
        .begin      = text.begin, 
        .current    = text.begin, 
        .end        = text.end, 
        .error      = NULL,
        .path       = path,
        .file_name  = file,
    };
}

void SCRead(SCReader *r, SCObject *out) {
    memset(out, 0, sizeof(*out));

    out->line_no   = r->line_no;
    out->column_no = r->column_no;
    out->path      = r->path;
    out->file_name = r->file_name;

    if (r->error) { 
        SCError(r, r->error, out); 
        return;
    }

    if (r->current == r->end) {
        out->type = SCObjectType_End;
    } else if (r->current[0] == '\\') {
        if (r->current + 1 == r->end) {
            SCError(r, "Backslash unescaped and with no function at the end of file", out);
        } else if (r->current[1] == '\\') {
            char *backslash_begin = r->current;
            SCConsume(r, 2);
            out->type          = SCObjectType_Backslash;
            out->full_text     = (Slice) {backslash_begin, r->current};
            out->end_line_no   = r->line_no;
            out->end_column_no = r->column_no;
        } else {
            SCReadFunction(r, out);
        }
    } else {
        char *start = r->current;
        SCConsumeUntil(r, '\\');
        char *end          = r->current;
        out->type          = SCObjectType_Text;
        out->full_text     = (Slice) {start, end};
        out->end_line_no   = r->line_no;
        out->end_column_no = r->column_no;
    }
}

Slice SCMakeErrorString(SCObject *obj, Arena *arena, const char *error_text) {
    ArenaString error = ArenaBeginString(arena);
    ArenaPushCStr(arena, "Error while reading SC file: ");
    ArenaPushCStr(arena, obj->file_name);
    ArenaPushf(arena,    "\nPath was: ");
    ArenaPushCStr(arena, obj->path);
    ArenaPushf(arena,    "\nError: %s\n", 
               error_text ? error_text : obj->error_text);
    ArenaPushf(arena,    "Starting location: line %d, col %d\n", 
               obj->line_no, obj->column_no);
    ArenaPushf(arena,    "Ending location:   line %d, col %d\n",
               obj->end_line_no, obj->end_column_no);
    return ArenaEndString(arena, error);
}

void PrintSCObject(SCObject *obj) {
    printf("SCObject(\n");
    printf("    type=%s\n", g_sc_object_type_name[obj->type]);
    printf("    text={{{\n");
    SlicePrint(obj->full_text);
    printf("\n----}}}\n");

    if (obj->type == SCObjectType_Func) {
        printf("    fname=");
        SlicePrint(obj->function_name);
        printf("\n");
        printf("    args={\n");
        for (int i = 0; i < obj->args_count; i++) {
            printf("     ");
            SlicePrint(obj->keys[i]);
            printf(" = \"");
            SlicePrint(obj->values[i]);
            printf("\"\n");
        }
        printf("    }\n");

        if (obj->has_block) {
            printf("    block={{{\n");
            SlicePrint(obj->block);
            printf("\n----}}}\n");
        }
    } else if (obj->type == SCObjectType_Error) {
        printf("    error=\"%s\"\n", obj->error_text);
    }

    printf(")\n");
}

static void SCReadArgument(SCReader *r, SCObject *out) {
    if (out->args_count >= SC_MAX_ARGS) {
        SCError(r, "Function exceeds the max argument count", out);
        return;
    }

    SCConsumeWhitespace(r);

    // Read parameter key
    char *key_begin = r->current;
    SCConsumeWhileFunc(r, IsNameOrKeyChar);
    char *key_end = r->current;

    if (key_begin == key_end) {
        SCError(r, "Expected a parameter name", out);
        return;
    }

    // Consume the equals sign
    SCConsumeWhitespace(r);
    if (!SCExpect(r, '=', "Expected = after param name", out)) { return; }
    SCConsumeWhitespace(r);

    if (r->current >= r->end) {
        SCError(r, "Reached EOF without finding parameter value", out);
        return;
    }

    // Read parameter value
    // Can be a quoted string (starts with quote)
    // or a number (made of digit characters)
    char *value_begin = NULL, *value_end = NULL;
    if (*r->current == '"') { // string
        SCConsume(r, 1);
        value_begin = r->current;
        SCConsumeUntil(r, '"');
        value_end = r->current;

        if (!SCExpect(r, '"',
                      "Reached EOF without finding closing quote",
                      out)) { return; }
    } else if (IsDigitChar(*r->current)) { // number
        // NOTE(eric): We are using the honor system on number formatting
        value_begin = r->current;
        SCConsumeWhileFunc(r, IsDigitChar);
        value_end = r->current;
    } else { 
        SCError(r, "Expected parameter value but found something else", out);
        return;
    }

    out->keys[out->args_count]   = (Slice) {key_begin, key_end};
    out->values[out->args_count] = (Slice) {value_begin, value_end};
    out->args_count++;

    // Parameter read completed!
    SCConsumeWhitespace(r);
}

static void SCReadArgumentList(SCReader *r, SCObject *out) {
    if (!SCExpect(r, '(', 
             "Parser internal problem."
             "Tried to read param list but there is no (",
             out)) { return; }

    while (r->current < r->end) {
        // NOTE(eric): ReadArgument handles all of the consuming of whitespace
        // You can safely assume after this call that you are looking at
        // non-whitespace.
        SCReadArgument(r, out);
        if (out->type == SCObjectType_Error) { return; }

        // A comma after consuming the last argument
        // indicates an additional argument, otherwise,
        // we are done.
        if (*r->current == ',') { 
            SCConsume(r, 1);
        } else {
            break; 
        }
    }

    // Check for the closing paren
    if (!SCExpect(r, ')',
             "Parameter list is missing the closing paren",
             out)) { return; }
}

static void SCReadBlock(SCReader *r, SCObject *out) {
    if (!SCExpect(r, '{',
                  "Parser internal problem"
                  "Tried to read param list but there is no {",
                  out)) { return; }

    char *begin = r->current;
    int level = 1;
    while (r->current < r->end && level) {
        if (*r->current == '{') { 
            level++; 
        } else if (*r->current == '}') {
            level--;

            // Do not consume the last '}' yet
            if (!level) { break; }
        }

        SCConsume(r, 1);
    }
    char *end = r->current;

    if (level) {
        SCError(r, "Closing brace of block is missing", out);
    } else {
        SCConsume(r, 1);
        out->has_block = 1;
        out->block = (Slice) {begin, end};
    }
}

static void SCReadFunction(SCReader *r, SCObject *out) {
    out->type      = SCObjectType_Func,
    out->full_text.begin = r->current;

    if (!SCExpect(r, '\\', 
             "Parser internal problem. "
             "Tried to read function but there is no \\ at the start",
             out)) { return; }

    char *name_begin = r->current;
    SCConsumeWhileFunc(r, IsNameOrKeyChar);

    if (name_begin == r->current) {
        SCError(r, "Expected function name after backslash", out);
        return;
    }

    out->function_name = (Slice) {name_begin, r->current};

    if (r->current < r->end && *r->current == '(') {
        SCReadArgumentList(r, out);
        if (out->type == SCObjectType_Error) { return; }
    }

    if (r->current < r->end && *r->current == '{') {
        SCReadBlock(r, out);
        if (out->type == SCObjectType_Error) { return; }
    }

    out->end_line_no   = r->line_no;
    out->end_column_no = r->column_no;
    out->full_text.end = r->current;
}

static int IsNameOrKeyChar(int c) {
    return c == '_' || isalnum(c);
}

static int IsDigitChar(int ch) {
    return isdigit(ch) || ch == '.' || ch == '-';
}

int IsAllWhitespace(Slice text) {
    while (text.begin != text.end) {
        if (!isspace(*text.begin)) { return 0; }
        text.begin++;
    }

    return 1;
}

static void SCError(SCReader *r, const char *str, SCObject *out) {
    r->error = str;

    out->type           = SCObjectType_Error;
    out->end_line_no    = r->line_no;
    out->end_column_no  = r->column_no;
    out->full_text.end  = r->current;
    out->error_text     = str;

    if (!out->line_no)   { out->line_no   = r->line_no; }
    if (!out->column_no) { out->column_no = r->column_no; }
    if (!out->full_text.begin) { out->full_text.begin = r->current; }
}

// Consume characters from the input, updating the line/col numbers
static void SCConsume(SCReader *r, memsize chars) {
    while (r->current < r->end && chars--) {
        if (*r->current == '\n') {
            r->line_no++;
            r->column_no = 0;
        }

        r->current++;
        r->column_no++;
    }
}

static int SCExpect(SCReader *r, char c, const char *err, SCObject *out) {
    if (r->current >= r->end || *r->current != c) {
        SCError(r, err, out);
        return 0;
    } else {
        SCConsume(r, 1);
        return 1;
    }
}


// Consume until char is reached
static void SCConsumeUntil(SCReader *r, char c) {
    while (r->current < r->end && *r->current != c) {
        SCConsume(r, 1);
    }
}

// Consume while predicate function is true
static void SCConsumeWhileFunc(SCReader *r, int (*func)(int ch)) {
    while (r->current < r->end && func(*r->current)) {
        SCConsume(r, 1);
    }
}

static void SCConsumeWhitespace(SCReader *r) {
    SCConsumeWhileFunc(r, isspace);
}


#ifndef NDEBUG
#include <stdio.h>
#include <assert.h>
void TEST_SCReader(void) {
    printf("Testing SCReader\n");
    const char *test = 
        "Herp derp derp\n"
        "herp derp\\\\ derp\n"
        "\\foo\n"
        "asdf asdf asdf \\qwer asdf"
        "\\herp(foo=2, bar=\"qwer\"){woop woop}\n"
        "\\derp{qwer \nasdf zxcv}\n"
        "\n"
        "\n";

    SCReader reader = MakeSCReader(SliceFromCStr(test), "test_path", "test_file");
    SCObject obj;

    const int type_seq[] = {
        SCObjectType_Text,
        SCObjectType_Backslash,
        SCObjectType_Text,
        SCObjectType_Func,
        SCObjectType_Text,
        SCObjectType_Func,
        SCObjectType_Text,
        SCObjectType_Func,
        SCObjectType_Text,
        SCObjectType_Func,
        SCObjectType_Text,
        SCObjectType_End,
    };

    const char *name_seq[] = {
        0,
        0,
        0,
        "foo",
        0,
        "qwer",
        0,
        "herp",
        0,
        "derp",
        0,
        0,
    };

    int i = 0;
    do {
        SCRead(&reader, &obj);
        assert(i < ArrayCount(type_seq) && obj.type == type_seq[i]);

        if (obj.type == SCObjectType_Func) {
            assert(SliceEqCStr(obj.function_name, name_seq[i]));

            if (SliceEqCStr(obj.function_name, "herp")) {
                assert(obj.has_block);
                assert(obj.args_count == 2);
                assert(SliceEqCStr(obj.keys[0], "foo"));
                assert(SliceEqCStr(obj.keys[1], "bar"));
                assert(SliceEqCStr(obj.values[0], "2"));
                assert(SliceEqCStr(obj.values[1], "qwer"));
                assert(SliceEqCStr(obj.block, "woop woop"));
            } else if (SliceEqCStr(obj.function_name, "derp")) {
                assert(obj.has_block);
                assert(obj.args_count == 0);
            }
        }
        i++;
    } while (obj.type != SCObjectType_End &&
             obj.type != SCObjectType_Error);

    printf("Seems good.\n");
} 
#endif

