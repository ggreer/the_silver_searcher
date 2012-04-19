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
    Search all files. This doesn't include hidden files.
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
  * `-G`, `--file-search-regex PATTERN`:
    Only search filenames matching PATTERN.
  * `--hidden`:
    Search hidden files. This option obeys ignore files.
  * `-i`, `--ignore-case`
  * `--[no]heading`
  * `-l --files-with-matches`:
    Only print filenames containing matches, not matching lines.
  * `--literal`:
    Do not parse PATTERN as a regular expression. Try to match it literally.
  * `-m --max-count NUM`:
    Skip the rest of a file after NUM matches. Default is 10,000.
  * `--print-long-lines`:
    Print matches on very long lines (> 2k characters by default)
  * `--search-binary`:
    Search binary files for matches.
  * `--stats`:
    Print stats (files scanned, time taken, etc)
  * `-u --unrestricted`:
    Search *all* files. This ignores .agignore, .gitignore, etc. It searches binary and hidden files as well.
  * `-v --invert-match`

## IGNORING FILES

Ag will ignore files matched by patterns in .gitignore, .hgignore, or .agignore. Ag also ignores binary files by default.

## EXAMPLES

`ag printf`:
  Find matches for "printf" in the current directory.

`ag foo /bar/`:
  Find matches for "foo" in path /bar/.

## SEE ALSO

grep(1)
