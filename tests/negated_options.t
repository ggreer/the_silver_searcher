Setup:

  $ . "${TESTDIR}/setup.sh"

Should accept both --no-<option> and --no<option> forms.

(Here we're just parsing out all of the options listed in the `ag` usage help
that can be negated with 'no', and checking to make sure that each of them works
with either form. This is slightly convoluted, but it should ensure that any
options added in the future meet this requirement — assuming they're added to
the usage help, anyway.)

  $ test_negated_options() {
  >   ag --help 2>&1 |
  >   grep -oiE -- '--\[no-?\][a-z0-9_-]+' |
  >   cut -d ']' -f '2' |
  >   sort -u |
  >   while read option; do
  >     # The point here is that if the option we're testing is illegal, `ag`
  >     # will catch it on invocation and dump the usage help, causing the test
  >     # to produce output and thus fail
  >     printf 'foo\n' | ag "--no-${option}" -v '^foo$' 2>&1
  >     printf 'foo\n' | ag "--no${option}"  -v '^foo$' 2>&1
  >   done
  >   return 0
  > }
  $ test_negated_options

