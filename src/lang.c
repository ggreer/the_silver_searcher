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
    { "clojure", { "clj" } },
    { "coffee", { "coffee" } },
    { "cpp", { "cpp", "cc", "cxx", "m", "hpp", "hh", "h", "hxx" } },
    { "csharp", { "cs" } },
    { "css", { "css" } },
    { "delphi", { "pas", "int", "dfm", "nfm", "dof", "dpk", "dproj", "groupproj", "bdsgroup", "bdsproj" } },
    { "elisp", { "el" } },
    { "erlang", { "erl", "hrl" } },
    { "fortran", { "f", "f77", "f90", "f95", "f03", "for", "ftn", "fpp" } },
    { "gettext", { "po", "pot", "mo" } },
    { "go", { "go" } },
    { "groovy", { "groovy", "gtmpl", "gpp", "grunit" } },
    { "haskell", { "hs", "lhs" } },
    { "hh", { "h" } },
    { "haml", { "haml" } },
    { "html", { "htm", "html", "shtml", "xhtml" } },
    { "java", { "java", "properties" } },
    { "js", { "js" } },
    { "jsp", { "jsp", "jspx", "jhtm", "jhtml" } },
    { "json", { "json" } },
    { "lisp", { "lisp", "lsp" } },
    { "lua", { "lua" } },
    { "m4", { "m4" } },
    { "matlab", { "m" } },
    { "make", { "Makefiles", "mk", "mak" } },
    { "mason", { "mas", "mhtml", "mpl", "mtxt" } },
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
    { "sass", { "sass", "scss" } },
    { "salt", { "sls" } },
    { "scala", { "scala" } },
    { "scheme", { "scm", "ss" } },
    { "shell", { "sh", "bash", "csh", "tcsh", "ksh", "zsh" } },
    { "smalltalk", { "st" } },
    { "sql", { "sql", "ctl" } },
    { "tcl", { "tcl", "itcl", "itk" } },
    { "tex", { "tex", "cls", "sty" } },
    { "tt", { "tt", "tt2", "ttml" } },
    { "vb", { "bas", "cls", "frm", "ctl", "vb", "resx" } },
    { "verilog", { "v", "vh", "sv" } },
    { "vhdl", { "vhd", "vhdl" } },
    { "vim", { "vim" } },
    { "yaml", { "yaml", "yml" } },
    { "xml", { "xml", "dtd", "xsl", "xslt", "ent" } },
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
