ag(1) -- The Silver Searcher. Like ack, but faster.
=============================================

## SYNOPSIS

`ag` [<options>] PATTERN [PATH]

## DESCRIPTION

Recursively search for PATTERN in PATH. Like grep or ack, but faster.

## OPTIONS

  * `--ackmate`:
    Output results in a format parseable by [AckMate](https://github.com/protocool/AckMate).
  * `-a --all-types`:
    Search all files. This doesn't include hidden files, and also doesn't respect any ignore files
  * `-A --after [LINES]`:
    Print lines before match. Defaults to 2.
  * `-B --before [LINES]`:
    Print lines after match. Defaults to 2.
  * `--[no]break`:
    Print a newline between matches in different files. Enabled by default.
  * `--[no]color`:
    Print color codes in results. Enabled by default.
  * `--column`:
    Print column numbers in results.
  * `-C --context [LINES]`:
    Print lines before and after matches. Defaults to 2.
  * `-D --debug`:
    Output ridiculous amounts of debugging info. Probably not useful.
  * `--depth NUM`:
    Search up to NUM directories deep. Default is 25.
  * `-f --follow`:
    Follow symlinks.
  * `--[no]group`
  * `-g PATTERN`:
    Print filenames matching PATTERN.
  * `-G`, `--file-search-regex PATTERN`:
    Only search filenames matching PATTERN.
  * `--[no]heading`
  * `--hidden`:
    Search hidden files. This option obeys ignore files.
  * `--ignore PATTERN`:
    Ignore files/directories matching this pattern. Literal file and directory names are also allowed.
  * `--ignore-dir NAME`:
    Alias for --ignore for compatibility with ack.
  * `-i --ignore-case`:
    Match case insensitively.
  * `-l --files-with-matches`:
    Only print filenames containing matches, not matching lines.
  * `-L --files-without-matches`:
    Only print filenames that don't contain matches.
  * `-m --max-count NUM`:
    Skip the rest of a file after NUM matches. Default is 10,000.
  * `-p --path-to-agignore STRING`:
    Provide a path to a specific .agignore file.
  * `--pager COMMAND`:
    Use a pager such as less. Use `--nopager` to override. This option is also ignored if output is piped to another program.
  * `--print-long-lines`:
    Print matches on very long lines (> 2k characters by default)
  * `-Q --literal`:
    Do not parse PATTERN as a regular expression. Try to match it literally.
  * `-s --case-sensitive`:
    Match case sensitively. Enabled by default.
  * `-S --smart-case`:
    Match case sensitively if there are any uppercase letters in PATTERN, or case insensitively otherwise.
  * `--search-binary`:
    Search binary files for matches.
  * `--stats`:
    Print stats (files scanned, time taken, etc)
  * `-t --all-text`:
    Search all text files. This doesn't include hidden files.
  * `-u --unrestricted`:
    Search *all* files. This ignores .agignore, .gitignore, etc. It searches binary and hidden files as well.
  * `-U --skip-vcs-ignores`:
    Ignore VCS ignore files (.gitigore, .hgignore, svn:ignore), but still use .agignore.
  * `-v --invert-match`
  * `-w --word-regexp`:
    Only match whole words.

## IGNORING FILES

By default, ag will ignore files matched by patterns in .gitignore, .hgignore, or .agignore. These files can be anywhere in the directories being searched. Ag also ignores files matched by the svn:ignore property in subversion repositories. Finally, ag looks in $HOME/.agignore for ignore patterns. Binary files are ignored by default as well.

If you want to ignore .gitignore, .hgignore, and svn:ignore but still take .agignore into account, use `-U`.

Use the `-t` option to search all text files, `-a` to search all files, and `-u` to search all including hidden files.

## EXAMPLES

`ag printf`:
  Find matches for "printf" in the current directory.

`ag foo /bar/`:
  Find matches for "foo" in path /bar/.

## SEE ALSO

grep(1)
