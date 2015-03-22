# All cram tests should use this. Make sure that "ag" runs the version
# of ag we just built, and make the output really simple.

alias ag="$TESTDIR/../ag --nocolor --workers=1 --parallel"

# Ignore the user's environment, which might change default behaviors
# export AG_IGNORE_ENVIRONMENT=true

# Instead, remove the variables from the environment
eval `env | egrep '^AG_' | sed -e 's/^\([^=]*\)=.*/unset \1/'`
