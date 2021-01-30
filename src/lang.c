#include <stdlib.h>
#include <string.h>

#include "lang.h"
#include "util.h"

#ifdef _WIN32
#define strcasecmp _stricmp
#endif

#define MAX_LANGS 1000

lang_spec_t langs[MAX_LANGS] = {
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

lang_spec_t *get_lang_slot() {
    lang_spec_t *first_free = langs;
    for (; first_free < langs + MAX_LANGS && first_free->name; ++first_free)
        ;

    return first_free;
}

char const *lang_add_ext(lang_spec_t *l, char const *ext) {
    const char **pExt = l->extensions;
    size_t i = 0;
    for (; i < MAX_EXTENSIONS && *pExt; ++pExt, ++i)
        ;

    if (!*pExt) {
        return (*pExt = strdup(ext));
    } else {
        return (*pExt);
    }
}

lang_spec_t *lang_new(char const *name) {
    lang_spec_t *result = get_lang_slot();
    if (result < langs + MAX_LANGS) {
        result->name = strdup(name);
        return result;
    }

    return 0;
}

lang_spec_t *lang_find(char const *name) {
    lang_spec_t *result = langs;
    for (; result < langs + MAX_LANGS; ++result)
        if (!strcasecmp(name, result->name))
            return result;

    return 0;
}

lang_spec_t *lang_parse_spec(char const *spec) {
    lang_spec_t *result = 0;
    char *_spec = strdup(spec);
    if (_spec) {
        const char *name = strtok(_spec, " ");
        if (name) {
            const char *op = strtok(NULL, " ");
            if (op) {
                switch (op[0]) {
                    case ':':
                        result = lang_new(name);
                        break;
                    case '+':
                        result = lang_find(name);
                        break;
                }
            }
        }

        if (result) {
            const char *ext = 0;
            while (ext = strtok(NULL, " ,"))
                lang_add_ext(result, ext);
        }

        free(_spec);
    }

    return result;
}

void lang_parse_file(const char *path) {
    FILE *f = fopen(path, "rt");
    if (f) {
        char *line = 0;
        size_t len = 0, read = 0;

        while ((read = getline(&line, &len, f)) != -1) {
            char *line_contents = strtok(line, "\n");
            if (line_contents) {
                if (line_contents[0] != '#' && line_contents[0] != 0)
                    lang_parse_spec(line_contents);
            }
        }

        if (line)
            free(line);
        fclose(f);
    }
}

void lang_parse_user_spec() {
    char dot_ag_path[1024] = { 0 };
#ifdef _WIN32
    strncat(dot_ag_path, getenv("USERPROFILE"), sizeof(dot_ag_path));
#else
    strncat(dot_ag_path, getenv("HOME"), sizeof(dot_ag_path));
#endif
    strncat(dot_ag_path, "\\.aglang", sizeof(dot_ag_path));
    if (access(dot_ag_path, 0) != -1)
        lang_parse_file(dot_ag_path);
}

size_t get_lang_count() {
    return (get_lang_slot() - langs);
}

lang_spec_t const *get_langs(void) {
    return langs;
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
