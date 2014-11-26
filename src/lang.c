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
    { "liquid", { "liquid" } },
    { "lisp", { "lisp", "lsp" } },
    { "lua", { "lua" } },
    { "m4", { "m4" } },
    { "make", { "Makefiles", "mk", "mak" } },
    { "markdown", { "markdown", "mdown", "mdwn", "mkdn", "mkd", "md" } },
    { "mason", { "mas", "mhtml", "mpl", "mtxt" } },
    { "matlab", { "m" } },
    { "objc", { "m", "h" } },
    { "objcpp", { "mm", "h" } },
    { "ocaml", { "ml", "mli", "mll", "mly" } },
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
    { "yaml", { "yaml", "yml" } }
};

unsigned int get_lang_count() {
    return sizeof(langs) / sizeof(lang_spec_t);
}

char *make_lang_regex(char* ext_array, unsigned int num_exts) {
    int regex_capacity = 100;
    char *regex = ag_malloc(regex_capacity);
    int regex_length = 3;
    int subsequent = 0;
    char *extension;
    unsigned int i;

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

unsigned int
combine_file_extensions(unsigned int*   extension_index,
                        unsigned int    len,
                        char**          exts)
{
    /* Keep it fixed as 100 for the reason that if you have more than 100 
     * file types to search, you'd better search all the files.
     * */
    unsigned int ext_capacity = 100;
    (*exts) = (char*)ag_malloc(ext_capacity * SINGLE_EXT_LEN);
    memset((*exts), 0, ext_capacity * SINGLE_EXT_LEN);
    unsigned int num_of_extensions= 0;

    unsigned int i = 0;
    for (; i < len; ++i) {
        unsigned int j = 0;
        const char* ext = langs[extension_index[i]].extensions[j];
        do {
            if (num_of_extensions == ext_capacity) {
                break;
            }
            char* pos = (*exts) + num_of_extensions * SINGLE_EXT_LEN;
            strncpy(pos, ext, strlen(ext));
            ++num_of_extensions;
            ext = langs[extension_index[i]].extensions[++j];
        } while(ext);
    }

    return num_of_extensions;
}


