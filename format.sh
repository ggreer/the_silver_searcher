#!/bin/bash

function usage() {
    echo "Usage: $0 test|reformat"
}

if [ $# -eq 0 ]
then
    usage
    exit 0
fi

if [ -z "$CLANG_FORMAT" ]
then
    CLANG_FORMAT=clang-format
    echo "No CLANG_FORMAT set. Using $CLANG_FORMAT"
fi

if ! type "$CLANG_FORMAT" &> /dev/null
then
    echo "The command \"$CLANG_FORMAT\" was not found"
    exit 1
fi

# Do not attempt reformating src/*.bat nor src/*.mak, as these are not C source files
SOURCE_FILES=$(git ls-files src/*.c src/*.h)

if [ "$1" == "reformat" ]
then
    echo "Reformatting source files"
    # shellcheck disable=2086
    echo $CLANG_FORMAT -style=file -i $SOURCE_FILES
    # shellcheck disable=2086
    $CLANG_FORMAT -style=file -i $SOURCE_FILES
    exit 0
elif  [ "$1" == "test" ]
then
    # shellcheck disable=2086
    RESULT=$($CLANG_FORMAT -style=file -output-replacements-xml $SOURCE_FILES | grep -c '<replacement ')
    if [ "$RESULT" -eq 0 ]
    then
        echo "code is formatted correctly :)"
        exit 0
    else
        echo "code is not formatted correctly! :("
        echo "Suggested change:"
        cp -r src clang_format_src
        $CLANG_FORMAT -style=file -i clang_format_src/*.c clang_format_src/*.h
        diff -ur src clang_format_src
        rm -r clang_format_src
        echo "Run '$0 reformat' to fix formatting"
        exit 1
    fi
else
    echo "invalid command: $1"
    usage
    exit 1
fi
