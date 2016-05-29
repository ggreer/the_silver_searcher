_ag() {
  local lngopt shtopt split=false
  local cur prev

  COMPREPLY=()
  cur=$(_get_cword "=")
  prev="${COMP_WORDS[COMP_CWORD-1]}"

  _expand || return 0

  lngopt='
    --ackmate
    --ackmate-dir-filter
    --affinity
    --after
    --all-text
    --all-types
    --before
    --break
    --case-sensitive
    --color
    --color-line-number
    --color-match
    --color-path
    --color-win-ansi
    --column
    --context
    --count
    --debug
    --depth
    --file-search-regex
    --filename
    --files-with-matches
    --files-without-matches
    --fixed-strings
    --follow
    --group
    --heading
    --help
    --hidden
    --ignore
    --ignore-case
    --ignore-dir
    --invert-match
    --line-numbers
    --list-file-types
    --literal
    --match
    --max-count
    --no-numbers
    --no-recurse
    --noaffinity
    --nobreak
    --nocolor
    --nofilename
    --nofollow
    --nogroup
    --noheading
    --nonumbers
    --nopager
    --norecurse
    --null
    --numbers
    --one-device
    --only-matching
    --pager
    --parallel
    --passthrough
    --passthru
    --path-to-agignore
    --print-long-lines
    --print0
    --recurse
    --search-binary
    --search-files
    --search-zip
    --silent
    --skip-vcs-ignores
    --smart-case
    --stats
    --unrestricted
    --version
    --vimgrep
    --word-regexp
    --workers
  '
  shtopt='
    -a -A -B -C -D
    -f -F -g -G -h
    -i -l -L -m -n
    -p -Q -r -R -s
    -S -t -u -U -v
    -V -w -z
  '

  types=$(ag --list-file-types |grep -- '--')

  # these options require an argument
  if [[ "${prev}" == -[ABCGgm] ]] ; then
    return 0
  fi

  _split_longopt && split=true

  case "${prev}" in
    --ignore-dir) # directory completion
              _filedir -d
              return 0;;
    --path-to-agignore) # file completion
              _filedir
              return 0;;
    --pager) # command completion
              COMPREPLY=( $(compgen -c -- "${cur}") )
              return 0;;
    --ackmate-dir-filter|--after|--before|--color-*|--context|--depth\
    |--file-search-regex|--ignore|--max-count|--workers)
              return 0;;
  esac

  $split && return 0

  case "${cur}" in
    -*)
          COMPREPLY=( $(compgen -W \
            "${lngopt} ${shtopt} ${types}" -- "${cur}") )
          return 0;;
    *)
          _filedir
          return 0;;
  esac
} &&

# shellcheck disable=SC2086
# shellcheck disable=SC2154,SC2086
complete -F _ag ${nospace} ag
