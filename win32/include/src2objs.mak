###############################################################################
#									      #
#   File name	    src2objs.mak					      #
#									      #
#   Description	    Convert the SOURCES list to an OBJECTS list		      #
#									      #
#   Notes	    For use with SysToolsLib's dos.mak, win32.mak, etc...     #
#									      #
#   History								      #
#    2017-03-01 JFL Created this make file.       			      #
#    2017-11-12 JFL Changed the output file name to $(PROGRAM).objects.       #
#    2018-01-12 JFL Changed the output make file name.			      #
#		    							      #
#         © Copyright 2017 Hewlett Packard Enterprise Development LP          #
# Licensed under the Apache 2.0 license - www.apache.org/licenses/LICENSE-2.0 #
###############################################################################

# Create the output directory if needed
!IF !EXIST($(O))
!  IF [$(MSG) Creating directory $(O) && md $(O)] != 0
!    ERROR Failed to create the $(O) directory
!  ENDIF
!ENDIF

# Invoke src2objs.bat for doing the conversion
# Do not name the output file $(PROGRAM).mak, as this triggers unwanted .mak.inc rules.
SRC2OBJS_MAK=$(O)\$(PROGRAM).objects.mak
SRC2OBJS_CMD="$(STINCLUDE)\src2objs.bat" -o $(O) $(SOURCES) >$(SRC2OBJS_MAK)
!MESSAGE $(SRC2OBJS_CMD)
!IF [$(SRC2OBJS_CMD)] == 0
!  MESSAGE Getting generated OBJECTS list from $(SRC2OBJS_MAK).
!  INCLUDE $(SRC2OBJS_MAK)
!ELSE
!  ERROR Failed to generate the OBJECTS file list
!ENDIF
