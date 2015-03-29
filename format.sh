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

SOURCE_FILES=`git ls-files src/`

if [ "$1" == "reformat" ]
then
    echo "Reformatting source files"
    echo $CLANG_FORMAT -style=file -i $SOURCE_FILES
    $CLANG_FORMAT -style=file -i $SOURCE_FILES
    exit 0
elif  [ "$1" == "test" ]
then
    RESULT=`$CLANG_FORMAT -style=file -output-replacements-xml $SOURCE_FILES | grep -c '<replacement '`
    if [ $RESULT -eq 0 ]
    then
        echo "code is formatted correctly :)"
        exit 0
    else
        echo "code is not formatted correctly! :("
        echo "Run '$0 reformat' to fix formatting"
        exit 1
    fi
else
    echo "invalid command: $1"
    usage
    exit 1
fi
