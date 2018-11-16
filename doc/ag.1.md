ag(1) -- The Silver Searcher. Like ack, but faster.
=============================================

## SYNOPSIS

`ag` [_options_] _pattern_ [_path ..._]

## DESCRIPTION

Recursively search for PATTERN in PATH. Like grep or ack, but faster.

## OPTIONS

  * `--ackmate`:
    Output results in a format parseable by [AckMate](https://github.com/protocool/AckMate).

  * `--[no]affinity`:
    Set thread affinity (if platform supports it). Default is true.

  * `-a --all-types`:
    Search all files. This doesn't include hidden files, and doesn't respect any ignore files.

  * `-A --after [LINES]`:
    Print lines after match. If not provided, LINES defaults to 2.

  * `-B --before [LINES]`:
    Print lines before match. If not provided, LINES defaults to 2.

  * `--[no]break`:
    Print a newline between matches in different files. Enabled by default.

  * `-c --count`:
    Only print the number of matches in each file.
    Note: This is the number of matches, **not** the number of matching lines.
    Pipe output to `wc -l` if you want the number of matching lines.

  * `--[no]color`:
    Print color codes in results. Enabled by default.

  * `--color-line-number`:
    Color codes for line numbers. Default is 1;33.

  * `--color-match`:
    Color codes for result match numbers. Default is 30;43.

  * `--color-path`:
    Color codes for path names. Default is 1;32.

  * `--column`:
    Print column numbers in results.

  * `-C --context [LINES]`:
    Print lines before and after matches. Default is 2.

  * `-D --debug`:
    Output ridiculous amounts of debugging info. Not useful unless you're actually debugging.

  * `--depth NUM`:
    Search up to NUM directories deep, -1 for unlimited. Default is 25.

  * `--[no]filename`:
    Print file names. Enabled by default, except when searching a single file.

  * `-f --[no]follow`:
    Follow symlinks. Default is false.

  * `-F --fixed-strings`:
    Alias for --literal for compatibility with grep.

  * `--[no]group`:
    The default, `--group`, lumps multiple matches in the same file
    together, and presents them under a single occurrence of the
    filename. `--nogroup` refrains from this, and instead places the
    filename at the start of each match line.

  * `-g PATTERN`:
    Print filenames matching PATTERN.

  * `-G --file-search-regex PATTERN`:
    Only search files whose names match PATTERN.

  * `-H --[no]heading`:
    Print filenames above matching contents.

  * `--hidden`:
    Search hidden files. This option obeys ignored files.

  * `--ignore PATTERN`:
    Ignore files/directories whose names match this pattern. Literal
    file and directory names are also allowed.

  * `--ignore-dir NAME`:
    Alias for --ignore for compatibility with ack.

  * `-i --ignore-case`:
    Match case-insensitively.

  * `-l --files-with-matches`:
    Only print the names of files containing matches, not the matching
    lines. An empty query will print all files that would be searched.

  * `-L --files-without-matches`:
    Only print the names of files that don't contain matches.

  * `--list-file-types`:
    See `FILE TYPES` below.

  * `-m --max-count NUM`:
    Skip the rest of a file after NUM matches. Default is 0, which never skips.

  * `--[no]mmap`:
    Toggle use of memory-mapped I/O. Defaults to true on platforms where
    `mmap()` is faster than `read()`. (All but macOS.)

  * `--[no]multiline`:
    Match regexes across newlines. Enabled by default.

  * `-n --norecurse`:
    Don't recurse into directories.

  * `--[no]numbers`:
    Print line numbers. Default is to omit line numbers when searching streams.

  * `-o --only-matching`:
    Print only the matching part of the lines.

  * `--one-device`:
    When recursing directories, don't scan dirs that reside on other storage
    devices. This lets you avoid scanning slow network mounts.
    This feature is not supported on all platforms.

  * `-p --path-to-ignore STRING`:
    Provide a path to a specific .ignore file.

  * `--pager COMMAND`:
    Use a pager such as `less`. Use `--nopager` to override. This option
    is also ignored if output is piped to another program.

  * `--parallel`:
    Parse the input stream as a search term, not data to search. This is meant
    to be used with tools such as GNU parallel. For example:
    `echo "foo\nbar\nbaz" | parallel "ag {} ."` will run 3 instances of ag,
    searching the current directory for "foo", "bar", and "baz".

  * `--print-long-lines`:
    Print matches on very long lines (> 2k characters by default).

  * `--passthrough --passthru`:
    When searching a stream, print all lines even if they don't match.

  * `-Q --literal`:
    Do not parse PATTERN as a regular expression. Try to match it literally.

  * `-r --recurse`:
    Recurse into directories when searching. Default is true.

  * `-s --case-sensitive`:
    Match case-sensitively.

  * `-S --smart-case`:
    Match case-sensitively if there are any uppercase letters in PATTERN,
    case-insensitively otherwise. Enabled by default.

  * `--search-binary`:
    Search binary files for matches.

  * `--silent`:
    Suppress all log messages, including errors.

  * `--stats`:
    Print stats (files scanned, time taken, etc).

  * `--stats-only`:
    Print stats (files scanned, time taken, etc) and nothing else.

  * `-t --all-text`:
    Search all text files. This doesn't include hidden files.

  * `-u --unrestricted`:
    Search *all* files. This ignores .ignore, .gitignore, etc. It searches
    binary and hidden files as well.

  * `-U --skip-vcs-ignores`:
    Ignore VCS ignore files (.gitignore, .hgignore), but still
    use .ignore.

  * `-v --invert-match`:
    Match every line *not* containing the specified pattern.

  * `-V --version`:
    Print version info.

  * `--vimgrep`:
    Output results in the same form as Vim's `:vimgrep /pattern/g`

    Here is a ~/.vimrc configuration example:

    `set grepprg=ag\ --vimgrep\ $*`
    `set grepformat=%f:%l:%c:%m`

    Then use `:grep` to grep for something.
    Then use `:copen`, `:cn`, `:cp`, etc. to navigate through the matches.

  * `-w --word-regexp`:
    Only match whole words.

  * `--workers NUM`:
    Use NUM worker threads. Default is the number of CPU cores, with a max of 8.

  * `-W --width NUM`:
    Truncate match lines after NUM characters.

  * `-z --search-zip`:
    Search contents of compressed files. Currently, gz and xz are supported.
    This option requires that ag is built with lzma and zlib.

  * `-0 --null --print0`:
    Separate the filenames with `\0`, rather than `\n`:
    this allows `xargs -0 <command>` to correctly process filenames containing
    spaces or newlines.


## FILE TYPES

It is possible to restrict the types of files searched. For example, passing
`--html` will search only files with the extensions `htm`, `html`, `shtml`
or `xhtml`. For a list of supported types, run `ag --list-file-types`.

## IGNORING FILES

By default, ag will ignore files whose names match patterns in .gitignore,
.hgignore, or .ignore. These files can be anywhere in the directories being
searched. Binary files are ignored by default as well. Finally, ag looks in
$HOME/.agignore for ignore patterns.

If you want to ignore .gitignore and .hgignore, but still take .ignore into
account, use `-U`.

Use the `-t` option to search all text files; `-a` to search all files; and `-u`
to search all, including hidden files.

## EXAMPLES

`ag printf`:
  Find matches for "printf" in the current directory.

`ag foo /bar/`:
  Find matches for "foo" in path /bar/.

`ag -- --foo`:
  Find matches for "--foo" in the current directory. (As with most UNIX command
  line utilities, "--" is used to signify that the remaining arguments should
  not be treated as options.)

## ABOUT

ag was originally created by Geoff Greer. More information (and the latest
release) can be found at http://geoff.greer.fm/ag

## SEE ALSO

grep(1)
