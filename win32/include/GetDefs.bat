@echo off
:# Get macro definitions for a specific program
:# %1 = Make file to parse
:# %2 = PROGRAM base name
:# Return 0 = Found
:# Return 1 = Not Found

setlocal EnableExtensions EnableDelayedExpansion

set "RESULT=1"

if not exist %1 exit /b 1

for %%v in (SOURCES OBJECTS LIBRARIES) do (
  set "LINE="
  for /f "delims=" %%l in ('findstr /C:%~2_%%v %1') do set "LINE=%%l"
  
  if defined LINE (
    echo !LINE:%~2_%%v=%%v!
    set "RESULT=0"
  )
)

exit /b %RESULT%
