# About site.c 

site.c is yet another static site generator. It takes a directory
of files, written using it own LaTeX style markup syntax, and produces an
output directory of HTML files. It supports the generation of blogs, including the
creation of navigation links and archives. It generates small, minimalist HTML5 with semantic tags.

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
the later section for a nav.sc example.

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

