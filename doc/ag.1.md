ag(1) -- The Silver Searcher. Like ack, but faster.
=============================================

## SYNOPSIS

`ag` [<file-type>] [<options>] PATTERN [PATH]

## DESCRIPTION

Recursively search for PATTERN in PATH. Like grep or ack, but faster.

## OPTIONS

  * `--ackmate`:
    Output results in a format parseable by [AckMate](https://github.com/protocool/AckMate).

  * `-a --all-types`:
    Search all files. This doesn't include hidden files, and doesn't respect any ignore files.

  * `-A --after [LINES]`:
    Print lines after match. Defaults to 2.

  * `-B --before [LINES]`:
    Print lines before match. Defaults to 2.

  * `--[no]break`:
    Print a newline between matches in different files. Enabled by default.

  * `-c --count`:
    Only print the number of matches in each file.
    Note: This is the number of matches, **not** the number of matching lines.
    Pipe output to `wc -l` if you want the number of matching lines.

  * `--[no]color`:
    Print color codes in results. Enabled by default.

  * `--color-line-number`:
    Color codes for line numbers. Defaults to 1;33.

  * `--color-match`:
    Color codes for result match numbers. Defaults to 30;43.

  * `--color-path`:
    Color codes for path names. Defaults to 1;32.

  * `--column`:
    Print column numbers in results.

  * `-C --context [LINES]`:
    Print lines before and after matches. Defaults to 2.

  * `-D --debug`:
    Output ridiculous amounts of debugging info. Probably not useful.

  * `--depth NUM`:
    Search up to NUM directories deep, -1 for unlimited. Default is 25.

  * `--[no]filename`:
    Print file names. Enabled by default, except when searching a single file.

  * `-f --follow`:
    Follow symlinks.

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
    Search hidden files. This option obeys ignore files.

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

  * `--[no]numbers`:
    Print line numbers. Default is to omit line numbers when searching streams.

  * `-o --only-matching`:
    Print only the matching part of the lines.
  
  * `-p --path-to-agignore STRING`:
    Provide a path to a specific .agignore file.

  * `--pager COMMAND`:
    Use a pager such as less. Use `--nopager` to override. This option
    is also ignored if output is piped to another program.

  * `--print-long-lines`:
    Print matches on very long lines (> 2k characters by default).

  * `--passthrough`:
    When searching a stream, print all lines even if they don't match.

  * `-Q --literal`:
    Do not parse PATTERN as a regular expression. Try to match it literally.

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

  * `-t --all-text`:
    Search all text files. This doesn't include hidden files.

  * `-u --unrestricted`:
    Search *all* files. This ignores .agignore, .gitignore, etc. It searches
    binary and hidden files as well.

  * `-U --skip-vcs-ignores`:
    Ignore VCS ignore files (.gitignore, .hgignore, svn:ignore), but still
    use .agignore.

  * `-v --invert-match`:
    Match every line *not* containing the specified pattern.

  * `--vimgrep`:
    Output results like vim's `:vimgrep /pattern/g` would (it reports every match on the line).   
    Here's a ~/.vimrc configuration example:

    `set grepprg=ag\ --vimgrep\ $*`   
    `set grepformat=%f:%l:%c:%m`

    Then use `:grep` to grep for something.   
    Then use `:copen`, `:cn`, `:cp`, etc. to navigate through the matches.

  * `-w --word-regexp`:
    Only match whole words.

  * `-z --search-zip`:
    Search contents of compressed files.

  * `-0 --null --print0`:
    Separate the filenames with `\0`, rather than `\n`:
    this allows `xargs -0 <command>` to correctly process filenames containing
    spaces or newlines.


## FILE TYPES

It is possible to restrict the types of files searched. For example, passing
`--html` as the `file-types` parameter will search only files with the
extensions `htm`, `html`, `shtml` or `xhtml`. For a list of supported `file-
types` run `ag --list-file-types`.

## IGNORING FILES

By default, ag will ignore files whose names match patterns in .gitignore,
.hgignore, or .agignore. These files can be anywhere in the directories being
searched. Ag also ignores files matched by the svn:ignore property if `svn
--version` is 1.6 or older.  Finally, ag looks in $HOME/.agignore for ignore
patterns. Binary files are ignored by default as well.

If you want to ignore .gitignore, .hgignore, and svn:ignore, but still take
.agignore into account, use `-U`.

Use the `-t` option to search all text files; `-a` to search all files; and `-u`
to search all, including hidden files.

## EXAMPLES

`ag printf`:
  Find matches for "printf" in the current directory.

`ag foo /bar/`:
  Find matches for "foo" in path /bar/.

## SEE ALSO

grep(1)
