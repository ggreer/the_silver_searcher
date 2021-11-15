###############################################################################
#									      #
#   File name	    lib2libs.mak					      #
#									      #
#   Description	    Generate LIBRARIES from LIB and LIBS		      #
#									      #
#   Notes	    For use with SysToolsLib's dos.mak, win32.mak, etc...     #
#									      #
#   History								      #
#    2018-01-12 JFL Created this make file.       			      #
#    2018-03-01 JFL Added a startup message.          			      #
#                   Also pass $(OBJECTS) to lib2libs.bat.		      #
#    2018-03-11 JFL Use variable _OBJECTS instead, to avoid too long lines.   *
#		    							      #
#         © Copyright 2018 Hewlett Packard Enterprise Development LP          #
# Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 #
###############################################################################

!MESSAGE Started lib2libs.mak

# Create the output directory if needed
!IF !EXIST($(O))
!  IF [$(MSG) Creating directory $(O) && md $(O)] != 0
!    ERROR Failed to create the $(O) directory
!  ENDIF
!ENDIF

# Invoke lib2libs.bat for doing the conversion
# Do not name the output file $(PROGRAM).mak, as this triggers unwanted .mak.inc rules.
LIB2LIBS_MAK=$(O)\$(PROGRAM).libraries.mak
# Use %STINCLUDE% instead of $(STINCLUDE). This saves 34 bytes on my system, which is enough to bring pthread4w command line below 4096 bytes!
LIB2LIBS_CMD=cmd /c "set "LIBS=$(LIBS)" & set "LIB=$(LIB)" & set "O=$(O)" & set "_OBJECTS=$(_OBJECTS)" & "%STINCLUDE%\lib2libs.bat" >$(LIB2LIBS_MAK)"
!MESSAGE  	$(LIB2LIBS_CMD) # Align this with nmake's own displayed commands. Begin the message with a hard space \xA0, to allow printing the TAB behind it.
!IF [$(LIB2LIBS_CMD)] == 0
!  MESSAGE Getting generated LIBRARIES list from $(LIB2LIBS_MAK).
!  INCLUDE $(LIB2LIBS_MAK)
!ELSE
!  ERROR Failed to generate the LIBRARIES list
!ENDIF
