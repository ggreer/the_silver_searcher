#include <stdlib.h>
#include <string.h>

#include "lang.h"
#include "util.h"

lang_spec_t langs[] = {
    { "actionscript", { "as", "mxml" } },
    { "ada", { "ada", "adb", "ads" } },
    { "asm", { "asm", "s" } },
    { "batch", { "bat", "cmd" } },
    { "cc", { "c", "h", "xs" } },
    { "cfmx", { "cfc", "cfm", "cfml" } },
    { "clojure", { "clj", "cljs", "cljx" } },
    { "coffee", { "coffee" } },
    { "cpp", { "cpp", "cc", "C", "cxx", "m", "hpp", "hh", "h", "H", "hxx" } },
    { "csharp", { "cs" } },
    { "css", { "css" } },
    { "delphi", { "pas", "int", "dfm", "nfm", "dof", "dpk", "dproj", "groupproj", "bdsgroup", "bdsproj" } },
    { "elisp", { "el" } },
    { "erlang", { "erl", "hrl" } },
    { "fortran", { "f", "f77", "f90", "f95", "f03", "for", "ftn", "fpp" } },
    { "gettext", { "po", "pot", "mo" } },
    { "go", { "go" } },
    { "groovy", { "groovy", "gtmpl", "gpp", "grunit" } },
    { "haml", { "haml" } },
    { "haskell", { "hs", "lhs" } },
    { "hh", { "h" } },
    { "html", { "htm", "html", "shtml", "xhtml" } },
    { "ini", { "ini" } },
    { "jade", { "jade" } },
    { "java", { "java", "properties" } },
    { "js", { "js" } },
    { "json", { "json" } },
    { "jsp", { "jsp", "jspx", "jhtm", "jhtml" } },
    { "less", { "less" } },
    { "lisp", { "lisp", "lsp" } },
    { "lua", { "lua" } },
    { "m4", { "m4" } },
    { "make", { "Makefiles", "mk", "mak" } },
    { "mason", { "mas", "mhtml", "mpl", "mtxt" } },
    { "matlab", { "m" } },
    { "objc", { "m", "h" } },
    { "objcpp", { "mm", "h" } },
    { "ocaml", { "ml", "mli" } },
    { "octave", { "m" } },
    { "parrot", { "pir", "pasm", "pmc", "ops", "pod", "pg", "tg" } },
    { "perl", { "pl", "pm", "pm6", "pod", "t" } },
    { "php", { "php", "phpt", "php3", "php4", "php5", "phtml" } },
    { "plone", { "pt", "cpt", "metadata", "cpy", "py" } },
    { "python", { "py" } },
    { "rake", { "Rakefiles" } },
    { "rs", { "rs" } },
    { "ruby", { "rb", "rhtml", "rjs", "rxml", "erb", "rake", "spec" } },
    { "rust", { "rs" } },
    { "salt", { "sls" } },
    { "sass", { "sass", "scss" } },
    { "scala", { "scala" } },
    { "scheme", { "scm", "ss" } },
    { "shell", { "sh", "bash", "csh", "tcsh", "ksh", "zsh" } },
    { "smalltalk", { "st" } },
    { "sql", { "sql", "ctl" } },
    { "stylus", { "styl" } },
    { "swift", { "swift" } },
    { "tcl", { "tcl", "itcl", "itk" } },
    { "tex", { "tex", "cls", "sty" } },
    { "tt", { "tt", "tt2", "ttml" } },
    { "vb", { "bas", "cls", "frm", "ctl", "vb", "resx" } },
    { "verilog", { "v", "vh", "sv" } },
    { "vhdl", { "vhd", "vhdl" } },
    { "vim", { "vim" } },
    { "xml", { "xml", "dtd", "xsl", "xslt", "ent" } },
    { "yaml", { "yaml", "yml" } },
    { NULL, { NULL } }
};

char *make_lang_regex(const char **extensions) {
    int regex_capacity = 100;
    char *regex = ag_malloc(regex_capacity);
    int regex_length = 3;
    int subsequent = 0;
    const char **extension;

    strcpy(regex, "\\.(");

    for (extension = extensions; *extension; ++extension) {
        int extension_length = strlen(*extension);
        while (regex_length + extension_length + 3 + subsequent > regex_capacity) {
            regex_capacity *= 2;
            regex = ag_realloc(regex, regex_capacity);
        }
        if (subsequent) {
            regex[regex_length++] = '|';
        } else {
            subsequent = 1;
        }
        strcpy(regex + regex_length, *extension);
        regex_length += extension_length;
    }

    regex[regex_length++] = ')';
    regex[regex_length++] = '$';
    regex[regex_length++] = 0;
    return regex;
}
