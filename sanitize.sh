#!/bin/bash
# Copyright 2016 Allen Wild
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

AVAILABLE_SANITIZERS=(
    address
    thread
    undefined
    valgrind
)

DEFAULT_SANITIZERS=(
    address
    thread
    undefined
)

usage() {
    cat <<EOF
Usage: $0 [-h] [valgrind | [SANITIZERS ...]]

    This script recompiles ag using -fsanitize=<SANITIZER> and then runs the test suite.
    Memory leaks or other errors will be printed in ag's output, thus failing the test.

    Available LLVM sanitizers are: ${AVAILABLE_SANITIZERS[*]}

    The compile-time sanitizers are supported in clang/llvm >= 3.1 and gcc >= 4.8
    for x86_64 Linux only. clang is preferred and will be used, if available.

    For function names and line numbers in error output traces, llvm-symbolizer needs
    to be available in PATH or set through ASAN_SYMBOLIZER_PATH.

    If 'valgrind' is passed as the sanitizer, then ag will be run through valgrind
    without recompiling. If $(dirname $0)/ag doesn't exist, then it will be built.

    WARNING: This script will run "make distclean" and "./configure" to recompile ag
             once per sanitizer (except for valgrind). If you need to pass additional
             options to ./configure, put them in the CONFIGOPTS environment variable.
EOF
}

vrun() {
    echo "Running: $*"
    "$@"
}

die() {
    echo "Fatal: $*"
    exit 1
}

valid_sanitizer() {
    for san in "${AVAILABLE_SANITIZERS[@]}"; do
        if [[ "$1" == "$san" ]]; then
            return 0
        fi
    done
    return 1
}

run_sanitizer() {
    sanitizer=$1
    if [[ "$sanitizer" == "valgrind" ]]; then
        run_valgrind
        return $?
    fi

    echo -e "\nCompiling for sanitizer '$sanitizer'"
    [[ -f Makefile ]] && vrun make distclean
    vrun ./configure $CONFIGOPTS CC=$SANITIZE_CC \
                     CFLAGS="-g -O0 -fsanitize=$sanitizer $EXTRA_CFLAGS"
    if [[ $? != 0 ]]; then
        echo "ERROR: Failed to configure. Try setting CONFIGOPTS?"
        return 1
    fi

    vrun make
    if [[ $? != 0 ]]; then
        echo "ERROR: failed to build"
        return 1
    fi

    echo "Testing with sanitizer '$sanitizer'"
    vrun make test
    if [[ $? != 0 ]]; then
        echo "Tests for sanitizer '$sanitizer' FAIL!"
        echo "Check the above output for failure information"
        return 2
    else
        echo "Tests for sanitizer '$sanitizer' PASS!"
        return 0
    fi
}

run_valgrind() {
    echo "Compiling ag normally for use with valgrind"
    [[ -f Makefile ]] && vrun make distclean
    vrun ./configure $CONFIGOPTS
    if [[ $? != 0 ]]; then
        echo "ERROR: Failed to configure. Try setting CONFIGOPTS?"
        return 1
    fi

    vrun make
    if [[ $? != 0 ]]; then
        echo "ERROR: failed to build"
        return 1
    fi

    echo "Running: AGPROG=\"valgrind -q $PWD/ag\" make test"
    AGPROG="valgrind -q $PWD/ag" make test
    if [[ $? != 0 ]]; then
        echo "Valgrind tests FAIL!"
        return 1
    else
        echo "Valgrind tests PASS!"
        return 0
    fi
}

#### MAIN ####
run_sanitizers=()
for opt in "$@"; do
    if [[ "$opt" == -* ]]; then
        case opt in
            -h|--help)
                usage
                exit 0
                ;;
            *)
                echo "Unknown option: '$opt'"
                usage
                exit 1
                ;;
        esac
    else
        if valid_sanitizer "$opt"; then
            run_sanitizers+=("$opt")
        else
            echo "Invalid Sanitizer: '$opt'"
            usage
            exit 1
        fi
    fi
done

if [[ ${#run_sanitizers[@]} == 0 ]]; then
    run_sanitizers=(${DEFAULT_SANITIZERS[@]})
fi

if [[ -n $CC ]]; then
    echo "Using CC=$CC"
    SANITIZE_CC="$CC"
elif which clang &>/dev/null; then
    SANITIZE_CC="clang"
else
    echo "Warning: CC unset and clang not found"
fi

if [[ -n $CFLAGS ]]; then
    EXTRA_CFLAGS="$CFLAGS"
    unset CFLAGS
fi

if [[ ! -e ./configure ]]; then
    echo "Warning: ./configure not found. Running autogen"
    vrun ./autogen.sh || die "autogen.sh failed"
fi

echo "Running sanitizers: ${run_sanitizers[*]}"
failedsan=()
for san in "${run_sanitizers[@]}"; do
    run_sanitizer $san
    if [[ $? != 0 ]]; then
        failedsan+=($san)
    fi
done

if [[ ${#failedsan[@]} == 0 ]]; then
    echo "All sanitizers PASSED"
    exit 0
else
    echo "The following sanitizers FAILED: ${failedsan[*]}"
    exit ${#failedsan[@]}
fi
