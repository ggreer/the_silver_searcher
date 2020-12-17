#include <stdlib.h>
#include <string.h>

#include "lang.h"
#include "util.h"

lang_spec_t langs[] = {
    { "actionscript", { "as", "mxml" } },
    { "ada", { "ada", "adb", "ads" } },
    { "asciidoc", { "adoc", "ad", "asc", "asciidoc" } },
    { "apl", { "apl" } },
    { "asm", { "asm", "s" } },
    { "asp", { "asp", "asa", "aspx", "asax", "ashx", "ascx", "asmx" } },
    { "aspx", { "asp", "asa", "aspx", "asax", "ashx", "ascx", "asmx" } },
    { "batch", { "bat", "cmd" } },
    { "bazel", { "bazel" } },
    { "bitbake", { "bb", "bbappend", "bbclass", "inc" } },
    { "cc", { "c", "h", "xs" } },
    { "cfmx", { "cfc", "cfm", "cfml" } },
    { "chpl", { "chpl" } },
    { "clojure", { "clj", "cljs", "cljc", "cljx", "edn" } },
    { "coffee", { "coffee", "cjsx" } },
    { "config", { "config" } },
    { "coq", { "coq", "g", "v" } },
    { "cpp", { "cpp", "cc", "C", "cxx", "m", "hpp", "hh", "h", "H", "hxx", "tpp" } },
    { "crystal", { "cr", "ecr" } },
    { "csharp", { "cs" } },
    { "cshtml", { "cshtml" } },
    { "css", { "css" } },
    { "cython", { "pyx", "pxd", "pxi" } },
    { "delphi", { "pas", "int", "dfm", "nfm", "dof", "dpk", "dpr", "dproj", "groupproj", "bdsgroup", "bdsproj" } },
    { "dlang", { "d", "di" } },
    { "dot", { "dot", "gv" } },
    { "dts", { "dts", "dtsi" } },
    { "ebuild", { "ebuild", "eclass" } },
    { "elisp", { "el" } },
    { "elixir", { "ex", "eex", "exs" } },
    { "elm", { "elm" } },
    { "erlang", { "erl", "hrl" } },
    { "factor", { "factor" } },
    { "fortran", { "f", "F", "f77", "f90", "F90", "f95", "f03", "for", "ftn", "fpp", "FPP" } },
    { "fsharp", { "fs", "fsi", "fsx" } },
    { "gettext", { "po", "pot", "mo" } },
    { "glsl", { "vert", "tesc", "tese", "geom", "frag", "comp" } },
    { "go", { "go" } },
    { "gradle", { "gradle" } },
    { "groovy", { "groovy", "gtmpl", "gpp", "grunit", "gradle" } },
    { "haml", { "haml" } },
    { "handlebars", { "hbs" } },
    { "haskell", { "hs", "hsig", "lhs" } },
    { "haxe", { "hx" } },
    { "hh", { "h" } },
    { "html", { "htm", "html", "shtml", "xhtml" } },
    { "idris", { "idr", "ipkg", "lidr" } },
    { "ini", { "ini" } },
    { "ipython", { "ipynb" } },
    { "isabelle", { "thy" } },
    { "j", { "ijs" } },
    { "jade", { "jade" } },
    { "java", { "java", "properties" } },
    { "jinja2", { "j2" } },
    { "js", { "es6", "js", "jsx", "vue" } },
    { "json", { "json" } },
    { "jsp", { "jsp", "jspx", "jhtm", "jhtml", "jspf", "tag", "tagf" } },
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
    { "md", { "markdown", "mdown", "mdwn", "mkdn", "mkd", "md" } },
    { "mercury", { "m", "moo" } },
    { "naccess", { "asa", "rsa" } },
    { "nim", { "nim" } },
    { "nix", { "nix" } },
    { "objc", { "m", "h" } },
    { "objcpp", { "mm", "h" } },
    { "ocaml", { "ml", "mli", "mll", "mly" } },
    { "octave", { "m" } },
    { "org", { "org" } },
    { "parrot", { "pir", "pasm", "pmc", "ops", "pod", "pg", "tg" } },
    { "pdb", { "pdb" } },
    { "perl", { "pl", "pm", "pm6", "pod", "t" } },
    { "php", { "php", "phpt", "php3", "php4", "php5", "phtml" } },
    { "pike", { "pike", "pmod" } },
    { "plist", { "plist" } },
    { "plone", { "pt", "cpt", "metadata", "cpy", "py", "xml", "zcml" } },
    { "powershell", { "ps1" } },
    { "proto", { "proto" } },
    { "ps1", { "ps1" } },
    { "pug", { "pug" } },
    { "puppet", { "pp" } },
    { "python", { "py" } },
    { "qml", { "qml" } },
    { "racket", { "rkt", "ss", "scm" } },
    { "rake", { "Rakefile" } },
    { "razor", { "cshtml" } },
    { "restructuredtext", { "rst" } },
    { "rs", { "rs" } },
    { "r", { "r", "R", "Rmd", "Rnw", "Rtex", "Rrst" } },
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
    { "stata", { "do", "ado" } },
    { "stylus", { "styl" } },
    { "swift", { "swift" } },
    { "tcl", { "tcl", "itcl", "itk" } },
    { "terraform", { "tf", "tfvars" } },
    { "tex", { "tex", "cls", "sty" } },
    { "thrift", { "thrift" } },
    { "tla", { "tla" } },
    { "tt", { "tt", "tt2", "ttml" } },
    { "toml", { "toml" } },
    { "ts", { "ts", "tsx" } },
    { "twig", { "twig" } },
    { "vala", { "vala", "vapi" } },
    { "vb", { "bas", "cls", "frm", "ctl", "vb", "resx" } },
    { "velocity", { "vm", "vtl", "vsl" } },
    { "verilog", { "v", "vh", "sv", "svh" } },
    { "vhdl", { "vhd", "vhdl" } },
    { "vim", { "vim" } },
    { "vue", { "vue" } },
    { "wix", { "wxi", "wxs" } },
    { "wsdl", { "wsdl" } },
    { "wadl", { "wadl" } },
    { "xml", { "xml", "dtd", "xsl", "xslt", "xsd", "ent", "tld", "plist", "wsdl" } },
    { "yaml", { "yaml", "yml" } },
    { "zeek", { "zeek", "bro", "bif" } },
    { "zephir", { "zep" } }
};

size_t get_lang_count() {
    return sizeof(langs) / sizeof(lang_spec_t);
}

char *make_lang_regex(size_t *selected_langs, size_t count) {
    size_t regex_capacity = 128;
    size_t regex_length = 3;
    char *regex = ag_malloc(regex_capacity);
    int first = 1;
    size_t i, j;

    strncpy(regex, "\\.(", regex_capacity);

    // for each requested language
    for (i = 0; i < count; ++i) {
        const char **current_lang_exts = langs[selected_langs[i]].extensions;

        // for each ext in the language
        for (j = 0; current_lang_exts[j] != NULL; ++j) {
            const char *ext = current_lang_exts[j];
            const size_t ext_len = strlen(ext);

            // resize regex buffer as needed. +4 is room for "|)$\0"
            while (regex_capacity < (regex_length + ext_len + 4)) {
                regex_capacity *= 2;
                regex = ag_realloc(regex, regex_capacity);
            }

            // add a | before all but the first ext in the list
            if (first) {
                first = 0;
            } else {
                regex[regex_length++] = '|';
            }

            // memcpy is safe because we've already made sure regex is big enough
            memcpy(regex + regex_length, ext, ext_len);
            regex_length += ext_len;
        }
    }

    regex[regex_length++] = ')';
    regex[regex_length++] = '$';
    regex[regex_length++] = '\0';
    return regex;
}
