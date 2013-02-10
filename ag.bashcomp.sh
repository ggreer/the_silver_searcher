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
    --hidden
    --ignore
    --ignore-case
    --ignore-dir
    --invert-match
    --line-numbers
    --literal
    --max-count
    --no-numbers
    --path-to-agignore
    --print-long-lines
    --search-binary
    --skip-vcs-ignores
    --smart-case
    --stats
    --unrestricted
    --word-regexp
  '
  shtopt='
    -a -A -B -C -D
    -f -g -G -i -l
    -L -m -p -Q -s
    -S -t -u -U -v
    -w
  '

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
    --after|--before|--color-*|--context|--depth|--file-search-regex|--ignore|--max-count)
              return 0;;
  esac

  $split && return 0

  case "${cur}" in
    -*)
          if [[ "${COMP_CWORD}" -eq 1 ]] ; then
            COMPREPLY=( $(compgen -W \
              "${lngopt} ${shtopt}" -- "${cur}") )
          else
            COMPREPLY=( $(compgen -W \
              "${lngopt} ${shtopt}" -- "${cur}") )
          fi
          return 0;;
    *)
          _filedir
          return 0;;
  esac
} &&
complete -F _ag ${nospace} ag
