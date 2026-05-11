; Build the pthreads library with the Digital Mars Compiler
;
set DMCDIR=c:\dm

;   RELEASE
%DMCDIR%\bin\dmc -D_MT -DHAVE_CONFIG_H -I.;c:\dm\include -o+all -WD pthread.c user32.lib+kernel32.lib+wsock32.lib -L/impl -L/NODEBUG -L/SU:WINDOWS

;   DEBUG
%DMCDIR%\bin\dmc -g -D_MT -DHAVE_CONFIG_H -I.;c:\dm\include -o+all -WD pthread.c user32.lib+kernel32.lib+wsock32.lib -L/impl -L/SU:WINDOWS
