cmake_minimum_required(VERSION 2.8)

project(AG)

#get the version from configure.ac
set(EXECUTE_COMMAND grep -Po "(?<=\\[)([0-9.]+.[0-9]+.[0-9]+)(?=\\])" ${AG_SOURCE_DIR}/configure.ac)
execute_process(COMMAND ${EXECUTE_COMMAND} OUTPUT_VARIABLE AG_VERSION)
#trim whitespace off of version
string(STRIP ${AG_VERSION} AG_VERSION)

#set the output directory of ag.exe to be the main directory
#could also set it to be ${AG_SOURCE_DIR}/bin if so choose
#AG_SOURCE_DIR is automatically set up when you specify
#project(AG) on line 3
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${AG_SOURCE_DIR})

#the sources
list(APPEND AG_SOURCES
    src\decompress.c
    src\decompress.h
    src\ignore.c
    src\ignore.h
    src\lang.c
    src\lang.h
    src\log.c
    src\log.h
    src\main.c
    src\options.c
    src\options.h
    src\print.c
    src\print.h
    src\print_w32.c
    src\scandir.c
    src\scandir.h
    src\search.c
    src\search.h
    src\uthash.h
    src\util.c
    src\util.h
)

#create executable
add_executable(ag ${AG_SOURCES})

#add compile definitions
#unsure if HAVE_REALPATH is needed... was getting define errors on ubuntu
target_compile_definitions(ag
    PUBLIC -DPACKAGE_VERSION="${AG_VERSION}"
    PUBLIC -DHAVE_REALPATH
)

#set include directories
target_include_directories(ag
    PUBLIC src/win32
)

#link in z, lzma, pcre, and pthread
target_link_libraries(ag
    z
    pthread
    pcre
    lzma
)