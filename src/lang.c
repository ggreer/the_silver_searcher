#include <stdlib.h>
#include <string.h>

#include "lang.h"
#include "util.h"

lang_spec_t langs[] = {
    { "actionscript", { "as", "mxml" }, {} },
    { "ada", { "ada", "adb", "ads" }, {} },
    { "asm", { "asm", "s" }, {} },
    { "batch", { "bat", "cmd" }, {} },
    { "bitbake", { "bb", "bbappend", "bbclass", "inc" }, {} },
    { "bro", { "bro", "bif" }, {} },
    { "cc", { "c", "h", "xs" }, {} },
    { "cfmx", { "cfc", "cfm", "cfml" }, {} },
    { "clojure", { "clj", "cljs", "cljc", "cljx" }, {} },
    { "cmake", { "cmake" }, { "CMakeLists.txt" } },
    { "coffee", { "coffee", "cjsx" }, {} },
    { "cpp", { "cpp", "cc", "C", "cxx", "m", "hpp", "hh", "h", "H", "hxx" }, {} },
    { "crystal", { "cr", "ecr" }, {} },
    { "csharp", { "cs" }, {} },
    { "css", { "css" }, {} },
    { "delphi", { "pas", "int", "dfm", "nfm", "dof", "dpk", "dproj", "groupproj", "bdsgroup", "bdsproj" }, {} },
    { "ebuild", { "ebuild", "eclass" }, {} },
    { "elisp", { "el" }, {} },
    { "elixir", { "ex", "exs" }, {} },
    { "erlang", { "erl", "hrl" }, {} },
    { "fortran", { "f", "f77", "f90", "f95", "f03", "for", "ftn", "fpp" }, {} },
    { "fsharp", { "fs", "fsi", "fsx" }, {} },
    { "gettext", { "po", "pot", "mo" }, {} },
    { "go", { "go" }, {} },
    { "groovy", { "groovy", "gtmpl", "gpp", "grunit" }, {} },
    { "haml", { "haml" }, {} },
    { "haskell", { "hs", "lhs" }, {} },
    { "hh", { "h" }, {} },
    { "html", { "htm", "html", "shtml", "xhtml" }, {} },
    { "ini", { "ini" }, {} },
    { "jade", { "jade" }, {} },
    { "java", { "java", "properties" }, {} },
    { "js", { "js", "jsx" }, {} },
    { "json", { "json" }, {} },
    { "jsp", { "jsp", "jspx", "jhtm", "jhtml" }, {} },
    { "less", { "less" }, {} },
    { "liquid", { "liquid" }, {} },
    { "lisp", { "lisp", "lsp" }, {} },
    { "lua", { "lua" }, {} },
    { "m4", { "m4" }, {} },
    { "make", { "Makefiles", "mk", "mak" }, { "Makefile", "GNUmakefile" }  },
    { "mako", { "mako" }, {} },
    { "markdown", { "markdown", "mdown", "mdwn", "mkdn", "mkd", "md" }, {} },
    { "mason", { "mas", "mhtml", "mpl", "mtxt" }, {} },
    { "matlab", { "m" }, {} },
    { "mathematica", { "m", "wl" }, {} },
    { "mercury", { "m", "moo" }, {} },
    { "nim", { "nim" }, {} },
    { "objc", { "m", "h" }, {} },
    { "objcpp", { "mm", "h" }, {} },
    { "ocaml", { "ml", "mli", "mll", "mly" }, {} },
    { "octave", { "m" }, {} },
    { "parrot", { "pir", "pasm", "pmc", "ops", "pod", "pg", "tg" }, {} },
    { "perl", { "pl", "pm", "pm6", "pod", "t" }, {} },
    { "php", { "php", "phpt", "php3", "php4", "php5", "phtml" }, {} },
    { "pike", { "pike", "pmod" }, {} },
    { "plone", { "pt", "cpt", "metadata", "cpy", "py" }, {} },
    { "puppet", { "pp" }, {} },
    { "python", { "py" }, {} },
    { "racket", { "rkt", "ss", "scm" }, {} },
    { "rake", { "Rakefiles" }, {} },
    { "restructuredtext", { "rst" }, {} },
    { "rs", { "rs" }, {} },
    { "r", { "R", "Rmd", "Rnw", "Rtex", "Rrst" }, {} },
    { "rdoc", { "rdoc" }, {} },
    { "ruby", { "rb", "rhtml", "rjs", "rxml", "erb", "rake", "spec" }, {} },
    { "rust", { "rs" }, {} },
    { "salt", { "sls" }, {} },
    { "sass", { "sass", "scss" }, {} },
    { "scons", { "py" }, {"SConstruct"} },
    { "scala", { "scala" }, {} },
    { "scheme", { "scm", "ss" }, {} },
    { "shell", { "sh", "bash", "csh", "tcsh", "ksh", "zsh" }, {} },
    { "smalltalk", { "st" }, {} },
    { "sml", { "sml", "fun", "mlb", "sig" }, {} },
    { "sql", { "sql", "ctl" }, {} },
    { "stylus", { "styl" }, {} },
    { "swift", { "swift" }, {} },
    { "tcl", { "tcl", "itcl", "itk" }, {} },
    { "tex", { "tex", "cls", "sty" }, {} },
    { "tt", { "tt", "tt2", "ttml" }, {} },
    { "toml", { "toml" }, {} },
    { "ts", { "ts", "tsx" }, {} },
    { "vala", { "vala", "vapi" }, {} },
    { "vb", { "bas", "cls", "frm", "ctl", "vb", "resx" }, {} },
    { "velocity", { "vm", "vtl", "vsl" }, {} },
    { "verilog", { "v", "vh", "sv" }, {} },
    { "vhdl", { "vhd", "vhdl" }, {} },
    { "vim", { "vim" }, {} },
    { "wsdl", { "wsdl" }, {} },
    { "wadl", { "wadl" }, {} },
    { "xml", { "xml", "dtd", "xsl", "xslt", "ent" }, {} },
    { "yaml", { "yaml", "yml" }, {} }
};

size_t get_lang_count() {
    return sizeof(langs) / sizeof(lang_spec_t);
}

char *make_lang_regex(char *ext_array, size_t num_exts,
                      char *full_filenames, size_t num_filenames) {
    // characters in the regex used for "management" not belonging to the names itself
    const size_t mgmt_overhead = 20;
    char *regex = ag_malloc(mgmt_overhead + 
            num_exts * (SINGLE_EXT_LEN+1) +
            num_filenames * (SINGLE_NAME_LEN+1) );
    int subsequent = 0;
    size_t i;

    // remember, the RE has the form
    // '((^|[/\\])(full_name1|full_name2)|\.(extension1|extension2...))$'

    regex[0] = '\0';

    strcat(regex, "((^|[/\\\\])(");

    for (i = 0; i < num_filenames; ++i) {
        char *name = full_filenames + i * SINGLE_NAME_LEN;
        if (subsequent) {
            strcat(regex, "|");
        } else {
            subsequent = 1;
        }
        strcat(regex, name);
    }

    strcat(regex, ")|\\.(");

    subsequent = 0;
    for (i = 0; i < num_exts; ++i) {
        char *extension = ext_array + i * SINGLE_EXT_LEN;
        if (subsequent) {
            strcat(regex, "|");
        } else {
            subsequent = 1;
        }
        strcat(regex, extension);
    }

    strcat(regex, "))$");

    return regex;
}

size_t combine_file_extensions(size_t *extension_index, size_t len, char **exts) {
    /* Keep it fixed as 100 for the reason that if you have more than 100
     * file types to search, you'd better search all the files.
     * */
    size_t ext_capacity = 100;
    (*exts) = (char *)ag_malloc(ext_capacity * SINGLE_EXT_LEN);
    memset((*exts), 0, ext_capacity * SINGLE_EXT_LEN);
    size_t num_of_extensions = 0;

    size_t i;
    for (i = 0; i < len; ++i) {
        size_t j = 0;
        const char *ext = langs[extension_index[i]].extensions[j];
        while (ext) {
            if (num_of_extensions == ext_capacity) {
                break;
            }
            char *pos = (*exts) + num_of_extensions * SINGLE_EXT_LEN;
            strncpy(pos, ext, strlen(ext));
            ++num_of_extensions;
            ext = langs[extension_index[i]].extensions[++j];
        }
    }

    return num_of_extensions;
}

size_t combine_full_filenames(size_t *extension_index, size_t len, char **names) {
    /* Keep it fixed as 100 for the reason that if you have more than 100
     * file types to search, you'd better search all the files.
     * */
    size_t names_capacity = 100;
    (*names) = (char *)ag_malloc(names_capacity * SINGLE_NAME_LEN);
    memset((*names), 0, names_capacity * SINGLE_NAME_LEN);
    size_t num_of_names = 0;

    size_t i;
    for (i = 0; i < len; ++i) {
        size_t j = 0;
        const char *name = langs[extension_index[i]].full_filenames[j];
        while (name) {
            if (num_of_names == names_capacity) {
                break;
            }
            char *pos = (*names) + num_of_names * SINGLE_NAME_LEN;
            strncpy(pos, name, strlen(name));
            ++num_of_names;
            name = langs[extension_index[i]].full_filenames[++j];
        }
    }

    return num_of_names;
}
