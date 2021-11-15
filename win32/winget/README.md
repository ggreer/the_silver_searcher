The winget package for installing ag.exe in Windows
===================================================


Since May 2021, winget.exe is the official package manager client for Windows 10 and later.  
It can be installed using instructions [there](https://github.com/microsoft/winget-cli), or as part of
the free [App Installer](https://www.microsoft.com/en-us/p/app-installer/9nblggh4nns1) package on the Microsoft Store.

This directory contains the tools for generating an ag_setup.exe program, and the corresponding winget manifest to
submit to Microsoft.

| *File*    | *Description*                                                                          |
|-----------|----------------------------------------------------------------------------------------|
| ag.yaml   | Template for the manifests to publish. !variable! will be expanded in the output file. |
| setup.bat | The installation script, that is automatically run after extracting all files from the ag_setup.exe program. |


Generating the winget package
-----------------------------

The winget package is composed of two parts:  
- A self-extracting setup program, called `ag_setup.exe`. This program is to be released on GitHub.
- A YAML manifest describing the release, called `%VERSION%.yaml`.

Prerequisites:  
- Install the [7-Zip LZMA SDK](https://www.7-zip.org/sdk.html).
- Configure it by running `configure.bat` in The Silver Searcher sources root directory.

ag_setup.exe and the yaml manifest are both generated when running `make release` from the project root directory.

Don't forget to update the `PACKAGE_VERSION` definition string in `src\config.h.MsvcLibX`, before building ag.exe.  
Then use that same %VERSION% string for publishing the release in GitHub. (Detailed further down)  
The release script automatically updates that !VERSION! in the yaml manifest.


Publishing the winget package
-----------------------------

ag_setup.exe is to be released along with ag.zip in the Silver Searcher repository at:  
https://github.com/JFLarvoire/the_silver_searcher/releases  
IMPORTANT: The Ag release *must* be named `%VERSION%-Windows`, to match the manifest contents.  
The files will then be available for download at:  
https://github.com/JFLarvoire/the_silver_searcher/releases/download/%VERSION%-Windows/  

Then test the winget manifest by running:
- `winget validate --manifest %VERSION%.yaml`
- `winget install -m %VERSION%.yaml`
- `winget list JFLarvoire.Ag`
- `winget uninstall JFLarvoire.Ag`

The winget manifest %VERSION%.yaml is to be renamed as JFLarvoire.Ag.yaml and pushed to:  
https://github.com/JFLarvoire/winget-pkgs/tree/master/manifests/j/JFLarvoire/Ag/%VERSION%/JFLarvoire.Ag.yaml  
Then a pull request is to be submitted to Microsoft at:  
https://github.com/microsoft/winget-pkgs/pulls


Ag Setup
--------

ag_setup.exe is a 7-Zip self-extracting installer program.  
Its contents are the same as ag.zip (but using the .7z format), plus the setup.bat file that does the installation.  
When ag_setup.exe is executed, it extracts eveything into a temporary directory, then runs setup.bat.  
See `DOC\installer.txt` in the 7-Zip LZMA SDK for a description of these self-extracting installer programs.

setup.bat, in turn, installs ag.exe into the first directory that matches one of these criteria:

 1. Over a previous instance of ag.exe *from the same [origin](https://github.com/JFLarvoire/the_silver_searcher)* found in the PATH
 2. In the directory defined in environment variable bindir_%PROCESSOR_ARCHITECTURE%
 3. In the directory defined in environment variable bindir
 4. In the directory defined in environment variable windir
 

Debugging Ag Setup installations
--------------------------------

In case of a problem, setup.bat logs everything it does into `%TEMP%\ag_setup.log`.
Analyzing this file allows to know what was done, and when. 


Generating the Chocolatey package
---------------------------------

For older versions of Windows, not supported by winget, it's still possible to use the Chocolatey package manager.

`make release` also generates two zip files with respectively the x86 and amd64 versions of ag.exe.  
Ex: `ag-2021-11-14-2.2.5-amd64.zip` and `ag-2021-11-14-2.2.5-x86.zip`

These two zip files should be released on GitHub together with the corresponding ag.zip and ag_setup.bat.


Publishing the Chocolatey package
---------------------------------

Update the nuspec manifest and tools at:   
https://github.com/AnthonyMastrean/chocolateypackages/tree/master/packages/ag  
Then submit a pull request to Anthony Mastrean.
