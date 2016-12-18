#include <stdlib.h>
#include <string.h>

#include "lang.h"
#include "util.h"

lang_spec_t langs[] = {
    { "actionscript", { "as", "mxml" } },
    { "ada", { "ada", "adb", "ads" } },
    { "asm", { "asm", "s" } },
    { "batch", { "bat", "cmd" } },
    { "bitbake", { "bb", "bbappend", "bbclass", "inc" } },
    { "bro", { "bro", "bif" } },
    { "cc", { "c", "h", "xs" } },
    { "cfmx", { "cfc", "cfm", "cfml" } },
    { "chpl", { "chpl" } },
    { "clojure", { "clj", "cljs", "cljc", "cljx" } },
    { "coffee", { "coffee", "cjsx" } },
    { "cpp", { "cpp", "cc", "C", "cxx", "m", "hpp", "hh", "h", "H", "hxx", "tpp" } },
    { "crystal", { "cr", "ecr" } },
    { "csharp", { "cs" } },
    { "css", { "css" } },
    { "cython", { "pyx", "pxd", "pxi" } },
    { "delphi", { "pas", "int", "dfm", "nfm", "dof", "dpk", "dpr", "dproj", "groupproj", "bdsgroup", "bdsproj" } },
    { "ebuild", { "ebuild", "eclass" } },
    { "elisp", { "el" } },
    { "elixir", { "ex", "eex", "exs" } },
    { "erlang", { "erl", "hrl" } },
    { "factor", { "factor" } },
    { "fortran", { "f", "f77", "f90", "f95", "f03", "for", "ftn", "fpp" } },
    { "fsharp", { "fs", "fsi", "fsx" } },
    { "gettext", { "po", "pot", "mo" } },
    { "glsl", { "vert", "tesc", "tese", "geom", "frag", "comp" } },
    { "go", { "go" } },
    { "groovy", { "groovy", "gtmpl", "gpp", "grunit" } },
    { "haml", { "haml" } },
    { "haskell", { "hs", "lhs" } },
    { "hh", { "h" } },
    { "html", { "htm", "html", "shtml", "xhtml" } },
    { "ini", { "ini" } },
    { "jade", { "jade" } },
    { "java", { "java", "properties" } },
    { "js", { "js", "jsx", "vue" } },
    { "json", { "json" } },
    { "jsp", { "jsp", "jspx", "jhtm", "jhtml" } },
    { "julia", { "jl" } },
    { "kotlin", { "kt" } },
    { "less", { "less" } },
    { "liquid", { "liquid" } },
    { "lisp", { "lisp", "lsp" } },
    { "log", { "log" } },
    { "lua", { "lua" } },
    { "m4", { "m4" } },
    { "make", { "Makefiles", "mk", "mak" } },
    { "mako", { "mako" } },
    { "markdown", { "markdown", "mdown", "mdwn", "mkdn", "mkd", "md" } },
    { "mason", { "mas", "mhtml", "mpl", "mtxt" } },
    { "matlab", { "m" } },
    { "mathematica", { "m", "wl" } },
    { "mercury", { "m", "moo" } },
    { "nim", { "nim" } },
    { "objc", { "m", "h" } },
    { "objcpp", { "mm", "h" } },
    { "ocaml", { "ml", "mli", "mll", "mly" } },
    { "octave", { "m" } },
    { "parrot", { "pir", "pasm", "pmc", "ops", "pod", "pg", "tg" } },
    { "perl", { "pl", "pm", "pm6", "pod", "t" } },
    { "php", { "php", "phpt", "php3", "php4", "php5", "phtml" } },
    { "pike", { "pike", "pmod" } },
    { "plone", { "pt", "cpt", "metadata", "cpy", "py", "xml", "zcml" } },
    { "proto", { "proto" } },
    { "puppet", { "pp" } },
    { "python", { "py" } },
    { "qml", { "qml" } },
    { "racket", { "rkt", "ss", "scm" } },
    { "rake", { "Rakefile" } },
    { "restructuredtext", { "rst" } },
    { "rs", { "rs" } },
    { "r", { "R", "Rmd", "Rnw", "Rtex", "Rrst" } },
    { "rdoc", { "rdoc" } },
    { "ruby", { "rb", "rhtml", "rjs", "rxml", "erb", "rake", "spec" } },
    { "rust", { "rs" } },
    { "salt", { "sls" } },
    { "sass", { "sass", "scss" } },
    { "scala", { "scala" } },
    { "scheme", { "scm", "ss" } },
    { "shell", { "sh", "bash", "csh", "tcsh", "ksh", "zsh", "fish" } },
    { "smalltalk", { "st" } },
    { "sml", { "sml", "fun", "mlb", "sig" } },
    { "sql", { "sql", "ctl" } },
    { "stylus", { "styl" } },
    { "swift", { "swift" } },
    { "tcl", { "tcl", "itcl", "itk" } },
    { "tex", { "tex", "cls", "sty" } },
    { "tt", { "tt", "tt2", "ttml" } },
    { "toml", { "toml" } },
    { "ts", { "ts", "tsx" } },
    { "vala", { "vala", "vapi" } },
    { "vb", { "bas", "cls", "frm", "ctl", "vb", "resx" } },
    { "velocity", { "vm", "vtl", "vsl" } },
    { "verilog", { "v", "vh", "sv" } },
    { "vhdl", { "vhd", "vhdl" } },
    { "vim", { "vim" } },
    { "wix", { "wxi", "wxs" } },
    { "wsdl", { "wsdl" } },
    { "wadl", { "wadl" } },
    { "xml", { "xml", "dtd", "xsl", "xslt", "ent", "tld" } },
    { "yaml", { "yaml", "yml" } }
};

size_t get_lang_count() {
    return sizeof(langs) / sizeof(lang_spec_t);
}

char *make_lang_regex(char *ext_array, size_t num_exts) {
    int regex_capacity = 100;
    char *regex = ag_malloc(regex_capacity);
    int regex_length = 3;
    int subsequent = 0;
    char *extension;
    size_t i;

    strcpy(regex, "\\.(");

    for (i = 0; i < num_exts; ++i) {
        extension = ext_array + i * SINGLE_EXT_LEN;
        int extension_length = strlen(extension);
        while (regex_length + extension_length + 3 + subsequent > regex_capacity) {
            regex_capacity *= 2;
            regex = ag_realloc(regex, regex_capacity);
        }
        if (subsequent) {
            regex[regex_length++] = '|';
        } else {
            subsequent = 1;
        }
        strcpy(regex + regex_length, extension);
        regex_length += extension_length;
    }

    regex[regex_length++] = ')';
    regex[regex_length++] = '$';
    regex[regex_length++] = 0;
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
        do {
            if (num_of_extensions == ext_capacity) {
                break;
            }
            char *pos = (*exts) + num_of_extensions * SINGLE_EXT_LEN;
            strncpy(pos, ext, strlen(ext));
            ++num_of_extensions;
            ext = langs[extension_index[i]].extensions[++j];
        } while (ext);
    }

    return num_of_extensions;
}
