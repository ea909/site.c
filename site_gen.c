#include "site_gen.h"
#include "paths.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>

// Every SC file has an info command to provide the title and date for the
// page.
typedef struct SCInfo {
    Slice title;
    Slice date;
} SCInfo;

static int GetSCInfo(Slice sc, 
              const char *path, const char *file, 
              Arena *arena, SCInfo *out, Slice *error_text) {
    SCReader reader = MakeSCReader(sc, path, file);
    SCObject obj = {0};
    int      found_info = 0;

    do {
        SCRead(&reader, &obj);
        switch (obj.type) {
        default: break;
        R_HandleSCObjectTypeError(obj, arena, error_text);
        case SCObjectType_Func:
        {
            if (!SliceEqCStr(obj.function_name, "info")) { continue; }

            // This increments on:
            // Having a title.
            // Having a date.
            // If all the required stuff is here, it should == 2 by the end
            for (int i = 0; i < obj.args_count; i++) {
                if (SliceEqCStr(obj.keys[i], "title")) {
                    found_info++;
                    out->title = obj.values[i];
                } else if (SliceEqCStr(obj.keys[i], "date")) {
                    found_info++;
                    out->date = obj.values[i];
                }
            }

            if (found_info != 2) {
                *error_text = SCMakeErrorString(&obj, arena,
                                                "Info command is missing required params");   
                return 0;
            } else {
                return 1;
            }
        } break;
        }
    } while (obj.type != SCObjectType_End &&
             obj.type != SCObjectType_Error);

    *error_text = SCMakeErrorString(&obj, arena,
                                    "Info command not found");
    return 0;
}

// At the root of a site is a nav.sc file, which lists the toplevel nav links
// for the site + some extra info like the site title
typedef struct SiteNavigation {
    Slice site_title;
    Slice site_copyright;
    Slice site_footer;
    Slice links [SITE_NAVIGATION_MAX_ENTRIES];
    Slice labels[SITE_NAVIGATION_MAX_ENTRIES];
    int   nav_count;
    int   root_is_blog;
} SiteNavigation;

static void GenerateFooter(SiteNavigation *nav, Slice date, Arena *arena) {
    ArenaPushCStr(arena, 
        "    <footer>\n"
        "      <hr>\n"
        "      <p>\n");
    ArenaPushSlice(arena, nav->site_copyright);
    ArenaPushCStr(arena, "<br>");
    HTMLWriteEscapedText(date, arena);
    ArenaPushCStr(arena, "<br>");
    ArenaPushSlice(arena, nav->site_footer);
    ArenaPushCStr(arena, 
        "      </p>\n"
        "    </footer>\n"
        "  </body>\n"
        "</html>\n");
}

static void GenerateHeader(SiteNavigation *nav, 
                    Slice site_title, Slice site_sub_title, Slice title, 
                    Arena *arena) {
    ArenaPushCStr(arena, 
               "<!doctype html>\n"
               "<html lang=\"en\">\n"
               "  <head>\n"
               "    <meta charset=\"utf-8\">\n"
               "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n"
               "    <title>\n");
    HTMLWriteEscapedText(site_title, arena);

    if (!IsNullSlice(site_sub_title)) {
        ArenaPushCStr(arena, " - ");
        HTMLWriteEscapedText(site_sub_title, arena);
    }

    if (!IsNullSlice(title)) {
        ArenaPushCStr(arena, " - ");
        HTMLWriteEscapedText(title, arena);
    }

    ArenaPushCStr(arena, 
               "\n"
               "    </title>\n"
               "    <link rel=\"stylesheet\" href=\"/style.css\">\n"
               "  </head>\n"
               "  <body>\n"
               "    <header>\n"
               "      <h1>\n");
    HTMLWriteEscapedText(site_title, arena);

    if (!IsNullSlice(site_sub_title)) {
        ArenaPushCStr(arena, " <small> - ");
        HTMLWriteEscapedText(site_sub_title, arena);
        ArenaPushCStr(arena, "</small>");
    }

    ArenaPushCStr(arena, 
               "\n"
               "      </h1>\n"
               "      <nav>\n"
               "        <ul>\n");
    for (int i = 0; i < nav->nav_count; i++) {
        ArenaPushCStr(arena, " <li><a href=\"");
        ArenaPushSlice(arena, nav->links[i]);
        ArenaPushCStr(arena, "\">");
        ArenaPushSlice(arena, nav->labels[i]);
        ArenaPushCStr(arena, "</a></li>\n");
    }
    ArenaPushCStr(arena, 
               "        </ul>\n"
               "      </nav>\n"
               "      <hr>\n"
               "    </header>\n");
}

static int GenerateNormalPage(SiteNavigation *nav, 
                       Slice source, const char *path, const char *file, 
                       Arena *arena, Slice *out_slice) {
    ArenaString out_string = ArenaBeginString(arena);

    SCInfo info;
    if (!GetSCInfo(source, path, file, arena, &info, out_slice)) { return 0; }
    GenerateHeader(nav, nav->site_title, NullSlice(), info.title, arena);
    if (!SCToHTML(source, path, file, arena, out_slice)) { return 0; }
    GenerateFooter(nav, info.date, arena);
    *out_slice = ArenaEndString(arena, out_string);
    return 1;
}

// Changes a file's extension to .html, producing a null terminated cstr
static const char *SwitchExtension(Slice in_file_name, Arena *arena) {
    char *curr = in_file_name.begin;

    // TODO(eric): Need to search from end of string back
    while (curr != in_file_name.end && *curr != '.') {
        curr++;
    }

    char *out_begin = arena->current;
    ArenaPushSlice(arena, (Slice) {in_file_name.begin, curr});
    ArenaPushCStr(arena, ".html");
    ArenaPushChar(arena, 0);
    return out_begin;
}

typedef struct BlogEntry {
    Slice       title;
    Slice       date;
    const char *in_file_name;
    const char *out_file_name;
    Slice       file_text;
} BlogEntry;

static int BlogEntryCmp(const void *va, const void *vb) {
    BlogEntry *a = (BlogEntry*)va;
    BlogEntry *b = (BlogEntry*)vb;
    return SliceCmp(a->date, b->date);
}

// Generate a blog page. Unlike a normal page, a blog page has a second tier
// of navigation for moving between blog posts
static int GenerateBlogPage(SiteNavigation *nav, Slice site_title, Slice blog_title, 
                     const char *path, const char *file,
                     BlogEntry *prev, BlogEntry *entry, BlogEntry *next, 
                     Arena *arena, Slice *page_data) { 
    ArenaString out_string = ArenaBeginString(arena);

    GenerateHeader(nav, site_title, blog_title, entry->title, arena);

    ArenaPushCStr(arena,
               "<aside>\n"
               "  <nav>\n"
               "    <ul>\n"
               "     <div>\n");


    if (prev) {
        ArenaPushCStr(arena, "      <li><a href=\"");
        ArenaPushCStr(arena, prev->out_file_name);
        ArenaPushCStr(arena, "\">Prev</a></li>\n");
    } else {
        ArenaPushCStr(arena, "      <li>Prev</li>");
    }

    if (next) {
        ArenaPushCStr(arena, "      <li><a href=\"");
        ArenaPushCStr(arena, next->out_file_name);
        ArenaPushCStr(arena, "\">Next</a></li>\n");
    } else {
        ArenaPushCStr(arena, "      <li>Next</li>");
    }

    ArenaPushCStr(arena, 
                  "      </div><div><li><a href=\"archive.html\">Archive</a></li>\n");

    ArenaPushCStr(arena, "      <li><a href=\"");
    ArenaPushCStr(arena, entry->out_file_name);
    ArenaPushCStr(arena, 
                  "\">Permalink</a></li>\n");

    ArenaPushCStr(arena,
               "     </div>\n"
               "    </ul>\n"
               "  </nav>\n"
               "</aside>\n");

    if (!SCToHTML(entry->file_text, path, file, arena, page_data)) {
        return 0;
    }

    GenerateFooter(nav, entry->date, arena);

    *page_data = ArenaEndString(arena, out_string);
    return 1;
}

typedef struct Blog {
    Slice      title;
    BlogEntry  entries[SITE_BLOG_MAX_ENTRIES];
    int        entries_count;
} Blog;

static int GenerateBlogDirectory(const char *in_dir_absolute,
                 const char *out_dir_absolute,
                 SiteNavigation *nav,
                 Arena *arena,
                 Slice *error);
static int GenerateNormalDirectory(const char *in_dir_absolute, 
                      const char *out_dir_absolute,
                      SiteNavigation *nav,
                      Arena *arena,
                      Slice *error);

// Generate html files for all of the sc files in a directory
// Reads sc files from in_dir_absolute, and outputs them
// to out_dir_absolute.
static int GenerateDirectory(Slice dir_name,
                      const char *in_dir_absolute, 
                      const char *out_dir_absolute,
                      SiteNavigation *nav,
                      Arena *arena,
                      Slice *error) {
    if (SliceStartsWithCStr(dir_name, "blog_")) {
        return GenerateBlogDirectory(in_dir_absolute, out_dir_absolute, nav, arena, error);
    } else {
        return GenerateNormalDirectory(in_dir_absolute, out_dir_absolute, nav, arena, error);
    }
}

static int GenerateBlogDirectory(const char *in_dir_absolute,
                 const char *out_dir_absolute,
                 SiteNavigation *nav,
                 Arena *arena,
                 Slice *error) {
    ArenaPos original_arena_pos = ArenaSave(arena);
    DirIter *dir_iter           = ArenaPushDirIter(arena);
    Blog    *blog               = ArenaPush(arena, Blog);
    memset(blog, 0, sizeof(*blog));

    // Get the blog title from the blog.sc file
    ChangeDirectory(in_dir_absolute);
    Slice blog_file = {0};
    if (!ReadEntireFile("blog.sc", arena, &blog_file)) {
        *error = ArenaPrintf(arena, "Could not read file: blog.sc, "
                             "Does it exist?, every blog folder needs one\n"
                             "Path was: %s\n", in_dir_absolute);
        return 0;
    }

    SCReader reader = MakeSCReader(blog_file, in_dir_absolute, "blog.sc");
    SCObject obj = {0};
    do {
        SCRead(&reader, &obj);
        switch (obj.type) {
        case SCObjectType_Func:
        {
            if (SliceEqCStr(obj.function_name, "title")) {
                R_CheckSCObjectHasBlock(obj, "title", arena, error);
                blog->title = obj.block;
            } else {
                *error = ArenaPrintf(arena, "blog.sc file has unknown command\nPath was: %s\n",
                                     in_dir_absolute);
                return 0;
            }
        } break;
        R_HandleSCObjectTypeError(obj, arena, error);
        default: break;
        }
    } while (obj.type != SCObjectType_Error &&
             obj.type != SCObjectType_End);

    // Load all of the blog pages and generate sub directories
    MakeDirectory(out_dir_absolute);
    BeginDirIter(dir_iter, ".");
    while (GetNextFile(dir_iter)) {
        const char *file_name_cstr = GetFileName(dir_iter);
        Slice       file_name      = SliceFromCStr(file_name_cstr);

        if (IsDirectory(dir_iter)) { 
            if (SliceEqCStr(file_name, "."))      { continue; }
            if (SliceEqCStr(file_name, ".."))     { continue; }
            if (SliceEqCStr(file_name, "static")) { continue; }
            ArenaPos before_dir = ArenaSave(arena);
            const char *sub_in_dir  = MakePath(arena, in_dir_absolute,  file_name_cstr, 0);
            const char *sub_out_dir = MakePath(arena, out_dir_absolute, file_name_cstr, 0);
            if (!GenerateDirectory(file_name, sub_in_dir, sub_out_dir,
                                   nav, arena, error)) { goto dir_failure; }

            // IMPORTANT(eric): Need to move back to input dir after
            // generating sub-dir
            ChangeDirectory(in_dir_absolute);
            ArenaRestore(arena, before_dir);
            continue;
        }

        if (!SliceEndsWithCStr(file_name, ".sc")) { continue; }
        if (SliceEqCStr(file_name, "nav.sc"))     { continue; }
        if (SliceEqCStr(file_name, "archive.sc")) { continue; }
        if (SliceEqCStr(file_name, "blog.sc")) { continue; }
        if (SliceEqCStr(file_name, "index.sc")) { continue; }

        if (blog->entries_count >= SITE_BLOG_MAX_ENTRIES) {
            *error = ArenaPrintf(arena, "Blog has too many entries!");
            goto dir_failure;
        }

        Slice file_data = {0};
        if (!ReadEntireFile(file_name_cstr, arena, &file_data)) {
            *error = ArenaPrintf(arena, "Could not read file: %s\n", file_name_cstr);
            goto dir_failure;
        }

        SCInfo sc_info = {0};
        if (!GetSCInfo(file_data, in_dir_absolute, file_name_cstr, 
                       arena, &sc_info, error)) { goto dir_failure; }

        BlogEntry *entry = blog->entries + blog->entries_count;
        const char *out_file_name_cstr = SwitchExtension(file_name, arena);

        *entry = (BlogEntry) {
            .title         = sc_info.title,
            .date          = sc_info.date,
            .in_file_name  = ArenaCloneCStr(arena, file_name_cstr),
            .out_file_name = out_file_name_cstr,
            .file_text     = file_data,
        };

        blog->entries_count++;
    }

    EndDirIter(dir_iter);

    // Sort the blog pages
    qsort(blog->entries, 
          blog->entries_count, sizeof(*blog->entries),
          BlogEntryCmp);

    // Generate the blog pages, with ordered navigation links
    ChangeDirectory(out_dir_absolute);
    for (int i = 0; i < blog->entries_count; i++) {
        ArenaPos iter_pos = ArenaSave(arena);
        BlogEntry *entry  = blog->entries + i;
        BlogEntry *prev   = i > 0                      ? entry - 1 : 0;
        BlogEntry *next   = i < blog->entries_count - 1 ? entry + 1 : 0;

        Slice page_data = {0};
        if (!GenerateBlogPage(nav, 
                              nav->site_title, blog->title,
                              in_dir_absolute, entry->in_file_name, 
                              prev, entry, next, arena, &page_data)) {
            *error = page_data;
            return 0;
        }

        if (!WriteEntireFile(page_data, entry->out_file_name)) {
            *error = ArenaPrintf(arena, "Could not write file: %s\n", entry->out_file_name);
            return 0;
        }

        if (i == blog->entries_count-1) {
            if (!WriteEntireFile(page_data, "index.html")) {
                *error = ArenaPrintf(arena, "Could not write file: %s\n", "index.html");
                return 0;
            }
        }

        ArenaRestore(arena, iter_pos);
    }

    { // Generate archive page
        ArenaString str = ArenaBeginString(arena);
        ArenaPushSlice(arena, blog->title);
        ArenaPushCStr(arena, " - Archive");
        Slice blog_archive_title = ArenaEndString(arena, str);

        str = ArenaBeginString(arena);
        GenerateHeader(nav, 
                       nav->site_title, blog->title, SliceFromCStr("Archive"), 
                       arena);
        ArenaPushCStr(arena, "<article>\n");
        ArenaPushCStr(arena, "  <h1>\n");
        ArenaPushSlice(arena, blog_archive_title);
        ArenaPushCStr(arena, "  </h1>\n");
        ArenaPushCStr(arena, "    <ul>\n");

        for (int i = 0; i < blog->entries_count; i++) {
            ArenaPushCStr(arena, "<li><a href=\"");
            ArenaPushCStr(arena, blog->entries[i].out_file_name);
            ArenaPushCStr(arena, "\">");
            ArenaPushSlice(arena, blog->entries[i].date);
            ArenaPushCStr(arena, " - ");
            ArenaPushSlice(arena, blog->entries[i].title);
            ArenaPushCStr(arena, "</a></li>\n");
        }

        ArenaPushCStr(arena, "    </ul>");
        ArenaPushCStr(arena, "</article>\n");

        if (blog->entries_count) {
            GenerateFooter(nav, blog->entries[blog->entries_count-1].date, arena);
        } else {
            GenerateFooter(nav, SliceFromCStr(""), arena);
        }

        if (!WriteEntireFile(ArenaEndString(arena, str), "archive.html")) {
            *error = ArenaPrintf(arena, "Could not write file: %s\n", "archive.html");
            return 0;
        }
    }

    ArenaRestore(arena, original_arena_pos);
    return 1;
dir_failure:
    EndDirIter(dir_iter);
    return 0;
}

static int GenerateNormalDirectory(const char *in_dir_absolute, 
                      const char *out_dir_absolute,
                      SiteNavigation *nav,
                      Arena *arena,
                      Slice *error) {
    ArenaPos original_arena_pos = ArenaSave(arena);
    DirIter *dir_iter           = ArenaPushDirIter(arena);

    // Generate each file and each subdirectory
    MakeDirectory(out_dir_absolute);
    ChangeDirectory(in_dir_absolute);
    BeginDirIter(dir_iter, ".");
    while (GetNextFile(dir_iter)) {
        ArenaPos    iter_pos = ArenaSave(arena);
        const char *file_name_cstr = GetFileName(dir_iter);
        Slice       file_name      = SliceFromCStr(file_name_cstr);

        if (IsDirectory(dir_iter)) { 
            // Skip static directory + dir nav entries
            if (SliceEqCStr(file_name, "."))      { continue; }
            if (SliceEqCStr(file_name, ".."))     { continue; }
            if (SliceEqCStr(file_name, "static")) { continue; }

            // Generate sub-dir
            const char *sub_in_dir  = MakePath(arena, in_dir_absolute,  file_name_cstr, 0);
            const char *sub_out_dir = MakePath(arena, out_dir_absolute, file_name_cstr, 0);
            if (!GenerateDirectory(file_name, sub_in_dir, sub_out_dir,
                                   nav, arena, error)) { goto failure; }
        } else {
            // Skip nav.sc and non-sc files
            if (!SliceEndsWithCStr(file_name, ".sc")) { continue; }
            if (SliceEqCStr(file_name, "nav.sc"))    { continue; }

            Slice file_data = {0};
            Slice page_data = {0};

            // Read sc file
            ChangeDirectory(in_dir_absolute);
            if (!ReadEntireFile(file_name_cstr, arena, &file_data)) {
                *error = ArenaPrintf(arena, "Could not read file: %s\n", file_name_cstr);
                goto failure;
            }

            // Generate page
            if (!GenerateNormalPage(nav, 
                                    file_data, in_dir_absolute, file_name_cstr,
                                    arena, &page_data)) {
                *error = page_data;
                goto failure;
            }

            const char *out_file_name_cstr = SwitchExtension(file_name, arena);

            // Write it out
            ChangeDirectory(out_dir_absolute);
            if (!WriteEntireFile(page_data, out_file_name_cstr)) {
                *error = ArenaPrintf(arena, "Could not write file: %s\n", out_file_name_cstr);
                goto failure;
            }
        }

        ArenaRestore(arena, iter_pos);
    }

    // NOTE(eric): If an error happens, the error handling intentionally
    // returns early without resetting the arena, 
    // since the error message is stored in the arena.
    // After an error, the caller should reclaim used arena space itself if it
    // plans on continuing on.
    //
    // Otherwise, if there is no error, site generation leaves
    // no relevant memory behind, so the used arena data is released
    ArenaRestore(arena, original_arena_pos);
    EndDirIter(dir_iter);
    return 1;
failure:
    EndDirIter(dir_iter);
    return 0;
}

int GenerateSite(const char *in_dir_relative,
                 const char *out_dir_relative,
                 Arena *arena, 
                 Slice *error) {

    // First, we take the input paths
    // (in_dir_relative, out_directory_relative)
    // which /might/ be relative paths, and convert them
    // into guaranteed absolute paths. This way, you can
    // chdir to them w/out have to chdir back to the original
    // directory first.
    ArenaPos original_arena_pos = ArenaSave(arena);
    char *original_directory    = ArenaPushMany(arena, char, BUF_SIZE);
    char *in_dir_absolute       = ArenaPushMany(arena, char, BUF_SIZE);
    char *out_dir_absolute      = ArenaPushMany(arena, char, BUF_SIZE);
    CurrentDirectory(original_directory, BUF_SIZE);

    if (!ChangeDirectory(in_dir_relative)) {
        *error = ArenaPrintf(arena, "Could not change to input directory:\n%s\n",
                             in_dir_relative);
        return 0;
    } else {
        CurrentDirectory(in_dir_absolute, BUF_SIZE);
    }

    if (!ChangeDirectory(original_directory)) {
        *error = ArenaPrintf(arena, "Could not change to original directory:\n%s\n",
                             original_directory);
        return 0;
    }

    MakeDirectory(out_dir_relative);
    if (!ChangeDirectory(out_dir_relative)) {
        *error = ArenaPrintf(arena, "Could not change to output directory:\n%s\n",
                             out_dir_relative);
        return 0;
    } else {
        CurrentDirectory(out_dir_absolute, BUF_SIZE);
    }

    ChangeDirectory(in_dir_absolute);

    // Next, we read the nav.sc file. This will give us the name of the site,
    // and the list of navigation links shown at the top of each page.
    Slice nav_data = {0};
    if (!ReadEntireFile("nav.sc", arena, &nav_data)) {
        *error = ArenaPrintf(arena, "Could not read the nav file (nav.sc)" 
                                    " from the root of the input directory");
        return 0;
    }

    SiteNavigation nav    = {0};
    SCReader       reader = MakeSCReader(nav_data, in_dir_absolute, "nav.sc");
    SCObject       obj    = {0};
    int            found_title = 0;

    do {
        SCRead(&reader, &obj);
        switch (obj.type) {
        R_HandleSCObjectTypeError(obj, arena, error);

        case SCObjectType_Func: 
        {
            if (SliceEqCStr(obj.function_name, "root_is_blog")) {
                nav.root_is_blog = 1;
            } else if (SliceEqCStr(obj.function_name, "title")) {
                R_CheckSCObjectHasBlock(obj, "title", arena, error);
                nav.site_title = obj.block;
            } else if (SliceEqCStr(obj.function_name, "copyright")) {
                R_CheckSCObjectHasBlock(obj, "copyright", arena, error);
                nav.site_copyright = obj.block;
            } else if (SliceEqCStr(obj.function_name, "footer")) {
                R_CheckSCObjectHasBlock(obj, "footer", arena, error);
                nav.site_footer = obj.block;
            } else if (SliceEqCStr(obj.function_name, "nav")) {
                if (nav.nav_count >= SITE_NAVIGATION_MAX_ENTRIES) {
                    *error = SCMakeErrorString(&obj, arena, "Maximum nav count reached");
                    return 0;
                }

                int found_label = 0, found_link = 0;
                for (int i = 0; i < obj.args_count; i++) {
                    if (SliceEqCStr(obj.keys[i], "label")) {
                        found_label = 1;
                        nav.labels[nav.nav_count] = obj.values[i];
                    } else if (SliceEqCStr(obj.keys[i], "link")) {
                        found_link = 1;
                        nav.links[nav.nav_count] = obj.values[i];
                    }
                }

                if (!found_label || !found_link) {
                    *error = SCMakeErrorString(&obj, arena, 
                                               "nav command is missing label or link param");
                    return 0;
                }

                nav.nav_count++;
            }
        } break;
        default: break;
        }
    } while (obj.type != SCObjectType_End &&
             obj.type != SCObjectType_Error);

    // Generate the root directory
    int success = 0;
    if (nav.root_is_blog) {
        success = GenerateBlogDirectory(in_dir_absolute,
                               out_dir_absolute,
                               &nav,
                               arena, 
                               error);
    } else {
        success = GenerateNormalDirectory(in_dir_absolute,
                                       out_dir_absolute,
                                       &nav,
                                       arena,
                                       error);
    }

    // Copy the stylesheet and static directory
    ChangeDirectory(original_directory);
    if (success) { 
        CopyDirectory(in_dir_absolute, "static",
                      out_dir_absolute, "static",
                      arena);
        CopyFileToDir(in_dir_absolute, "style.css",
                 out_dir_absolute,
                 arena);
        ArenaRestore(arena, original_arena_pos); 
    }
    return success;
}

#ifndef NDEBUG
#include <stdio.h>
void TEST_GetSCInfo(void) {
    SCInfo info;
    Slice  error_text;
    memset(&info, 0, sizeof(info));

    printf("Testing reading SCInfo from a file\n");

    const char *test = 
        "herp derp derp\n"
        "\\other_command{QWER}\n"
        "\\info(title=\"Woop\", date=\"2018-04-02\")\n"
        " qwer qwer \n";
    Arena test_arena = AllocArena(ARENA_SIZE);

    printf("  This first test should succeed\n");
    assert(GetSCInfo(SliceFromCStr(test), "test_path", "test_file", &test_arena, &info, &error_text));
    printf("    Good\n");
    printf("    Title: "); SlicePrint(info.title); printf("\n");
    printf("    Date: ");  SlicePrint(info.date);  printf("\n");

    test = "\\info(foo=\"bar\")";

    printf("  This next test should fail\n");
    assert(!GetSCInfo(SliceFromCStr(test), "test_path", "test_file", &test_arena, &info, &error_text));
    printf("    Good\n");
    SlicePrint(error_text);
    printf("\n");

    test = "\\blah qwer";
    printf("  This next test should fail\n");
    assert(!GetSCInfo(SliceFromCStr(test), "test_path", "test_file", &test_arena, &info, &error_text));
    printf("    Good\n");
    SlicePrint(error_text);
    printf("\n");

    FreeArena(&test_arena);
    printf("Seems good.\n");
}

void TEST_GenerateNormalPage(void) {
    Arena test_arena = AllocArena(ARENA_SIZE);
    Slice result;

    SiteNavigation nav = {0};
    nav.site_title = SliceFromCStr("Awesome test site");
    nav.nav_count = 2;
    nav.links [0] = SliceFromCStr("http://zombo.com");
    nav.labels[0] = SliceFromCStr("zombo");
    nav.links [1] = SliceFromCStr("http://wombo.com");
    nav.labels[1] = SliceFromCStr("wombo");

    SCInfo info;
    info.title = SliceFromCStr("Page title");
    info.date = SliceFromCStr("Page date");

    printf("Testing GenerateNormalPage\n");
    printf("  GenerateNormalPage should fail this test:\n");
    int success = GenerateNormalPage(&nav, 
                                     SliceFromCStr("qwer \\boop qwer"), 
                                     "test_path", "test_file",
                                     &test_arena, &result);
    assert(!success);
    printf("    It did fail, error is:\n");
    SlicePrint(result);

    printf("  GenerateNormalPage should fail this test:\n");
    success = GenerateNormalPage(&nav, 
                                     SliceFromCStr(" \\info(title=\"zzz\", date=\"22\")"
                                                   " qwer "
                                                   " \\boop qwer"),
                                     "test_path",
                                     "test_file",
                                     &test_arena, &result);
    assert(!success);
    printf("    It did fail, error is:\n");
    SlicePrint(result);

    printf("  GenerateNormalPage should fail this test:\n");
    success = GenerateNormalPage(&nav, 
                                     SliceFromCStr("\\section{q} qwer \\info(title=\"zzz\", date=\"22\")"
                                                   " qwer "
                                                   " qwer "),
                                     "test_path",
                                     "test_file",
                                     &test_arena, &result);
    assert(!success);
    printf("    It did fail, error is:\n");
    SlicePrint(result);

    printf("  GenerateNormalPage should pass this test:\n");
    success = GenerateNormalPage(&nav, 
                                     SliceFromCStr(" \\info(title=\"zzz\", date=\"22\")"
                                                   " qwer "
                                                   " \\bold{woo} qwer"),
                                     "test_path", "test_file",
                                     &test_arena, &result);
    assert(success);
    printf("    It did pass, text is:\n");
    SlicePrint(result);

    FreeArena(&test_arena);
}
#endif

