Setup:

  $ . $TESTDIR/setup.sh

Language types are output:

  $ ag --list-file-types
  The following file types are supported:
    --actionscript
        .as  .mxml
  
    --ada
        .ada  .adb  .ads
  
    --asciidoc
        .adoc  .ad  .asc  .asciidoc
  
    --apl
        .apl
  
    --asm
        .asm  .s
  
    --batch
        .bat  .cmd
  
    --bitbake
        .bb  .bbappend  .bbclass  .inc
  
    --bro
        .bro  .bif
  
    --cc
        .c  .h  .xs
  
    --cfmx
        .cfc  .cfm  .cfml
  
    --chpl
        .chpl
  
    --clojure
        .clj  .cljs  .cljc  .cljx
  
    --coffee
        .coffee  .cjsx
  
    --coq
        .coq  .g  .v
  
    --cpp
        .cpp  .cc  .C  .cxx  .m  .hpp  .hh  .h  .H  .hxx  .tpp
  
    --crystal
        .cr  .ecr
  
    --csharp
        .cs
  
    --css
        .css
  
    --cython
        .pyx  .pxd  .pxi
  
    --delphi
        .pas  .int  .dfm  .nfm  .dof  .dpk  .dpr  .dproj  .groupproj  .bdsgroup  .bdsproj
  
    --dlang
        .d  .di
  
    --dot
        .dot  .gv
  
    --dts
        .dts  .dtsi
  
    --ebuild
        .ebuild  .eclass
  
    --elisp
        .el
  
    --elixir
        .ex  .eex  .exs
  
    --elm
        .elm
  
    --erlang
        .erl  .hrl
  
    --factor
        .factor
  
    --fortran
        .f  .f77  .f90  .f95  .f03  .for  .ftn  .fpp
  
    --fsharp
        .fs  .fsi  .fsx
  
    --gettext
        .po  .pot  .mo
  
    --glsl
        .vert  .tesc  .tese  .geom  .frag  .comp
  
    --go
        .go
  
    --groovy
        .groovy  .gtmpl  .gpp  .grunit  .gradle
  
    --haml
        .haml
  
    --handlebars
        .hbs
  
    --haskell
        .hs  .lhs
  
    --haxe
        .hx
  
    --hh
        .h
  
    --html
        .htm  .html  .shtml  .xhtml
  
    --idris
        .idr  .ipkg  .lidr
  
    --ini
        .ini
  
    --ipython
        .ipynb
  
    --isabelle
        .thy
  
    --j
        .ijs
  
    --jade
        .jade
  
    --java
        .java  .properties
  
    --jinja2
        .j2
  
    --js
        .es6  .js  .jsx  .vue
  
    --json
        .json
  
    --jsp
        .jsp  .jspx  .jhtm  .jhtml  .jspf  .tag  .tagf
  
    --julia
        .jl
  
    --kotlin
        .kt
  
    --less
        .less
  
    --liquid
        .liquid
  
    --lisp
        .lisp  .lsp
  
    --log
        .log
  
    --lua
        .lua
  
    --m4
        .m4
  
    --make
        .Makefiles  .mk  .mak
  
    --mako
        .mako
  
    --markdown
        .markdown  .mdown  .mdwn  .mkdn  .mkd  .md
  
    --mason
        .mas  .mhtml  .mpl  .mtxt
  
    --matlab
        .m
  
    --mathematica
        .m  .wl
  
    --md
        .markdown  .mdown  .mdwn  .mkdn  .mkd  .md
  
    --mercury
        .m  .moo
  
    --naccess
        .asa  .rsa
  
    --nim
        .nim
  
    --nix
        .nix
  
    --objc
        .m  .h
  
    --objcpp
        .mm  .h
  
    --ocaml
        .ml  .mli  .mll  .mly
  
    --octave
        .m
  
    --org
        .org
  
    --parrot
        .pir  .pasm  .pmc  .ops  .pod  .pg  .tg
  
    --pdb
        .pdb
  
    --perl
        .pl  .pm  .pm6  .pod  .t
  
    --php
        .php  .phpt  .php3  .php4  .php5  .phtml
  
    --pike
        .pike  .pmod
  
    --plist
        .plist
  
    --plone
        .pt  .cpt  .metadata  .cpy  .py  .xml  .zcml
  
    --proto
        .proto
  
    --puppet
        .pp
  
    --python
        .py
  
    --qml
        .qml
  
    --racket
        .rkt  .ss  .scm
  
    --rake
        .Rakefile
  
    --restructuredtext
        .rst
  
    --rs
        .rs
  
    --r
        .r  .R  .Rmd  .Rnw  .Rtex  .Rrst
  
    --rdoc
        .rdoc
  
    --ruby
        .rb  .rhtml  .rjs  .rxml  .erb  .rake  .spec
  
    --rust
        .rs
  
    --salt
        .sls
  
    --sass
        .sass  .scss
  
    --scala
        .scala
  
    --scheme
        .scm  .ss
  
    --shell
        .sh  .bash  .csh  .tcsh  .ksh  .zsh  .fish
  
    --smalltalk
        .st
  
    --sml
        .sml  .fun  .mlb  .sig
  
    --sql
        .sql  .ctl
  
    --stylus
        .styl
  
    --swift
        .swift
  
    --tcl
        .tcl  .itcl  .itk
  
    --tex
        .tex  .cls  .sty
  
    --thrift
        .thrift
  
    --tla
        .tla
  
    --tt
        .tt  .tt2  .ttml
  
    --toml
        .toml
  
    --ts
        .ts  .tsx
  
    --twig
        .twig
  
    --vala
        .vala  .vapi
  
    --vb
        .bas  .cls  .frm  .ctl  .vb  .resx
  
    --velocity
        .vm  .vtl  .vsl
  
    --verilog
        .v  .vh  .sv
  
    --vhdl
        .vhd  .vhdl
  
    --vim
        .vim
  
    --wix
        .wxi  .wxs
  
    --wsdl
        .wsdl
  
    --wadl
        .wadl
  
    --xml
        .xml  .dtd  .xsl  .xslt  .ent  .tld  .plist
  
    --yaml
        .yaml  .yml
  
