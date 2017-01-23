# Microsoft Developer Studio Project File - Name="Gen_m3a" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=Gen_m3a - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Gen_m3a.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Gen_m3a.mak" CFG="Gen_m3a - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Gen_m3a - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Gen_m3a - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "Perforce Project"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Gen_m3a - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GEN_M3A_EXPORTS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /Od /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GEN_M3A_EXPORTS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x1009 /d "NDEBUG"
# ADD RSC /l 0x1009 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 advapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib version.lib Shlwapi.lib comctl32.lib coolsb_detours.lib /nologo /dll /machine:I386 /nodefaultlib:"libc.lib"
# SUBTRACT LINK32 /debug /nodefaultlib
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy release\gen_m3a.dll c:\progra~1\winamp\plugins	copy release\gen_m3a.dll install
# End Special Build Tool

!ELSEIF  "$(CFG)" == "Gen_m3a - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GEN_M3A_EXPORTS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GEN_M3A_EXPORTS" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x1009 /d "_DEBUG"
# ADD RSC /l 0x1009 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib version.lib Shlwapi.lib comctl32.lib coolsb_detours.lib /nologo /dll /debug /machine:I386 /nodefaultlib:"libc.lib" /pdbtype:sept
# SUBTRACT LINK32 /nodefaultlib
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy debug\gen_m3a.dll c:\progra~1\winamp\plugins
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "Gen_m3a - Win32 Release"
# Name "Gen_m3a - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Album.cpp
# End Source File
# Begin Source File

SOURCE=.\AlbumList.cpp
# End Source File
# Begin Source File

SOURCE=.\Gen_m3a.cpp
# End Source File
# Begin Source File

SOURCE=.\HTMLite.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\Util.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Album.h
# End Source File
# Begin Source File

SOURCE=.\AlbumList.h
# End Source File
# Begin Source File

SOURCE=.\ALFront.h
# End Source File
# Begin Source File

SOURCE=.\Gen_m3a.h
# End Source File
# Begin Source File

SOURCE=.\HTMLite.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\Util.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\Cover.bmp
# End Source File
# Begin Source File

SOURCE=.\Cover_mask.bmp
# End Source File
# Begin Source File

SOURCE=.\Gen_m3a.rc
# End Source File
# Begin Source File

SOURCE=.\icon_al.bmp
# End Source File
# Begin Source File

SOURCE=.\icon_al_h.bmp
# End Source File
# End Group
# Begin Group "Winamp API Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\AutoChar.h
# End Source File
# Begin Source File

SOURCE=.\AutoWide.h
# End Source File
# Begin Source File

SOURCE=.\Gen.h
# End Source File
# Begin Source File

SOURCE=.\ipc_pe.h
# End Source File
# Begin Source File

SOURCE=.\ml.h
# End Source File
# Begin Source File

SOURCE=.\ml_ipc.h
# End Source File
# Begin Source File

SOURCE=.\NxSThingerAPI.h
# End Source File
# Begin Source File

SOURCE=.\wa_dlg.h
# End Source File
# Begin Source File

SOURCE=.\wa_hotkeys.h
# End Source File
# Begin Source File

SOURCE=.\wa_ipc.h
# End Source File
# Begin Source File

SOURCE=.\wa_msgids.h
# End Source File
# End Group
# Begin Group "Config"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ConfigAbout.cpp
# End Source File
# Begin Source File

SOURCE=.\ConfigAbout.h
# End Source File
# Begin Source File

SOURCE=.\ConfigAboutML.cpp
# End Source File
# Begin Source File

SOURCE=.\ConfigAboutML.h
# End Source File
# Begin Source File

SOURCE=.\ConfigCover.cpp
# End Source File
# Begin Source File

SOURCE=.\ConfigCover.h
# End Source File
# Begin Source File

SOURCE=.\ConfigHistory.cpp
# End Source File
# Begin Source File

SOURCE=.\ConfigHistory.h
# End Source File
# Begin Source File

SOURCE=.\ConfigLanguage.cpp
# End Source File
# Begin Source File

SOURCE=.\ConfigLanguage.h
# End Source File
# Begin Source File

SOURCE=.\ConfigMain.cpp
# End Source File
# Begin Source File

SOURCE=.\ConfigMain.h
# End Source File
# Begin Source File

SOURCE=.\ConfigNaming.cpp
# End Source File
# Begin Source File

SOURCE=.\ConfigNaming.h
# End Source File
# Begin Source File

SOURCE=.\ConfigOption.cpp
# End Source File
# Begin Source File

SOURCE=.\ConfigOption.h
# End Source File
# Begin Source File

SOURCE=.\ConfigPath.cpp
# End Source File
# Begin Source File

SOURCE=.\ConfigPath.h
# End Source File
# Begin Source File

SOURCE=.\ConfigProfile.cpp

!IF  "$(CFG)" == "Gen_m3a - Win32 Release"

# ADD CPP /Od

!ELSEIF  "$(CFG)" == "Gen_m3a - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ConfigProfile.h
# End Source File
# Begin Source File

SOURCE=.\Preference.cpp
# End Source File
# Begin Source File

SOURCE=.\Preference.h
# End Source File
# End Group
# Begin Group "Downloaded Source"

# PROP Default_Filter ""
# Begin Group "CoolSB"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CoolSB_detours.h
# End Source File
# Begin Source File

SOURCE=.\CoolScroll.h
# End Source File
# End Group
# Begin Group "API Hijack"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\APIHijack.cpp
# End Source File
# Begin Source File

SOURCE=.\APIHijack.h
# End Source File
# End Group
# Begin Group "RLE Compression"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\rle.cpp
# End Source File
# Begin Source File

SOURCE=.\rle.h
# End Source File
# End Group
# Begin Group "VisualStylesXP"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\VisualStylesXP.cpp
# End Source File
# Begin Source File

SOURCE=.\VisualStylesXP.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\AggressiveOptimize.h
# End Source File
# Begin Source File

SOURCE=.\crc32.cpp
# End Source File
# Begin Source File

SOURCE=.\URLMon.cpp
# End Source File
# End Group
# Begin Group "User Interface"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\AlbumInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\AlbumInfo.h
# End Source File
# Begin Source File

SOURCE=.\JumpToAlbum.cpp
# End Source File
# Begin Source File

SOURCE=.\JumpToAlbum.h
# End Source File
# Begin Source File

SOURCE=.\ProfileName.cpp
# End Source File
# Begin Source File

SOURCE=.\ProfileName.h
# End Source File
# Begin Source File

SOURCE=.\TabbedUI.cpp
# End Source File
# Begin Source File

SOURCE=.\TabbedUI.h
# End Source File
# Begin Source File

SOURCE=.\UserInterface.cpp
# End Source File
# Begin Source File

SOURCE=.\UserInterface.h
# End Source File
# Begin Source File

SOURCE=.\UserInterface_ListCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\UserInterface_ListCtrl.h
# End Source File
# Begin Source File

SOURCE=.\WinampAL.cpp
# End Source File
# Begin Source File

SOURCE=.\WinampAL.h
# End Source File
# Begin Source File

SOURCE=.\WinampGen.cpp
# End Source File
# Begin Source File

SOURCE=.\WinampGen.h
# End Source File
# Begin Source File

SOURCE=.\WinampMain.cpp
# End Source File
# Begin Source File

SOURCE=.\WinampMain.h
# End Source File
# Begin Source File

SOURCE=.\WinampPE.cpp
# End Source File
# Begin Source File

SOURCE=.\WinampPE.h
# End Source File
# End Group
# Begin Group "File Info"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CAPEInfo.h
# End Source File
# Begin Source File

SOURCE=.\CFrameHeader.cpp
# End Source File
# Begin Source File

SOURCE=.\CFrameHeader.h
# End Source File
# Begin Source File

SOURCE=.\CId3Tag.cpp
# End Source File
# Begin Source File

SOURCE=.\CId3Tag.h
# End Source File
# Begin Source File

SOURCE=.\CM4AInfo.h
# End Source File
# Begin Source File

SOURCE=.\CMP3Info.cpp
# End Source File
# Begin Source File

SOURCE=.\CMP3Info.h
# End Source File
# Begin Source File

SOURCE=.\CMPCInfo.h
# End Source File
# Begin Source File

SOURCE=.\CPSFInfo.h
# End Source File
# Begin Source File

SOURCE=.\CSPCInfo.h
# End Source File
# Begin Source File

SOURCE=.\CVBitRate.cpp
# End Source File
# Begin Source File

SOURCE=.\CVBitRate.h
# End Source File
# Begin Source File

SOURCE=.\CWAExtendedInfo.h
# End Source File
# Begin Source File

SOURCE=.\CWAVInfo.h
# End Source File
# Begin Source File

SOURCE=.\CWMAInfo.h
# End Source File
# Begin Source File

SOURCE=.\FileInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\FileInfo.h
# End Source File
# End Group
# Begin Group "Drag & Drop"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\DataObject.cpp
# End Source File
# Begin Source File

SOURCE=.\DropSource.cpp
# End Source File
# Begin Source File

SOURCE=.\EnumFormat.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\Install\Gen_m3a.nsi
# End Source File
# Begin Source File

SOURCE=.\Gen_m3a.ver

!IF  "$(CFG)" == "Gen_m3a - Win32 Release"

USERDEP__GEN_M="Release\Gen_m3a.dll"	
# Begin Custom Build
InputPath=.\Gen_m3a.ver

"a.out" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	verupdate.exe $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "Gen_m3a - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Install\History.txt
# End Source File
# Begin Source File

SOURCE=.\Install\Readme\readme.htm
# End Source File
# Begin Source File

SOURCE=.\Install\Languages\sample.al
# End Source File
# End Target
# End Project
