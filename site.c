/*************************************************************************
Copyright 2018 Eric Alzheimer

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*****************************************************************************/

/** site.c - Single file static site generator 
 * Badly Drawn Squirrel (Eric Alzheimer) 2018
 * -------------------------------------------------- 
 *  This is written in C for entertainment purposes only :)
 */
/*
# About site.c 

site.c is yet another static site generator. It takes a directory
of files, written using it own LaTeX style markup syntax, and produces an
output directory of HTML files. It supports the generation of blogs, including the
creation of navigation links and archives. It generates HTML5 with semantic tags.

Supports Windows, Linux and maybe macOS (haven't tried it), and it requires no
dependencies other than the C compiler.

# site.c Users Guide

## Building site.c

You can compile site.c with clang, cl, or gcc with no special options or dependencies, ex:

    clang site.c -o site
    clang-cl -MT site.c 
    cl /MT site.c

## Running site.c

    Usage: site in_dir out_dir [memory]

site.c takes two required arguments:

* in_dir is the input directory full of SC source files.
* out_dir is the output directory to which the generated files are written. If it does
  not exist, it will be created.

Optionally, you can specify:

* memory, the amount of memory, in megabytes, used by the program. The default
  is 128 megabytes, which should be enough for any site. I doubt that the
  combined text of anyone's blog will exceed 128 mb. 

## SC File Format

site.c uses a custom file format with a command syntax similar to LaTeX. An SC
file consists of plain text with commands interspersed. All commands start
with a backslash followed by a command name. Ex:

    This is plain text. In the middle of the text, here is a commmand: \this_is_a_command
    And here is \another_command
    If you want to enter a backslash, just use two like \\
    There cannot be a space between the command name and the backslash.
    \ command <-- error
    Commands can contain any ascii symbol that a C identifier can contain.

A command may have parameters. Parameters are key-value pairs, with a keyword
for a key and either a string or number for the value. Ex:

    Here is a command with parameters: \command(arg1="String", arg2=2.22)
    Note that there cannot be a space between the command and the parenthesis.
    \command (foo="bar") <-- error

A command may have a block. A block is a (potentially) multiline collection of
text. Example:

    \command_with_block{ 
        This is text in a block
        This is text in a block
        This is text in a block
    }

    \command_with_block{single line block}

As with parenthesis, there cannot be a space between the command name and the
opening brace of the block. A command can have both parameters and a block:

    \command_with_both(arg="qwer", foo=2){this is the block}
    
## Site Input Directory Layout

There is only one required file in the input directory: nav.sc. This file sets
the website name, sets whether the input directory stores a blog directory or
a normal directory, and lists the toplevel navigation links for the site. See
the next section for a nav.sc example.

You can optionally have a style.css file, which will be copied to the output
directory. You can also have a `static` subdirectory. The `static` directory`
will not be processed by the site generator, but will be copied as-is into the
output directory.

You can have any number of subdirectories, recursively. Each subdirectory is
processed as either a normal directory or a blog directory. To make a
directory a blog, have its name begin with "blog_".

A blog directory must have a blog.sc file in it, which provides the name of
the blog.

### Example Directory Layout

In this example, the root directory contains a blog. There are two
sub-directories, one of which is a (separate) blog and the other of which 
has some pages to convert (normal directory)

    in_dir/
        static/
            my_cat.png
        style.css
        nav.sc
        blog.sc
        a_blog_entry.sc
        another_one.sc
        blog_cat_facts/
            blog.sc
            cat_facts_blog_entry_1.sc
            cat_facts_blog_entry_2.sc
        my_cat_pages/
            fluffy.sc
            spoofy.sc
            sporky.sc

After running site.c, you would get the following output files:

    out_dir/
        static/
            my_cat.png
        style.css
        index.html
        archive.html
        a_blog_entry.html
        another_one.html
        blog_cat_facts/
            index.html
            archive.html
            cat_facts_blog_entry_1.html
            cat_facts_blog_entry_2.html
        my_cat_pages/
            fluffy.html
            spoofy.html
            sporky.html

### Example `nav.sc` File

The only mandatory command is `\title`, but you will probably want at least
one `\nav` as well. 

    \title{Example Site Title}
    \copyright{&copy; Herp Derpington 2018}
    \footer{While you're here, check out my
            <a href="http://zombo.com">soundcloud</a>.}
    \root_is_blog

    \nav(link="/", label="Main Cat Blog")
    \nav(link="/blog_cat_facts", label="Cat Facts Blog")
    \nav(link="/my_cat_pages/fluffy.html",  label="Fluffy")
    \nav(link="/my_cast_pages/spoofy.html", label="Spoofy")

The `\root_is_blog` command is used to mark the root directory as a blog.
Without it, pages in the root directory are processed normally.

`\copyright` and `\footer` are output at the bottom of every page.

### Example `blog.sc` File

    \title{Cat Facts Blog}

A `blog.sc` file just names the blog.

### Page Files

All other files are page files which get converted into HTML. Every page file
must begin with an info command:

    \info(title="Title of the page", date="2018-08-08")

After that, page text and other commands can follow. If the page is in a blog
directory, the date is especially important, as it is used to sort the pages
for generating next/prev links and an archive.

## SC Page File Commands

An SC page file has two different types of commands: block commands and
inline commands. Inline commands are used in the midst of some text and modify
it without changing the document flow. That is, they do not create new
sections, begin or end lists, etc. For example, `bold` is a basic inline command:

    This is a line with \bold{some bolded text}. The inline command inserted
    the bold text in the middle of the sentence.

Block commands are used to demarcate the structure of the document. A block
command typically moves you out of your current paragraph, list item, etc,
into some subsequent section. For example, `section` is a block command:

    \section{Section One}

    This is text in a paragraph under the heading section one

    \section{Section Two}

    That text is followed by a "Section Two" heading and then this paragraph
    follows that

Some block commands are sub-block commands - they can only be used after a
block command that begins the correct type of section. Sub-block commands do not
make you leave your current section (ie, a paragraph, a list, a table) but
instead open some form of sub-section inside of it. `\item` is the main
example of a sub-block command:

    \section{Section One}

    This is the first paragraph in the section

    \paragraph

    We began a second paragraph in the section

    \unordered_list
        \item This is the first item in our list
        \item This is the second item in our list
    Note that `\\item` does not cause you to exit the unordered list like `\\paragraph` would.

    This text is still part of the second list item. Whitespace does not
    control the document structure.

    \paragraph

    This text is not part of the unordered list. The `\\paragraph` command is
    a block command, not a sub-clock command, so it closes the unordered list
    and begins a paragraph

### Inline Commands

* `\bold{<TEXT>}` - Makes the enclosed text bold
* `\italic{<TEXT>}` - Makes the enclosed text italic
* `\inline{<TEXT>}` - Makes the enclosed text monospaced
* `\link(url="<URL>"){<TEXT>}` - Turns the text into a link to the given url

### Block Commands

* `\section{<HEADING_TEXT>}` - Creates a new level one section with the
  given heading.
* `\subsection{<HEADING_TEXT>}` - Creates a new level two section with the
  given heading.
* `\paragraph` - Begins a new paragraph. Plain text after `\section` or
  `\subsection` automatically starts in its own new paragraph.
* `\ordered_list`, `\unordered_list`, `\horizontal_list` - All three of
  these commands start a new list. Horizontal list is a list that is
  supposed to be styled to layout horzontally instead of having one line
  per item.
* `\table` - Begins a table. Optionally, takes a table title/caption as a
  block: `\table{Table Caption/Title}`.
* `\html{<TEXT>}` - Outputs the block text as unescaped html.
* `\code{<TEXT>}` - Outputs the block text as a code block (`<pre><code>`)
* `\quote{<TEXT>}` - Outputs the block text as a block quote.
* `\image(url="<URL>", ...<HTMLAttributes>)` - Creates a centered image with the
  given url as source. Additional HTML img tag attributes can be passed
  through by including them in the parameter list. Ex: 
  `\image(url="/static/my_cat.png", width=120, height=120, title="MY CAT")`

### Sub-Block Commands

The main sub-block command is `\item`. It is used inside of a list
(`\unordered_list`, `\ordered_list`, or `\horizontal_list`) to start a new
list item. It is also used to start a new column in a table row.

The other sub-block commands are used inside of tables:

* `\row` - Starts a new row in the table.
* `\hitem` - Like `\item`, but used for a heading row in the table.

Some examples:

    \ordered_list
    \item One
    \item Two 
    \item Three

    \unordered_list
    \item One
    \item Two
    \item Three

    \table
    \row
    \hitem Column A
    \hitem Column B
    \hitem Column C
    \row
    \item 1
    \item 2
    \item 3
    \row \item 4 \item 5 \item 6 

## Blog Directory VS Normal Directory

Every directory is either a blog directory or a normal directory, and this
affects how pages are generated.

The root directory is marked as a blog with the `\root_is_blog` command in the
`nav.sc` file. Other directories are marked as blogs by having the directory
name start with the string "blog_".

If a directory is not a blog, each page file is converted into a corresponding
HTML file with the site header, the navigation header (as specified by the
                                                       `nav.sc`) and the
footer added to make a complete HTML5 file.

If the directory is a blog, each page is converted in the same way, except, in
addition to the main navigation header, there is another row of navigation
buttons added. These buttons are used to navigate the blog pages: "Prev",
"Next", "Archive", "Permalink". Pages are linked together with next
and prev links based on the dates specified in the info command for
each page.

Additionally, a blog auto-generates an `index.html` and and an `archive.html`. The
`index.html` file will be a copy of the most recent blog post. The archive
page will contain a chronological listing of all of the posts in the blog.

Remember, a blog requires a `blog.sc` file which contains the title of the
blog. This title is added as a subtitle to the site title.

## Memory

site.c uses a single large allocation of memory as stack allocator (aka arena).
Files are loaded into the arena and generated output is written to it. After
each file in a normal directory is processed, the memory used is rolled back
and reused. A blog directory loads all files at once, but then reuses memory
for each page generated.

The point is, unless you have a blog with > ~40mb of text in it, the default
is fine. Otherwise, use the third argument to request more memory.

Eventually, I'll change the arena implementation to grow as needed, but I
don't think I'll ever use more than 128mb.

## This file is too big!

This was a for-fun side project and one of the goals was to eliminate
dependencies from the site generation process. C makes it really annoying, due
to build systems and header files, to starting splitting things up, so I just
didn't do it. 

## Why didn't you use Markdown?

This is actually the second C site generator I've written. The first one did
have a markdown based generator. In the end, I did not like it. 
Markdown makes for nice
looking plain text, but it has a complex grammar with many unrelated syntaxes.
Because of all the symbols it uses, you end up escaping them all the time,
especially \_. I've escaped so many \_s in markdown!. 

Because of the complex grammar, adding your own features to the generator
involves finding some way to shoehorn it into the existing collection of
syntaxes. This inevitably results in adding more lexing and parsing - you
cannot just have a simple table or chain of ifs to dispatch each command or 
feature.

Because markdown is so particular, you end up needing to create another file
format for things like the navigation list (`nav.sc`). All of markdown's
syntax is for printed text - there is no way to include metadata commands in
the file.

Lastly, most markdown generator's I've seen do not output nice looking HTML.
Specifically, I want to use HTML5 semantic tags to create the document
hierarchy, whereas markdown just spits out h1, h2, h3, h4, and h5 tags.

Markdown's main benefit over other formats are it's terseness and the fact
that it is clean and readable as plaintext. Well, I can make something about
as terse, and no one will be reading the plain text source of a website,
they'll just read the website. So there is little gain there.

So, to overcome these issues, this site generator uses its own file format.
The style of the format is inspired by LaTeX (and that brings over some muscle
                                              memory from all the LaTeX
                                              documents I've typed). The LaTeX
command syntax is nice because it only requires the escaping of a single character
(backslash). I can use a single file format for both pages and metadata. I can
add commands without having to change the lexing/parsing. It spits out HTML5
with semantic tags.

*/
#define VERSION_STRING "0.1"

#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
typedef size_t   memsize;

#define ArrayCount(arr) (sizeof(arr)/sizeof(arr[0]))

// Main arena size for loading and writing files
#define ARENA_SIZE     (128 * 1024 * 1024) 
#define MIN_ARENA_SIZE (16 * 1024 * 1024)

// Used for paths, since they have to be null
// terminated thanks to the OS APIs
#define BUF_SIZE    4096 

// A Slice is a range of chars
// Used to represent strings most of the time.
// Because we know the full range, there is no null termination
typedef struct Slice {
    char *begin;
    char *end;
} Slice;

Slice MakeSlice(char *buffer, memsize size) { 
    return (Slice) {buffer, buffer + size}; 
}

Slice NullSlice(void) { 
    return (Slice) {0, 0};
}

int IsNullSlice(Slice s) { return s.begin == NULL; }

memsize SliceLength(Slice s) { 
    return (memsize)(s.end - s.begin); 
}

// "CStr" is the name in the project for a null terminated string (ie, a
// C-style String -> CStr)
Slice SliceFromCStr(const char *cstr) {
    return (Slice) {(char*)cstr, (char*)cstr + strlen(cstr)};
}

void SliceFPrint(Slice s, FILE *out) { fwrite(s.begin, sizeof(char), SliceLength(s), out); }
void SlicePrint(Slice s) { fwrite(s.begin, sizeof(char), SliceLength(s), stdout); }

// Compare two slices lexicographically, returns:
// >  0 -> a >  b
// == 0 -> a == b
// <  0 -> a <  b
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


// Returns true if the slice equals the given
// null terminated string
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

// An Arena is a basic linear allocator
// All memory is grabbed from an arena, and all strings
// are constructed by pushing onto it too.
typedef struct Arena {
    char *current;
    char *begin;
    char *end;
} Arena;

typedef void* ArenaPos;

Arena MakeArena(char *buffer, memsize size) {
    return (Arena) {buffer, buffer, buffer + size};
}

// Allocates `size` bytes from the arena
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

// These macros are used for most arena allocations. They take a type
// argument, generate a call to RawPushArena with the correct byte count, and
// cast the result to the requested type.
#define  ArenaPush(a, T) (T*)RawArenaPush((a), sizeof(T))
#define  ArenaPushMany(a, T, count) (T*)RawArenaPush((a), sizeof(T) * count)

void     ArenaReset(Arena *a) { a->current = a->begin; }
memsize  ArenaSpace(Arena *a) { return (memsize)(a->end - a->current); }

// Save and Restore allow arena space to be reused. You can mark a position in
// the arena, do a ton of work that involves allocation from the arena, and
// roll back in one instruction once its done.
ArenaPos ArenaSave(Arena *a) { return (void*)a->current; }
void     ArenaRestore(Arena *a, ArenaPos pos) { a->current = pos; }

// BeginString/EndString marks the start of a string and
// then produces a slice after building it with the Push functions below
typedef char* ArenaString;
ArenaString ArenaBeginString(Arena *a) { return a->current; }
Slice       ArenaEndString(Arena *a, ArenaString s) {
    return (Slice) {s, a->current};
}

Arena AllocArena(memsize size) {
    char *buffer = malloc(size);

    if (!buffer) {
        fprintf(stderr, "Could not allocate memory arena, size was %d", (int)size);
        exit(-1);
    }

    return MakeArena(buffer, size);
}

void FreeArena(Arena *a) { free(a->begin); }

////////////////////////////////////////////////
// Arena String Building functions.
// Use between ArenaBeginString/ArenaEndString
////////////////////////////////////////////////

void ArenaPushChar(Arena *a, char value) {
    *RawArenaPush(a, 1) = value;
}

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
    if (amt > ArenaSpace(a)-1) { amt = ArenaSpace(a)-1; }
    a->current += amt;
}

// Push the result of an sprintf into the arena
void ArenaPushf(Arena *a, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    ArenaPushfv(a, fmt, ap);
    va_end(ap);
}

// Allocates the result of a sprintf in the arena, returning
// a Slice of the result
Slice ArenaPrintf(Arena *a, const char *fmt, ...) {
    char *begin = a->current;

    va_list ap;
    va_start(ap, fmt);
    ArenaPushfv(a, fmt, ap);
    va_end(ap);

    return (Slice) {begin, a->current};
}

// Like the above, but makes a null terminated c string
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

// IMPORTANT(eric): The varargs for this function are null terminated!
// EX: MakePath(arena, "foo", "bar", 0);
// Don't forget it!
// Constructs a path out of a collection of sub-paths / parts.
// Arguments are a series of c-strs followed by 0 (null pointer terminator)
// For example (using unix paths):
//
// MakePath(arena, "/usr/", "local", "bin", 0) 
// returns "/usr/local/bin"
#ifdef _WIN32
#   define SC_PATH_SEP '\\'
#else
#   define SC_PATH_SEP '/'
#endif
const char *MakePath(Arena *arena, ...) {
    va_list ap;
    va_start(ap, arena);
    ArenaString path = ArenaBeginString(arena);

    char * str = va_arg(ap, char*);
    while (str) {
        ArenaPushCStr(arena, str);

        str = va_arg(ap, char*);

        // Need to add path separators
        if (str && arena->current[-1] != SC_PATH_SEP) {
            ArenaPushChar(arena, SC_PATH_SEP);
        }
    }

    ArenaPushChar(arena, 0);
    va_end(ap);

    return (const char *)path;
}

#ifdef _WIN32
#   include <windows.h>

// A DirIter is an iterator over the contents of a directory.
typedef struct DirIter {
    char               buf[BUF_SIZE];
    HANDLE             find_handle;
    WIN32_FIND_DATAA   find_data;
} DirIter;

// Sets up a directory iterator, which provides an
// iterator to access each item in the given path.
//
// This does not get the first item, it just sets things up.
// To get the first item, call GetNextItem
//
// ex:
/*
 * BeginDirIter(dir_iter, "folder");
 * while (GetNextFile(dir_iter) {
 *     ... do stuff with dir entry ...
 *  }
 *  EndDirIter(dir_iter);
 */
void BeginDirIter(DirIter *dir, const char *path) {
    memset(dir, 0, sizeof(*dir));
    memsize path_len = strlen(path);

    if (path[path_len-1] == '\\') {
        snprintf(dir->buf, BUF_SIZE, "%s*", path);
    } else {
        snprintf(dir->buf, BUF_SIZE, "%s\\*", path);
    }
}

// Get the next item in the directory
// Returns false if there is no next item, otherwise true
// Use IsDirectory, GetFileName to query the dir entry
int GetNextFile(DirIter *dir) {
    if (dir->find_handle == 0) {
        dir->find_handle = FindFirstFileA(dir->buf, &dir->find_data);
        return dir->find_handle != INVALID_HANDLE_VALUE;
    } else {
        return FindNextFileA(dir->find_handle, &dir->find_data);
    }
}

// True if current directory item is a directory
int IsDirectory(DirIter *dir) {
    return dir->find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
}

// Returns the file name of the current directory item.
// This string is stored in a buffer in the DirIter. As a result,
// GetNextFile or EndDirIter will invalidate this pointer.
const char *GetFileName(DirIter *dir) {
    return dir->find_data.cFileName;
}

// Make sure to call this to not leak HANDLEs
void EndDirIter(DirIter *dir) {
    FindClose(dir->find_handle);
}

// Change directory to the given path
// Can be relative or absolute.
// Returns false on failure.
int ChangeDirectory(const char *path) {
    return SetCurrentDirectoryA(path);
}

// Copy the current directories absolute path into the given buffer
// Null-terminated.
void CurrentDirectory(char *buf, memsize buf_len) {
    GetCurrentDirectoryA(buf_len, buf);
}

// Makes a directory. 
// Consider successful if the directory already exists
// Returns false on failure
int MakeDirectory(const char *path) {
    if (!CreateDirectoryA(path, NULL)) {
        DWORD error = GetLastError();
        if (error != ERROR_ALREADY_EXISTS) {
            return 0;
        }
    }

    return 1;
}

// Read whole file into arena, Slice *out points to file data
// Returns false on failure
int ReadEntireFile(const char *file_path, Arena *arena, Slice *out) {
    HANDLE file = CreateFileA(file_path, 
                              GENERIC_READ,
                              0, // share mode
                              0, // security
                              OPEN_EXISTING,
                              0, // file attr
                              0); // template

    if (file == INVALID_HANDLE_VALUE) { return 0; }

    LARGE_INTEGER val;
    memset(&val, 0, sizeof(val));

    if (!GetFileSizeEx(file, &val) || val.HighPart) { 
        CloseHandle(file);
        return 0; 
    }

    ArenaPos pos = ArenaSave(arena);
    char *buf = RawArenaPush(arena, val.LowPart);
    unsigned long bytes_read = 0;

    if (!ReadFile(file, buf, val.LowPart, &bytes_read, 0) || 
        bytes_read != val.LowPart) {
        CloseHandle(file);
        ArenaRestore(arena, pos);
        return 0;
    }

    *out = MakeSlice(buf, val.LowPart);
    CloseHandle(file);
    return 1;
}

// Write whole file from slice data.
// Returns false on failure
int WriteEntireFile(Slice data, const char *file_path) {
    HANDLE file = CreateFileA(file_path, 
                              GENERIC_WRITE,
                              0, // share mode
                              0, // security
                              CREATE_ALWAYS,
                              0, // file attr
                              0); // template

    if (file == INVALID_HANDLE_VALUE) { return 0; }
    memsize len = SliceLength(data);
    unsigned long bytes_read = 0;

    if (!WriteFile(file, data.begin, len, &bytes_read, 0) ||
        bytes_read != len) {
        CloseHandle(file);
        return 0;
    }

    CloseHandle(file);
    return 1;
}

// I still cannot find a good way to do these.
// There is SHFileOperationA, but it is apparently deprecated and 
// replaced by the IFileOperation COM object.

void CopyDirectory(const char *src_path, const char *src_name,
                   const char *dst_path, const char *dst_name, 
                   Arena *arena) {
    Arena a = *arena;
    const char *full_src = MakePath(arena, src_path, src_name, 0);
    const char *full_dst = MakePath(arena, dst_path, dst_name, 0);
    const char * cmd = ArenaPrintfCStr(arena,
                           "xcopy /Y /I /Q /E \"%s\" \"%s\"",
                           full_src,
                           full_dst);
    system(cmd);
}

void CopyFileToDir(const char *src_path, const char *src_name,
              const char *dst_path, Arena *arena) {
    Arena a = *arena;
    const char *full_src = MakePath(arena, src_path, src_name, 0);
    const char *full_dst = MakePath(arena, dst_path, src_name, 0);
    const char * cmd = ArenaPrintfCStr(arena,
                           "copy /Y \"%s\" \"%s\"",
                           full_src,
                           full_dst);
    system(cmd);
}

#else // Linux/Unix/macOS/POSIX
#   include <dirent.h>
#   include <unistd.h>
#   include <sys/stat.h>
// See windows impls for commentary
//
typedef struct DirIter {
    char buf[BUF_SIZE];
    DIR *dirp;
    struct dirent *dirent;
} DirIter;

void BeginDirIter(DirIter *dir, const char *path) {
    memset(dir, 0, sizeof(*dir));
    memsize path_len = strlen(path);
    snprintf(dir->buf, BUF_SIZE, "%s", path);
}

int GetNextFile(DirIter *dir) {
    if (!dir->dirp) {
        dir->dirp = opendir(dir->buf);

        if (!dir->dirp) {
            return 0;
        }
    }

    dir->dirent = readdir(dir->dirp);
    if (!dir->dirent) { return 0; }
    return 1;
}

int IsDirectory(DirIter *dir) {
    return dir->dirent->d_type & DT_DIR;
}

const char *GetFileName(DirIter *dir) {
    return dir->dirent->d_name;
}

void EndDirIter(DirIter *dir) {
    closedir(dir->dirp);
}

int ChangeDirectory(const char *path) {
    return chdir(path) == 0;
}

void CurrentDirectory(char *buf, memsize buf_len) {
    getcwd(buf, buf_len);
}

int MakeDirectory(const char *path) {
    return 0 == mkdir(path, 0777);
}

int ReadEntireFile(const char *file_path, Arena *arena, Slice *out) {
    FILE *file = fopen(file_path, "rb");
    if (!file) { return 0; }

    int start = ftell(file);
    if (start < 0) { goto failure; }
    if (fseek(file, 0, SEEK_END) < 0) { goto failure; }
    int end = ftell(file);
    if (end < 0) { goto failure; }
    if (fseek(file, 0, SEEK_SET) < 0) { goto failure; }
    memsize size  = (memsize)(end - start);

    char *buffer = ArenaPushMany(arena, char, size);

    memsize amt_read = fread(buffer, sizeof(char), size, file);

    if (amt_read != size) {
        goto failure;
    }

    fclose(file);
    *out = MakeSlice(buffer, size);
    return 1;

failure:
    fclose(file);
    return 0;
}

int WriteEntireFile(Slice data, const char *file_path) {
    FILE *file = fopen(file_path, "wb");
    if (!file) { 
        return 0; 
    }

    memsize size = SliceLength(data);
    memsize amt_written = fwrite(data.begin, sizeof(char), size, file);

    if (amt_written != size) {
        printf("FUCK\n");
        fclose(file);
        return 0;
    }

    fclose(file);
    return 1;
}

// meh
void CopyDirectory(const char *src_path, const char *src_name,
                   const char *dst_path, const char *dst_name, 
                   Arena *arena) {
    Arena a = *arena;
    const char *full_src = MakePath(arena, src_path, src_name, 0);
    const char *full_dst = MakePath(arena, dst_path, dst_name, 0);
    const char * cmd = ArenaPrintfCStr(arena,
                           "cp -R -T \"%s\" \"%s\"",
                           full_src,
                           full_dst);
    system(cmd);
}

void CopyFileToDir(const char *src_path, const char *src_name,
              const char *dst_path, Arena *arena) {
    Arena a = *arena;
    const char *full_src = MakePath(arena, src_path, src_name, 0);
    const char *full_dst = MakePath(arena, dst_path, src_name, 0);
    const char * cmd = ArenaPrintfCStr(arena,
                           "cp \"%s\" \"%s\"",
                           full_src,
                           full_dst);
    system(cmd);
}

#endif


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

const char *g_sc_object_type_name[SCObjectType_COUNT] = {
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

// Valid characters for function names or argument keys
int IsNameOrKeyChar(int c) {
    return c == '_' || isalnum(c);
}

// Matches chars that make up a decimal number
int IsDigitChar(int ch) {
    return isdigit(ch) || ch == '.' || ch == '-';
}

int IsAllWhitespace(Slice text) {
    while (text.begin != text.end) {
        if (!isspace(*text.begin)) { return 0; }
        text.begin++;
    }

    return 1;
}

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

// Fill out an SCObjectType_Error object with the given error message.
void SCError(SCReader *r, const char *str, SCObject *out) {
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
void SCConsume(SCReader *r, memsize chars) {
    while (r->current < r->end && chars--) {
        if (*r->current == '\n') {
            r->line_no++;
            r->column_no = 0;
        }

        r->current++;
        r->column_no++;
    }
}

// Checks for a character. Consumes if present and fails otherwise
int SCExpect(SCReader *r, char c, const char *err, SCObject *out) {
    if (r->current >= r->end || *r->current != c) {
        SCError(r, err, out);
        return 0;
    } else {
        SCConsume(r, 1);
        return 1;
    }
}

// Consume until char is reached
void SCConsumeUntil(SCReader *r, char c) {
    while (r->current < r->end && *r->current != c) {
        SCConsume(r, 1);
    }
}

// Consume while predicate function is true
void SCConsumeWhileFunc(SCReader *r, int (*func)(int ch)) {
    while (r->current < r->end && func(*r->current)) {
        SCConsume(r, 1);
    }
}

void SCConsumeWhitespace(SCReader *r) {
    SCConsumeWhileFunc(r, isspace);
}

// An argument is a key value pair with a 
// bareword key and either a string or number value.
//
// Ex:
// this_is_a_key = "This is a value"
// this_is_a_number = 22.3
//
// In a function:
// \example(arg1=22, arg2="string")
void SCReadArgument(SCReader *r, SCObject *out) {
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

// An argument list is a list of key value pairs (see SCReadArgument) wrapped
// in parens, ex:
// (arg1 = "this is a value", herparg = 33.6)
void SCReadArgumentList(SCReader *r, SCObject *out) {
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
void SCReadBlock(SCReader *r, SCObject *out) {
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
void SCReadFunction(SCReader *r, SCObject *out) {
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

// Read the next object from the SC file
//
// If an error occurs, an object of SCObjectType_Error is returned, and the
// same object is returned on all subsequent calls
//
// Once the end of the file is reached, an SCObjectType_End object is
// returned, and will be returned on all subsequent calls
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

// Convenience function to construct an error message with line number info. 
// If error_text is null, uses the error from the SCObject.
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

// Checks to ensure that a command has a block, returns with error if not
#define R_CheckSCObjectHasBlock(obj, name, arena, error_out_slice_ptr) \
    if (!(obj.has_block)) {                                           \
        *(error_out_slice_ptr) = SCMakeErrorString(&(obj), (arena),   \
                                                   name " commands require a block"); \
        return 0;                                                     \
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

#define SC_HTML_MAX_TAG_DEPTH 128

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

const char *g_html_tag_type_open_text[HTMLTagType_COUNT] = {
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

const char *g_html_tag_type_close_text[HTMLTagType_COUNT] = {
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

typedef struct HTMLTagStack {
    HTMLTagType stack[SC_HTML_MAX_TAG_DEPTH];
    int tag_pos;
    int section_depth;
    Arena *arena;
} HTMLTagStack;

void InitHTMLTagStack(HTMLTagStack *s, Arena *arena) {
    s->stack[0]      = HTMLTagType_TOS;
    s->tag_pos       = 0;
    s->section_depth = 0;
    s->arena         = arena;
}

HTMLTagType HTMLTop(HTMLTagStack *s) { return s->stack[s->tag_pos]; }

// Push a tag and print the opening tag
void HTMLPushTag(HTMLTagStack *s, HTMLTagType tag) {
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
HTMLTagType HTMLPopTag(HTMLTagStack *s) {
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

void HTMLRiseToLowestSection(HTMLTagStack *s) {
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

void HTMLWriteAttribute(Slice key, Slice value, Arena *arena) {
    ArenaPushChar(arena, ' ');
    ArenaPushSlice(arena, key);
    ArenaPushCStr(arena, "=\"");
    ArenaPushSlice(arena, value);
    ArenaPushChar(arena, '"');
}

// Create a new section at the specified heading level, and with the given
// heading text
void HTMLOpenSection(HTMLTagStack *s, int level, Slice heading) {
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
void HTMLWriteInTag(Slice text, const char *tag, Arena *arena) {
    ArenaPushf(arena, "<%s>\n", tag);
    HTMLWriteEscapedText(text, arena);
    ArenaPushf(arena, "</%s>\n", tag);
}

// Close tags until you reach the section tag, then open a new one
void HTMLOpenTag(HTMLTagStack *s, HTMLTagType tag) {
    HTMLRiseToLowestSection(s);
    HTMLPushTag(s, tag);
}

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
            // pointer table
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

// Every SC file has an info command to provide the title and date for the
// page.
typedef struct SCInfo {
    Slice title;
    Slice date;
} SCInfo;

int GetSCInfo(Slice sc, 
              const char *path, const char *file, 
              Arena *arena, SCInfo *out, Slice *error_text) {
    SCReader reader = MakeSCReader(sc, path, file);
    SCObject obj = {};
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
did_not_find_info:
    *error_text = SCMakeErrorString(&obj, arena,
                                    "Info command not found");
    return 0;
}

// At the root of a site is a nav.sc file, which lists the toplevel nav links
// for the site + some extra info like the site title
#define SITE_NAVIGATION_MAX_ENTRIES 32
typedef struct SiteNavigation {
    Slice site_title;
    Slice site_copyright;
    Slice site_footer;
    Slice links [SITE_NAVIGATION_MAX_ENTRIES];
    Slice labels[SITE_NAVIGATION_MAX_ENTRIES];
    int   nav_count;
    int   root_is_blog;
} SiteNavigation;

void GenerateFooter(SiteNavigation *nav, Slice date, Arena *arena) {
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

void GenerateHeader(SiteNavigation *nav, 
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

int GenerateNormalPage(SiteNavigation *nav, 
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
const char *SwitchExtension(Slice in_file_name, Arena *arena) {
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

#define SITE_BLOG_MAX_ENTRIES 4096

typedef struct BlogEntry {
    Slice       title;
    Slice       date;
    const char *in_file_name;
    const char *out_file_name;
    Slice       file_text;
} BlogEntry;

int BlogEntryCmp(const void *va, const void *vb) {
    BlogEntry *a = (BlogEntry*)va;
    BlogEntry *b = (BlogEntry*)vb;
    return SliceCmp(a->date, b->date);
}

// Generate a blog page. Unlike a normal page, a blog page has a second tier
// of navigation for moving between blog posts
int GenerateBlogPage(SiteNavigation *nav, Slice site_title, Slice blog_title, 
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

int GenerateBlogDirectory(const char *in_dir_absolute,
                 const char *out_dir_absolute,
                 SiteNavigation *nav,
                 Arena *arena,
                 Slice *error);
int GenerateNormalDirectory(const char *in_dir_absolute, 
                      const char *out_dir_absolute,
                      SiteNavigation *nav,
                      Arena *arena,
                      Slice *error);
int GenerateDirectory(Slice dir_name,
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

int GenerateBlogDirectory(const char *in_dir_absolute,
                 const char *out_dir_absolute,
                 SiteNavigation *nav,
                 Arena *arena,
                 Slice *error) {
    ArenaPos original_arena_pos = ArenaSave(arena);
    DirIter *dir_iter           = ArenaPush(arena, DirIter);
    Blog    *blog               = ArenaPush(arena, Blog);
    memset(blog, 0, sizeof(*blog));

    // Get the blog title from the blog.sc file
    ChangeDirectory(in_dir_absolute);
    Slice blog_file = {};
    if (!ReadEntireFile("blog.sc", arena, &blog_file)) {
        *error = ArenaPrintf(arena, "Could not read file: blog.sc, "
                             "Does it exist?, every blog folder needs one\n"
                             "Path was: %s\n", in_dir_absolute);
        return 0;
    }

    SCReader reader = MakeSCReader(blog_file, in_dir_absolute, "blog.sc");
    SCObject obj = {};
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

        Slice file_data = {};
        if (!ReadEntireFile(file_name_cstr, arena, &file_data)) {
            *error = ArenaPrintf(arena, "Could not read file: %s\n", file_name_cstr);
            goto dir_failure;
        }

        SCInfo sc_info = {};
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

        Slice page_data = {};
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

int GenerateNormalDirectory(const char *in_dir_absolute, 
                      const char *out_dir_absolute,
                      SiteNavigation *nav,
                      Arena *arena,
                      Slice *error) {
    ArenaPos original_arena_pos = ArenaSave(arena);
    DirIter *dir_iter           = ArenaPush(arena, DirIter);

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

            Slice file_data = {};
            Slice page_data = {};

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
    Slice nav_data = {};
    if (!ReadEntireFile("nav.sc", arena, &nav_data)) {
        *error = ArenaPrintf(arena, "Could not read the nav file (nav.sc)" 
                                    " from the root of the input directory");
        return 0;
    }

    SiteNavigation nav    = {};
    SCReader       reader = MakeSCReader(nav_data, in_dir_absolute, "nav.sc");
    SCObject       obj    = {};
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

    SiteNavigation nav = {};
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


int main(int argc, char **argv) {

#ifndef NDEBUG
    TEST_SCReader();
    TEST_SCToHTML();
    TEST_GetSCInfo();
    TEST_GenerateNormalPage();
#endif

    if (argc < 3) {
        printf("site.exe: simple static site generator version %s.\n", VERSION_STRING);
        printf("(c) Badly Drawn Squirrel Studios (Eric Alzheimer), 2018\n");
        printf("Released under the MIT license.\n");
        printf("Usage: site.exe in_directory out_directory [arena_size]\n");
        printf("  in_directory  - Directory containing site source data.\n");
        printf("  out_directory - Directory to generate site html into.\n");
        printf("                  Will create it if it doesn't exist.\n");
        printf("  memory - Amount of memory allocated, in megabytes, for loading and\n" 
               "           generating files. Default amount is 128.\n");
        return 0;
    }

    memsize arena_size = ARENA_SIZE;
    if (argc > 3) {
        arena_size = ((memsize)atoi(argv[3])) * 1024 * 1024;
        if (arena_size < MIN_ARENA_SIZE) {
            arena_size = MIN_ARENA_SIZE;
        }
    }

    Arena arena  = AllocArena(arena_size);
    Slice error;
    if (!GenerateSite(argv[1], argv[2], &arena, &error)) {
        fprintf(stderr, "Could not generate site, error happened:\n");
        SliceFPrint(error, stderr);
        return -1;
    }

    return 0;
}

