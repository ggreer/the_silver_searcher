have ag &&
_ag() {
  local lngopt shtopt split=false
  local cur prev

  COMPREPLY=()
  cur=$(_get_cword "=")
  prev="${COMP_WORDS[COMP_CWORD-1]}"

  _expand || return 0

  lngopt='
    --ackmate
    --all-text
    --all-types
    --after
    --before
    --break
    --nobreak
    --case-sensitive
    --color-line-number
    --color-match
    --color-path
    --color
    --nocolor
    --column
    --context
    --debug
    --depth
    --file-search-regex
    --files-with-matches
    --files-without-matches
    --follow
    --group
    --nogroup
    --heading
    --noheading
    --help
    --hidden
    --ignore
    --ignore-case
    --ignore-dir
    --invert-match
    --line-numbers
    --list-file-types
    --literal
    --max-count
    --no-numbers
    --pager
    --nopager
    --parallel
    --path-to-agignore
    --print-long-lines
    --recurse
    --no-recurse
    --search-binary
    --search-files
    --search-zip
    --silent
    --skip-vcs-ignores
    --smart-case
    --stats
    --unrestricted
    --version
    --word-regexp
    --workers
  '
  shtopt='
    -a -A -B -C -D
    -f -g -G -h -i
    -l -L -m -n -p
    -Q -r -R -s -S
    -t -u -U -v -V
    -w -z
  '

  types=$(ag --list-file-types |grep -- '--')

  # these options require an argument
  if [[ "${prev}" == -@(A|B|C|G|g|m) ]] ; then
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
complete -F _ag ${nospace} ag
