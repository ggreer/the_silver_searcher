#include <stdlib.h>
#include <string.h>

#include "lang.h"
#include "util.h"

lang_spec_t langs[] = {
    { "actionscript", { "as", "mxml" }, {} },
    { "ada", { "ada", "adb", "ads" }, {} },
    { "asciidoc", { "adoc", "ad", "asc", "asciidoc" }, {} },
    { "asm", { "asm", "s" }, {} },
    { "batch", { "bat", "cmd" }, {} },
    { "bitbake", { "bb", "bbappend", "bbclass", "inc" }, {} },
    { "bro", { "bro", "bif" }, {} },
    { "cc", { "c", "h", "xs" }, {} },
    { "cfmx", { "cfc", "cfm", "cfml" }, {} },
    { "chpl", { "chpl" }, {} },
    { "clojure", { "clj", "cljs", "cljc", "cljx" }, {} },
    { "cmake", { "cmake" }, { "CMakeLists.txt" } },
    { "coffee", { "coffee", "cjsx" }, {} },
    { "cpp", { "cpp", "cc", "C", "cxx", "m", "hpp", "hh", "h", "H", "hxx", "tpp" }, {} },
    { "crystal", { "cr", "ecr" }, {} },
    { "csharp", { "cs" }, {} },
    { "css", { "css" }, {} },
    { "cython", { "pyx", "pxd", "pxi" }, {} },
    { "delphi", { "pas", "int", "dfm", "nfm", "dof", "dpk", "dpr", "dproj", "groupproj", "bdsgroup", "bdsproj" }, {} },
    { "dot", { "dot", "gv" }, {} },
    { "ebuild", { "ebuild", "eclass" }, {} },
    { "elisp", { "el" }, {} },
    { "elixir", { "ex", "eex", "exs" }, {} },
    { "elm", { "elm" }, {} },
    { "erlang", { "erl", "hrl" }, {} },
    { "factor", { "factor" }, {} },
    { "fortran", { "f", "f77", "f90", "f95", "f03", "for", "ftn", "fpp" }, {} },
    { "fsharp", { "fs", "fsi", "fsx" }, {} },
    { "gettext", { "po", "pot", "mo" }, {} },
    { "glsl", { "vert", "tesc", "tese", "geom", "frag", "comp" }, {} },
    { "go", { "go" }, {} },
    { "groovy", { "groovy", "gtmpl", "gpp", "grunit", "gradle" }, {} },
    { "haml", { "haml" }, {} },
    { "handlebars", { "hbs" }, {} },
    { "haskell", { "hs", "lhs" }, {} },
    { "haxe", { "hx" }, {} },
    { "hh", { "h" }, {} },
    { "html", { "htm", "html", "shtml", "xhtml" }, {} },
    { "ini", { "ini" }, {} },
    { "ipython", { "ipynb" }, {} },
    { "jade", { "jade" }, {} },
    { "java", { "java", "properties" }, {} },
    { "js", { "es6", "js", "jsx", "vue" }, {} },
    { "json", { "json" }, {} },
    { "jsp", { "jsp", "jspx", "jhtm", "jhtml", "jspf", "tag", "tagf" }, {} },
    { "julia", { "jl" }, {} },
    { "kotlin", { "kt" }, {} },
    { "less", { "less" }, {} },
    { "liquid", { "liquid" }, {} },
    { "lisp", { "lisp", "lsp" }, {} },
    { "log", { "log" }, {} },
    { "lua", { "lua" }, {} },
    { "m4", { "m4" }, {} },
    { "make", { "Makefiles", "mk", "mak" }, { "Makefile" } },
    { "mako", { "mako" }, {} },
    { "markdown", { "markdown", "mdown", "mdwn", "mkdn", "mkd", "md" }, {} },
    { "mason", { "mas", "mhtml", "mpl", "mtxt" }, {} },
    { "matlab", { "m" }, {} },
    { "mathematica", { "m", "wl" }, {} },
    { "md", { "markdown", "mdown", "mdwn", "mkdn", "mkd", "md" }, {} },
    { "mercury", { "m", "moo" }, {} },
    { "nim", { "nim" }, {} },
    { "nix", { "nix" }, {} },
    { "objc", { "m", "h" }, {} },
    { "objcpp", { "mm", "h" }, {} },
    { "ocaml", { "ml", "mli", "mll", "mly" }, {} },
    { "octave", { "m" }, {} },
    { "org", { "org" }, {} },
    { "parrot", { "pir", "pasm", "pmc", "ops", "pod", "pg", "tg" }, {} },
    { "perl", { "pl", "pm", "pm6", "pod", "t" }, {} },
    { "php", { "php", "phpt", "php3", "php4", "php5", "phtml" }, {} },
    { "pike", { "pike", "pmod" }, {} },
    { "plist", { "plist" }, {} },
    { "plone", { "pt", "cpt", "metadata", "cpy", "py", "xml", "zcml" }, {} },
    { "proto", { "proto" }, {} },
    { "puppet", { "pp" }, {} },
    { "python", { "py" }, {} },
    { "qml", { "qml" }, {} },
    { "racket", { "rkt", "ss", "scm" }, {} },
    { "rake", {}, { "Rakefile" } },
    { "restructuredtext", { "rst" }, {} },
    { "rs", { "rs" }, {} },
    { "r", { "R", "Rmd", "Rnw", "Rtex", "Rrst" }, {} },
    { "rdoc", { "rdoc" }, {} },
    { "ruby", { "rb", "rhtml", "rjs", "rxml", "erb", "rake", "spec" }, {} },
    { "rust", { "rs" }, {} },
    { "salt", { "sls" }, {} },
    { "sass", { "sass", "scss" }, {} },
    { "scala", { "scala" }, {} },
    { "scheme", { "scm", "ss" }, {} },
    { "shell", { "sh", "bash", "csh", "tcsh", "ksh", "zsh", "fish" }, {} },
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
    { "twig", { "twig" }, {} },
    { "vala", { "vala", "vapi" }, {} },
    { "vb", { "bas", "cls", "frm", "ctl", "vb", "resx" }, {} },
    { "velocity", { "vm", "vtl", "vsl" }, {} },
    { "verilog", { "v", "vh", "sv" }, {} },
    { "vhdl", { "vhd", "vhdl" }, {} },
    { "vim", { "vim" }, {} },
    { "wix", { "wxi", "wxs" }, {} },
    { "wsdl", { "wsdl" }, {} },
    { "wadl", { "wadl" }, {} },
    { "xml", { "xml", "dtd", "xsl", "xslt", "ent", "tld", "plist" }, {} },
    { "yaml", { "yaml", "yml" }, {} }
};

size_t get_lang_count() {
    return sizeof(langs) / sizeof(lang_spec_t);
}

char *make_lang_regex(const char **ext_array, size_t num_exts, const char **name_array, size_t num_names) {
    int regex_capacity = 100;
    char *regex = ag_malloc(regex_capacity);
    int regex_length = 0;
    int subsequent = 0;
    size_t i;

    if (num_exts > 0) {
        regex_length = 4;
        strcpy(regex, "(\\.(");

        for (i = 0; i < num_exts; ++i) {
            const char *extension = ext_array[i];
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
    }

    for (i = 0; i < num_names; ++i) {
        const char *name = name_array[i];
        int name_length = strlen(name);
        while (regex_length + name_length + 3 + subsequent > regex_capacity) {
            regex_capacity *= 2;
            regex = ag_realloc(regex, regex_capacity);
        }
        if (subsequent) {
            regex[regex_length++] = '|';
        } else {
            subsequent = 1;
        }
        regex[regex_length++] = '/';
        strcpy(regex + regex_length, name);
        regex_length += name_length;
    }

    if (num_exts > 0)
        regex[regex_length++] = ')';
    regex[regex_length++] = '$';
    regex[regex_length++] = 0;

    return regex;
}

void combine_file_extensions(size_t *extension_index, size_t len,
                             size_t *num_extensions, const char ***exts,
                             size_t *num_names, const char ***names) {
    /* Keep it fixed as 100 for the reason that if you have more than 100
     * file types to search, you'd better search all the files.
     * */
    static const size_t ext_capacity = 100;
    static const size_t name_capacity = 100;
    (*exts) = (const char **)ag_malloc(ext_capacity * sizeof(const char **));
    memset((*exts), 0, ext_capacity * sizeof(const char **));
    (*names) = (const char **)ag_malloc(ext_capacity * sizeof(const char **));
    memset((*names), 0, name_capacity * sizeof(const char **));
    size_t num_of_extensions = 0;
    size_t num_of_names = 0;

    size_t i;
    for (i = 0; i < len; ++i) {
        size_t j = 0;
        const char *ext = langs[extension_index[i]].extensions[j];
        while (ext && j < MAX_EXTENSIONS) {
            if (num_of_extensions == ext_capacity) {
                break;
            }
            (*exts)[num_of_extensions++] = ext;
            ext = langs[extension_index[i]].extensions[++j];
        }

        j = 0;
        const char *name = langs[extension_index[i]].names[j];
        while (name && j < MAX_NAMES) {
            if (num_of_names == name_capacity) {
                break;
            }
            (*names)[num_of_names++] = name;
            name = langs[extension_index[i]].names[++j];
        }
    }

    *num_extensions = num_of_extensions;
    *num_names = num_of_names;
}
