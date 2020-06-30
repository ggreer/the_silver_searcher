The winget package for installing ag.exe in Windows
===================================================


winget is intended to become the default package manager client for Windows.  
As of June 2020, it's still in beta, and can be installed using instructions [there](https://github.com/microsoft/winget-cli).

This directory contains the tools for generating an ag_setup.exe program, and the corresponding winget manifest to
submit to Microsoft.  
They're both generated when running `make release` from the project root directory,
if the [7-Zip LZMA SDK](https://www.7-zip.org/sdk.html) is installed and configured using `configure.bat`.

Don't forget to update the `PACKAGE_VERSION` definition string in `src\config.h.MsvcLibX`, before building ag.exe.  
Then use that same %VERSION% string for publishing the release in GitHub. (Detailed further down)

ag_setup.exe is a 7-Zip self-extracting installer program.  
Its contents are the same as ag.zip (but using the .7z format), plus the setup.bat file that does the installation.  
When ag_setup.exe is executed, it extracts eveything into a temporary directory, then runs setup.bat.  
See `DOC\installer.txt` in the 7-Zip LZMA SDK for a description of these self-extracting installer programs.

setup.bat, in turn, installs ag.exe into the first directory that matches one of these criteria:

 1. Over a previous instance of ag.exe *from the same [origin](https://github.com/JFLarvoire/the_silver_searcher)* found in the PATH
 2. In the directory defined in environment variable bindir_%PROCESSOR_ARCHITECTURE%
 3. In the directory defined in environment variable bindir
 4. In the directory defined in environment variable windir

ag_setup.exe is to be released along with ag.zip in the Silver Searcher repository at:  
https://github.com/JFLarvoire/the_silver_searcher/releases/download/%VERSION%-Windows/  
And so the Ag release *must* be named `%VERSION%-Windows`, to match the manifest contents.

The winget manifest %VERSION%.yaml is to be pushed to:  
https://github.com/JFLarvoire/winget-pkgs/tree/master/manifests/JFLarvoire/Ag/  
Then a pull request is to be submitted to Microsoft at:  
https://github.com/microsoft/winget-pkgs/pulls
