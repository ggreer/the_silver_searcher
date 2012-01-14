ag(1) -- The Silver Searcher. Like ack, but faster.
=============================================

## SYNOPSIS

`ag` [<options>] PATTERN PATH

## DESCRIPTION

Recursively search for PATTERN in PATH. Like grep or ack, but faster.

## OPTIONS

  * `--ackmate`:
    Output results in a format parseable by AckMate.
  * `-A --after` [LINES]:
    Print lines before match.
  * `-B --before` [LINES]:
    Print lines after match.
  * `--[no]break`:
    Print a newline between matches in different files. Enabled by default.
  * `--[no]color`:
    Print color codes in results. Enabled by default.
  * `--column`:
    Print column numbers in results.
  * `-C --context` [LINES]:
    Print lines before and after matches.
  * `-D --debug`:
    Ridiculous debugging. Probably not useful.
  * `-f --follow`:
    Follow symlinks.
  * `--[no]group`: 
  * `-G`, `--file-search-regex`
  * `-i`, `--ignore-case`
  * `--invert-match`
  * `--[no]heading`
  * `--literal`:
    Do not parse PATTERN as a regular expression. Try to match it literally.
  * `--search-binary`:
    Search binary files for matches.
  * `--stats`:
    Print stats (files scanned, time taken, etc)

## IGNORING FILES

Ag will ignore files matched by patterns in .gitignore, .hgignore, or .aginore. Ag also ignores binary files by default.

## EXAMPLES

`ag -i foo /bar/`:
  Find matches for "foo" in path /bar/.

## SEE ALSO

grep(1)
