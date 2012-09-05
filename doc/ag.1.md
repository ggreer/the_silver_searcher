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
    Search all files. This doesn't include hidden files, and doesn't respect any ignore files (except -p)
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
    Same as --[no]break --[no]heading
  * `-g PATTERN`:
    Print filenames matching PATTERN.
  * `-G`, `--file-search-regex PATTERN`:
    Only search filenames matching PATTERN.
  * `--[no]heading`
  * `--hidden`:
    Search hidden files. This option obeys ignore files.
  * `-i`, `--ignore-case`
  * `-l --files-with-matches`:
    Only print filenames containing matches, not matching lines.
  * `-L --files-without-matches`:
    Only print filenames that don't contain matches.
  * `-m --max-count NUM`:
    Skip the rest of a file after NUM matches. Default is 10,000.
  * `-p --path-to-agignore STRING`: Provide a path to a specific .agignore file
  * `--print-long-lines`:
    Print matches on very long lines (> 2k characters by default)
  * `-Q --literal`:
    Do not parse PATTERN as a regular expression. Try to match it literally.
  * `--search-binary`:
    Search binary files for matches.
  * `--stats`:
    Print stats (files scanned, time taken, etc)
  * `-u --unrestricted`:
    Search *all* files. This ignores .agignore, .gitignore, etc. It searches binary and hidden files as well.
  * `-v --invert-match`
  * `-w --word-regexp`:
    Only match whole words.

## IGNORING FILES

By default, ag will ignore files matched by patterns in .gitignore, .hgignore, or .agignore. Ag also ignores files matched by the svn:ignore property in subversion repositories. Binary files are ignored by default as well.

Use the `-a` option to search all text files, and `-u` to search *all* files.

## EXAMPLES

`ag printf`:
  Find matches for "printf" in the current directory.

`ag foo /bar/`:
  Find matches for "foo" in path /bar/.

## SEE ALSO

grep(1)
