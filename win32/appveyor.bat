@echo off
:: Batch file for building/testing Ag on AppVeyor


cd %APPVEYOR_BUILD_FOLDER%
if "%1"=="" (
  set target=build
) else (
  set target=%1
)

for %%i in (msys2 cygwin) do if "%compiler%"=="%%i" goto %compiler%_%target%

echo Unknown build target.
exit 1

:msys2_build
:: ----------------------------------------------------------------------
:: Using msys2
@echo on
PATH C:\%MSYS2_DIR%\%MSYSTEM%\bin;C:\%MSYS2_DIR%\usr\bin;%PATH%
set CHERE_INVOKING=yes
bash -lc 'if [ "x$(checkupdates)" != x ]; then exit 1; fi'
if ERRORLEVEL 1 (
	@rem Update found
	@rem Remove unused toolchain to reduce the time for updating
	if "%MSYSTEM%"=="MINGW64" (
		bash -lc "pacman --noconfirm -Rs mingw-w64-i686-toolchain"
	) else if "%MSYSTEM%"=="MINGW32" (
		bash -lc "pacman --noconfirm -Rs mingw-w64-x86_64-toolchain"
	)
	@rem Synchronize package databases and upgrade the core system
	C:\%MSYS2_DIR%\usr\bin\pacman --noconfirm --noprogressbar -Syu
	@rem Run again to update the rest of packages
	C:\%MSYS2_DIR%\usr\bin\pacman --noconfirm --noprogressbar -Su
)
:: Install necessary packages
bash -lc "for i in {1..3}; do pacman --noconfirm --noprogressbar -S --needed mingw-w64-%MSYS2_ARCH%-{pcre,xz} && break || sleep 15; done"

bash -lc "./autogen.sh"
:: Patching configure.
:: Workaround for "./configure: line xxx: 0: Bad file descriptor"
perl -i".bak" -pe "s/^test -n \".DJDIR\"/#$&/" configure
:: Use static link.
bash -lc "./configure PCRE_CFLAGS=-DPCRE_STATIC LDFLAGS=-static && make"

@echo off
goto :eof

:msys2_test
@echo on
:: Check filetype
c:\cygwin\bin\file ag.exe
:: Check if it works
.\ag --version || exit 1

:: Run tests
rem bash -lc "make test"

@echo off
goto :eof

:msys2_package
md package

:: Get version
if "%APPVEYOR_REPO_TAG_NAME%"=="" (
  for /f %%i in ('git rev-parse --short HEAD') do set ver=%%i
) else (
  set ver=%APPVEYOR_REPO_TAG_NAME%
)

:: Create zip package
set filelist=ag.exe LICENSE README.md
robocopy . package %filelist% > nul
cd package
rem 7z a ..\ag-%ver%-%ARCH%.debug.zip %filelist%
strip *.exe
7z a ..\ag-%ver%-%ARCH%.zip %filelist%
cd ..
goto :eof


:cygwin_build
:: ----------------------------------------------------------------------
:: Using Cygwin, iconv enabled
@echo on
c:\cygwin\setup-x86.exe -qnNdO -P liblzma-devel,libpcre-devel,python3,python3-setuptools
PATH c:\cygwin\bin;%PATH%
set CHERE_INVOKING=yes
bash -lc "./autogen.sh"
:: Patching configure.
:: Workaround for "./configure: line XXX: 0: Bad file descriptor"
perl -i".bak" -pe "s/^test -n \".DJDIR\"/#$&/" configure
bash -lc "./configure && make"

@echo off
goto :eof

:cygwin_test
@echo on
:: Check filetype
c:\cygwin\bin\file ag.exe
:: Check if it works
.\ag --version || exit 1

:: Run tests
bash -lc "python3 -m easy_install pip && pip install cram"
bash -lc "make test"

@echo off
goto :eof

:cygwin_package
:: Do nothing.
goto :eof
