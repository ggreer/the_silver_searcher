AG for Windows release history and change log
=============================================


2026-05-03 - Version 2.2.7 - Updated libraries and new ARM & ARM64 versions
---------------------------------------------------------------------------

New:
- This release now includes ARM and ARM64 versions of ag.exe.

Updated libraries:
- MsvcLibX to version 2026-04-16
- PCRE to version 8.45 2021-06-15
- pthread-win32 to version 3.0.3
- zlib to version 1.3.2

Fixed:
- Bug #7 ".gitignore patterns respected by Linux ag don't work in Windows ag" fixed by the MsvcLibX upgrade.
- Bug #8 "Where the hell is it?" fixed by ag_setup.exe now displaying a message box in the end, confirming the installation location.


2026-03-05 - Version 2.2.6 - Updated MsvcLibX library
-----------------------------------------------------

This release includes the latest MsvcLibX library from [SysToolsLib](https://github.com/JFLarvoire/SysToolsLib).  
The most visible change is that Unicode characters outside of plane 0 (= characters with code points > U+FFFF)
(= those with more than 16 bits) are now displayed correctly.


2021-11-14 - Version 2.2.5 - Updated setup
------------------------------------------

This release only includes improvements and bug fixes in the setup program.  
The ag.exe program itself is unchanged, apart from the version string.

- The setup script now adds an entry in the "Add / Remove Programs" list.
   - This fixes the issue with winget, which did not list The Silver Searcher as installed after successfully installing it.
   - This also allows uninstalling The Silver Searcher either using winget or using the Windows Control Panel.
- The setup script now detects ag.exe instances installed by Chocolatey or Scoop, and aborts the installation in this case.
   It displays a message recommending to uninstall the other packages first, using their respective package managers.
   This is done to prevent conflicts between older versions of ag.exe that may be earlier in the PATH.

The release now also includes two architecture-specific zip files, for use by Chocolatey, etc.


2021-06-04 - Version 2.2.4 - Better support for Unicode
-------------------------------------------------------

- Can now search in UTF-16 text files, in addition to the UTF-8 and Windows system code page files that were already supported.  
  Note that searching in UTF-16 files is slower than for the other types of files supported, because the text is converted to UTF-8 first.
- Can now search in UTF-8 text piped in via standard input in any code page, in addition to the current console code page that was already supported.
- Converts \xXX, \uXXXX, and \UXXXXXXXX escape sequences in the search pattern to the equivalent Unicode character.
- Added detailed explanations in the help screen about input and output encoding rules and limitations.
- Bug fix: The console text color was not restored properly in case Ctrl-C was used to abort a long search.
- Merged all changes from the Unix master sources as of 2021-06-03 (where the version still is 2.2), including several bug fixes, and several new known file types.
- Updated the MsvcLibX library to version 2021-06-03.
- Updated the PCRE library to version 8.44.

### Details on the Unicode escape sequences support

The previous versions of ag.exe only handled \xXX escape sequences for defining bytes in regular expressions.
Due to limitations of the PCRE 1 library, the Unicode escape sequences \uXXXX & \UXXXXXXXX were not supported.
Also no such escape sequence replacement was done when searching for fixed strings.

This version converts \xXX, \uXXXX, and \UXXXXXXXX escape sequences in the pattern string to the equivalent Unicode character.
This is done in the argument processing phase, prior to passing the search pattern to the search functions.
Use option --verbose to display the pattern string generated.

This conversion is done when searching for regular expressions, and for fixed strings (Using option -F|--fixed-strings).
It is NOT done when using option -Q/--literal. (So in that sense, the -F and -Q options aren't strictly equivalent anymore.)

Example of use: List UTF-8 and UTF-16 text files that contain a Unicode BOM:

    ag -l \uFEFF

Caution: \x80 to \x9F are invalid Unicode code points.
To search for the Euro sign, use either "€" or "\u20AC", but not "\x80", even for CP 1252 files.
Likewise, to search for all non-ASCII characters, use "[^\x00-\x7F]", not "[\x80-\xFF]".


2019-04-27 - Version 2.2.3 - Expand wildcards in command-line arguments
-----------------------------------------------------------------------

The main improvement is that this version now expands wildcards in command-line arguments.  
This is useful because cmd.exe and PowerShell, contrary to Unix shells, don't expand wildcards.  
Ex: `ag greer src\*.h`

Other changes:
* Merged in the latest updates from Geoff Greer's master version.
* Convert path separators in error messages back to a \\.
* Display a more detailed version for the Windows port with option -V.


2018-11-20 - Version 2.2.2 - Sync with master 2.2.0 + pthreads4w 3.0.0 + minor bug fix
--------------------------------------------------------------------------------------

* Merged in the latest updates from the master version for Unix.
* Updated the pthread4w library to the new version 3.0.0.
* Fixed output line endings, which often had two CR before the LF.
  This was invisible in the console, but sometimes confused other apps, when ag.exe output was piped to them.


2018-05-13 - Version 2.2.1 - Fix for bug #4, two libraries updates, and help improvements
-----------------------------------------------------------------------------------------

Changes in this release:

* Fix for bug #4 - Getting crashed when using --context option
* Updated the MsvcLibX library to version 2018-05-02
* Updated the PCRE library to version 8.42
* Moved the display of the two text file encodings supported from --version to --help.
* Updated --version to only display if the two-encodings feature for text files is supported or not.
* Updated --help to list contributors to this Windows version, and where it is available.


2018-03-14 - Version 2.2.0 - Version 2.2.0 + ε
----------------------------------------------

* Merged in changes from [Unix version 2.2.0](https://github.com/ggreer/the_silver_searcher/releases/tag/2.2.0),
  and subsequent updates to this day.
* Upgraded the pthreads4w library to the latest version.
* Upgraded the MsvcLibX library to the latest version.
* Improved the make system. It now works with Visual Studio version 12 to 15. (2013 to 2017)  
  (Older versions do not support C99.)


2017-09-01 - Version 2.1.0 - Merged-in changes from Unix version 2.1.0
----------------------------------------------------------------------

Merged-in changes from [Unix version 2.1.0](https://github.com/ggreer/the_silver_searcher/releases/tag/2.1.0),
and fixed bug #3.


2017-08-10 - Version 2.0.0 - Support two text encodings at once
---------------------------------------------------------------

This is a new build for Windows, based on the July 2017 ag 2.0.0 sources.

- Supports Windows XP and newer versions of Windows. (Windows Server 2003 minimum for the 64-bits version.)
- Supports path names up to 64KB. (Using WIN32 APIs not limited to 260 bytes, even in XP.)
- Supports Unicode path names.
- Supports symbolic links and junctions.
- Supports text files encoded both in UTF-8 and in the current Windows System Code Page.  
  (The latter depends on the Windows localization. Ex: CP 1252 for US and West-European versions of Windows.)
- Supports Unicode search strings and regular expressions.  
  (Some limitations for regexps searches in files encoded in Windows system CP, especially for MBCS CPs.)  
- Outputs non-ASCII characters correctly in any console code page.  
  (Provided that the console font has bitmaps for them. See the table below.)  
  (But if the output is piped to a file or another program, it'll be converted to the current console code page,
  and unsupported characters will be lost. This is a limitation of the Windows console, not of ag.exe.)

The zip file contains 32 and 64-bits ag.exe versions.

Console font          | Character families it can display
--------------------- | ---------------------------------------------------------------------
Arial sans MS         | Latin; Greek; Cyrillic
Liberation Mono       | Latin; Greek; Cyrillic; Hebrew
Microsoft YaHei       | Latin; Greek; Cyrillic; CJK Ideograms; Hiragana; Katakana; Hangul
Yu Gothic             | Latin; Greek; Cyrillic; Dingbats; CJK Ideograms; Hiragana; Katakana


2017-04-05 - Version 1.0.2 - ag.exe for Windows
-----------------------------------------------

This is a new build for Windows, based on the March 2017 ag [sources](https://github.com/JFLarvoire/the_silver_searcher).
- Supports and displays all Unicode pathnames in any code page. (Provided that the console fonts support them.)
- Can search for Unicode strings and regular expressions.
- Supports pathnames > 260 characters, up to at least 4KB.
- Supports symbolic links and junctions.

The zip file contains 32 and 64-bits ag.exe versions.

Known limitation: The regular expressions engine will refuse to search in files with non-ASCII characters encoded
in code pages other than UTF-8. For example, "Fran.ois" will match "François" in a UTF-8 file,
but NOT in an ANSI file encoded in code page 1252. Worse still, if an ANSI file contains non-ASCII characters, like '©',
then you can't even search for ASCII regular expressions. Only for fixed ASCII strings.

