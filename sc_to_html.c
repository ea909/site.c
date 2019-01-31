#include "sc_to_html.h"
#include <assert.h>

typedef enum HTMLTagType {
    HTMLTagType_Article,
    HTMLTagType_Section,
    HTMLTagType_Paragraph,
    HTMLTagType_OrderedList,
    HTMLTagType_UnorderedList,
    HTMLTagType_HorizontalList,
    HTMLTagType_TableDiv, // hack to get tables to horiz scroll
    HTMLTagType_Table,
    HTMLTagType_ListItem,
    HTMLTagType_TableRow,
    HTMLTagType_TableColumn,
    HTMLTagType_TableHeadingColumn,
    HTMLTagType_TOS,
    HTMLTagType_COUNT,
} HTMLTagType;

typedef struct HTMLTagStack {
    HTMLTagType stack[SC_HTML_MAX_TAG_DEPTH];
    int tag_pos;
    int section_depth;
    Arena *arena;
} HTMLTagStack;

// Text used in <> brackets when opening a tag
static const char *g_html_tag_type_open_text[HTMLTagType_COUNT] = {
    "article",
    "section",
    "p",
    "ol",
    "ul",
    "ul class=\"horizlist\"",
    "div class=\"tablediv\"",
    "table",
    "li",
    "tr",
    "td",
    "th",
    "TOS",
};

// Text used in <> brackets when closing a tag
static const char *g_html_tag_type_close_text[HTMLTagType_COUNT] = {
    "article",
    "section",
    "p",
    "ol",
    "ul",
    "ul",
    "div",
    "table",
    "li",
    "tr",
    "td",
    "th",
    "TOS",
};

static void InitHTMLTagStack(HTMLTagStack *s, Arena *arena);
static HTMLTagType HTMLTop(HTMLTagStack *s);

// Push a tag onto the stack and print the opening tag
static void HTMLPushTag(HTMLTagStack *s, HTMLTagType tag);

// Pop the topmost tag and print the closing tag
static HTMLTagType HTMLPopTag(HTMLTagStack *s);

// Close tags until a Section or Article tag is reached
static void HTMLRiseToLowestSection(HTMLTagStack *s);

static void HTMLWriteAttribute(Slice key, Slice value, Arena *arena);

// Create a new section at the specified heading level, and with the given
// heading text
static void HTMLOpenSection(HTMLTagStack *s, int level, Slice heading);

// Write escaped text wrapped in a tag
static void HTMLWriteInTag(Slice text, const char *tag, Arena *arena);

// Close tags until you reach the section tag, then open a new one
static void HTMLOpenTag(HTMLTagStack *s, HTMLTagType tag);

// Converts the input sc file text into an html file.
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
    SCObject obj = {};
    SCReader reader = MakeSCReader(sc, path, file);
    HTMLTagStack tags = {};
    ArenaString out_string = ArenaBeginString(arena);
    InitHTMLTagStack(&tags, arena);
    HTMLPushTag(&tags, HTMLTagType_Article);

    do {
        SCRead(&reader, &obj);
        switch (obj.type) {
        R_HandleSCObjectTypeError(obj, arena, out_slice);
        case SCObjectType_End: break;
        default: break;

        case SCObjectType_Text:
        {
            // Open up an implicit paragraph if we start putting text right after a
            // section or article tag
            if ((HTMLTop(&tags) == HTMLTagType_Article ||
                HTMLTop(&tags) == HTMLTagType_Section) &&
                !IsAllWhitespace(obj.full_text)) {
                HTMLPushTag(&tags, HTMLTagType_Paragraph);
            }

            HTMLWriteEscapedText(obj.full_text, arena);
        } break;

        case SCObjectType_Backslash:
        {
            // Open up an implicit paragraph if we start putting text right after a
            // section or article tag
            if (HTMLTop(&tags) == HTMLTagType_Article ||
                HTMLTop(&tags) == HTMLTagType_Section) {
                HTMLPushTag(&tags, HTMLTagType_Paragraph);
            }

            ArenaPushChar(arena, '\\');
        } break;

        case SCObjectType_Func:
        {
            // NOTE(eric): Pro-Tip: If this gets too big, it should become a cstr -> function
            // pointer table...
            if (SliceEqCStr(obj.function_name, "section")) {
                R_CheckSCObjectHasBlock(obj, "section", arena, out_slice);
                HTMLOpenSection(&tags, 2, obj.block);
            } else if (SliceEqCStr(obj.function_name, "subsection")) {
                R_CheckSCObjectHasBlock(obj, "subsection", arena, out_slice);
                HTMLOpenSection(&tags, 3, obj.block);
            } else if (SliceEqCStr(obj.function_name, "paragraph")) {
                HTMLOpenTag(&tags, HTMLTagType_Paragraph);
            } else if (SliceEqCStr(obj.function_name, "ordered_list")) {
                HTMLOpenTag(&tags, HTMLTagType_OrderedList);
            } else if (SliceEqCStr(obj.function_name, "unordered_list")) {
                HTMLOpenTag(&tags, HTMLTagType_UnorderedList);
            } else if (SliceEqCStr(obj.function_name, "horizontal_list")) {
                HTMLOpenTag(&tags, HTMLTagType_HorizontalList);
            } else if (SliceEqCStr(obj.function_name, "table")) {
                HTMLOpenTag(&tags, HTMLTagType_TableDiv);
                HTMLPushTag(&tags, HTMLTagType_Table);

                if (obj.has_block) {
                    HTMLWriteInTag(obj.block, "caption", arena);
                }
            } else if (SliceEqCStr(obj.function_name, "item")) {
                if (HTMLTop(&tags) == HTMLTagType_ListItem    ||
                    HTMLTop(&tags) == HTMLTagType_TableColumn ||
                    HTMLTop(&tags) == HTMLTagType_TableHeadingColumn) {
                    HTMLPopTag(&tags);
                }

                HTMLTagType top = HTMLTop(&tags);
                if (top != HTMLTagType_UnorderedList  &&
                    top != HTMLTagType_OrderedList    &&
                    top != HTMLTagType_HorizontalList &&
                    top != HTMLTagType_TableRow) {
                    *out_slice = SCMakeErrorString(&obj, arena,
                        "You can only open an \\item in a table row or list");
                    return 0;
                } else if (top == HTMLTagType_TableRow) {
                    HTMLPushTag(&tags, HTMLTagType_TableColumn);
                } else {
                    HTMLPushTag(&tags, HTMLTagType_ListItem);
                }
            } else if (SliceEqCStr(obj.function_name, "hitem")) {
                if (HTMLTop(&tags) == HTMLTagType_TableColumn ||
                    HTMLTop(&tags) == HTMLTagType_TableHeadingColumn) {
                    HTMLPopTag(&tags);
                }

                HTMLTagType top = HTMLTop(&tags);
                if (top != HTMLTagType_TableRow) {
                    *out_slice = SCMakeErrorString(&obj, arena,
                        "You can only open an \\hitem in a table row");
                    return 0;
                }

                HTMLPushTag(&tags, HTMLTagType_TableHeadingColumn);
            } else if (SliceEqCStr(obj.function_name, "row")) {
                if (HTMLTop(&tags) == HTMLTagType_TableHeadingColumn ||
                    HTMLTop(&tags) == HTMLTagType_TableColumn) {
                    HTMLPopTag(&tags); 
                }

                if (HTMLTop(&tags) == HTMLTagType_TableRow) { 
                    HTMLPopTag(&tags); 
                }

                if (HTMLTop(&tags) != HTMLTagType_Table) { 
                    *out_slice = SCMakeErrorString(&obj, arena, 
                                      "You can only open a \\row in a table");
                    return 0;
                }

                HTMLPushTag(&tags, HTMLTagType_TableRow);
            } else if (SliceEqCStr(obj.function_name, "html")) {
                R_CheckSCObjectHasBlock(obj, "html", arena, out_slice);
                HTMLRiseToLowestSection(&tags);
                ArenaPushSlice(arena, obj.block);
            } else if (SliceEqCStr(obj.function_name, "code")) {
                R_CheckSCObjectHasBlock(obj, "code", arena, out_slice);
                HTMLRiseToLowestSection(&tags);
                ArenaPushCStr(arena, "<pre><code>");
                Slice text = obj.block;

                // Need to get rid of the first newline, ie:
                // \code{
                // FunctionCall();
                // }
                // There is a newline there before "FunctionCall". If not
                // removed, it will be shown since code uses a pre block.
                while (text.begin != text.end) {
                    if (*text.begin == '\n') {
                        text.begin++; // Move one past first newline
                        break;
                    } else {
                        text.begin++;
                    }
                }

                HTMLWriteEscapedText(obj.block, arena);
                ArenaPushCStr(arena, "</code></pre>\n");
            } else if (SliceEqCStr(obj.function_name, "quote")) {
                R_CheckSCObjectHasBlock(obj, "quote", arena, out_slice);
                HTMLRiseToLowestSection(&tags);
                HTMLWriteInTag(obj.block, "blockquote", arena);
            } else if (SliceEqCStr(obj.function_name, "bold")) {
                R_CheckSCObjectHasBlock(obj, "bold", arena, out_slice);
                HTMLWriteInTag(obj.block, "b", arena);
            } else if (SliceEqCStr(obj.function_name, "italic")) {
                R_CheckSCObjectHasBlock(obj, "italic", arena, out_slice);
                HTMLWriteInTag(obj.block, "i", arena);
            } else if (SliceEqCStr(obj.function_name, "inline")) {
                R_CheckSCObjectHasBlock(obj, "inline", arena, out_slice);
                HTMLWriteInTag(obj.block, "code", arena);
            } else if (SliceEqCStr(obj.function_name, "link")) {
                R_CheckSCObjectHasBlock(obj, "link", arena, out_slice);

                int found_url = 0;
                ArenaPushCStr(arena, "<a");

                for (int i = 0; i < obj.args_count; i++) {
                    if (SliceEqCStr(obj.keys[i], "url")) {
                        found_url = 1;
                        HTMLWriteAttribute(SliceFromCStr("href"), obj.values[i], arena);
                    } else {
                        HTMLWriteAttribute(obj.keys[i], obj.values[i], arena);
                    }
                }

                if (!found_url) {
                    *out_slice = SCMakeErrorString(&obj, arena,
                                     "Missing required url parameter in link");
                    return 0;
                }

                ArenaPushCStr(arena, ">");
                HTMLWriteEscapedText(obj.block, arena);
                ArenaPushCStr(arena, "</a>");
            } else if (SliceEqCStr(obj.function_name, "image")) {
                int found_url = 0;
                HTMLRiseToLowestSection(&tags);
                ArenaPushCStr(arena, "<img");

                for (int i = 0; i < obj.args_count; i++) {
                    if (SliceEqCStr(obj.keys[i], "url")) {
                        found_url = 1; 
                        HTMLWriteAttribute(SliceFromCStr("src"), obj.values[i], arena);
                    } else {
                        HTMLWriteAttribute(obj.keys[i], obj.values[i], arena);
                    }
                }

                if (!found_url) {
                    *out_slice = SCMakeErrorString(&obj, arena,
                                     "Missing required url parameter in image");
                    return 0;
                }

                ArenaPushCStr(arena, ">\n");
            } else if (SliceEqCStr(obj.function_name, "info")) {
                HTMLRiseToLowestSection(&tags);

                if (HTMLTop(&tags) != HTMLTagType_Article) {
                    *out_slice = SCMakeErrorString(&obj, arena,
                               "Info command should be at the beginning of the file");
                    return 0;
                }

                for (int i = 0; i < obj.args_count; i++) {
                    if (!SliceEqCStr(obj.keys[i], "title")) { continue; }
                    HTMLWriteInTag(obj.values[i], "h1", arena);
                }
            } else {
                *out_slice = SCMakeErrorString(&obj, arena, "Unknown command");
                return 0;
            }
        } break;
        }
    } while (obj.type != SCObjectType_End &&
             obj.type != SCObjectType_Error);

    while (HTMLTop(&tags) != HTMLTagType_TOS) {
        HTMLPopTag(&tags);
    }

    *out_slice = ArenaEndString(arena, out_string);
    return 1;
}




static HTMLTagType HTMLTop(HTMLTagStack *s) { 
    return s->stack[s->tag_pos]; 
}

static void InitHTMLTagStack(HTMLTagStack *s, Arena *arena) {
    s->stack[0]      = HTMLTagType_TOS;
    s->tag_pos       = 0;
    s->section_depth = 0;
    s->arena         = arena;
}

// Push a tag and print the opening tag
static void HTMLPushTag(HTMLTagStack *s, HTMLTagType tag) {
    assert(s->tag_pos + 1 < SC_HTML_MAX_TAG_DEPTH);
    ArenaPushf(s->arena, "<%s>\n", g_html_tag_type_open_text[tag]);
    s->stack[++s->tag_pos] = tag;

    // The number of sections is tracked so that subsection/section can
    // find the right place in the stack to rise to before pushing their tag
    if (tag == HTMLTagType_Article ||
        tag == HTMLTagType_Section) {
        s->section_depth++;
    }
}

// Pop the topmost tag and print the closing tag
static HTMLTagType HTMLPopTag(HTMLTagStack *s) {
    assert(s->tag_pos > 0);
    HTMLTagType tag = s->stack[s->tag_pos--];
    ArenaPushf(s->arena, "</%s>\n", g_html_tag_type_close_text[tag]);

    // The number of sections is tracked so that subsection/section can
    // find the right place in the stack to rise to before pushing their tag
    if (tag == HTMLTagType_Article ||
        tag == HTMLTagType_Section) {
        s->section_depth--;
    }

    return tag;
}

// Close tags until a Section or Article tag is reached
static void HTMLRiseToLowestSection(HTMLTagStack *s) {
    while (HTMLTop(s) != HTMLTagType_Section &&
           HTMLTop(s) != HTMLTagType_Article) {
        HTMLPopTag(s);
    }
}

// NOTE(eric): This escape function only contains characters I've actually
// used.
// Writes text to the arena, escaping html special characters
void HTMLWriteEscapedText(Slice text, Arena *arena) {
    while (text.begin != text.end) {
        switch (*text.begin) {
        case '"': ArenaPushCStr(arena, "&quot;"); break;
        case '&': ArenaPushCStr(arena, "&amp;"); break;
        case '<': ArenaPushCStr(arena, "&lt;"); break;
        case '>': ArenaPushCStr(arena, "&gt;"); break;
        default: ArenaPushChar(arena, *text.begin); break;
        }

        text.begin++;
    }
}

static void HTMLWriteAttribute(Slice key, Slice value, Arena *arena) {
    ArenaPushChar(arena, ' ');
    ArenaPushSlice(arena, key);
    ArenaPushCStr(arena, "=\"");
    ArenaPushSlice(arena, value);
    ArenaPushChar(arena, '"');
}

// Create a new section at the specified heading level, and with the given
// heading text
static void HTMLOpenSection(HTMLTagStack *s, int level, Slice heading) {
    // Rise up to Article level
    while (s->section_depth > level-1) { HTMLPopTag(s); }

    // Make the section
    while (s->section_depth != level) {
        HTMLPushTag(s, HTMLTagType_Section);
    }

    ArenaPushCStr(s->arena, "<h1>");
    HTMLWriteEscapedText(heading, s->arena);
    ArenaPushCStr(s->arena, "</h1>\n");
}

// Write escaped text wrapped in a tag
static void HTMLWriteInTag(Slice text, const char *tag, Arena *arena) {
    ArenaPushf(arena, "<%s>\n", tag);
    HTMLWriteEscapedText(text, arena);
    ArenaPushf(arena, "</%s>\n", tag);
}

// Close tags until you reach the section tag, then open a new one
static void HTMLOpenTag(HTMLTagStack *s, HTMLTagType tag) {
    HTMLRiseToLowestSection(s);
    HTMLPushTag(s, tag);
}

#ifndef NDEBUG
#include <stdio.h>
void TEST_SCToHTML(void) {
    printf("Testing the SCToHTML function\n");
    printf("  This first test should fail!\n");
    Arena test_arena = AllocArena(ARENA_SIZE);
    const char *test = 
        "herp derp\n"
        "\\bold($$$)";
    Slice result;
    int failure = !SCToHTML(SliceFromCStr(test), "test_path", "test_file", &test_arena, &result);

    assert(failure);
    printf("    It failed with error string:\n");
    SlicePrint(result);

    test = 
        "\\section{hello}\n"
        "poopidiscoop\n"
        "\\bold{scoopidiwoop}\n"
        "woop di scoop di poop\n"
        "\\ordered_list\n"
        "\\item Qwer Asdf\n"
        "\\item asdf Asdf\n"
        "\\item xczv Asdf\n"
        "\\unordered_list\n"
        "\\item \\bold{1Qwer} Asdf\n"
        "\\item 1asdf Asdf\n"
        "\\item 1xczv Asdf\n"
        "\\table{1xczv Asdf}\n"
        "\\row \\item herp \\item derp"
        "\\row \\item herp \\item derp\n"
        "\\row \\item herp \\item derp"
        "\\paragraph\n"
        "qwer qwer qwer\n"
        "asdf asdf asdf\n"
        "\\section{QWER QWER}\n"
        "\\link(url=\"zombo.com\"){Zombo com link here}\n"
        "\\image(url=\"poop.jpg\", width=640, height=480)\n"
        "\\subsection{QWER QWER}\n"
        "\\html{<div>\n"
        "derp\n"
        "</div>\n"
        "}\n"
        "\\code{<div>\n"
        "derp\n"
        "</div>\n"
        "}\n"
        "\\quote{<div>\n"
        "derp\n"
        "</div>\n"
        "}\n";

    printf("  This next test should fail\n");
    failure = !SCToHTML(SliceFromCStr(test), "test_path", "test_file", &test_arena, &result);
    assert(!failure);
    printf("    It did not fail\n");
    printf("    Here is the text:\n");
    SlicePrint(result);

    printf("Seems good.\n");
    FreeArena(&test_arena);
}
#endif



