!define VER_MAJOR 2
!define VER_MINOR 07

;!define MUI_PRODUCT "Album List" ;Define your own software name here
;!define MUI_VERSION "${VER_MAJOR}.${VER_MINOR}" ;Define your own software version here

!include "MUI.nsh"

;--------------------------------
;Configuration

  ;General
  Name "Album List ${VER_MAJOR}.${VER_MINOR}"
  OutFile AlbumListv${VER_MAJOR}${VER_MINOR}.exe

  ;Folder selection page
  InstallDir $PROGRAMFILES\Winamp
  
  ;Remember install folder
  InstallDirRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Winamp" "UninstallString"

  ;Install types
  InstType "Full"
  InstType "Normal"
  InstType "Lite"

  BrandingText " "
  CRCCheck on
  WindowIcon off
  ShowInstDetails show

  ;Compiler Flags

  SetCompress auto
  SetCompressor /solid lzma
  SetDatablockOptimize on
  SetDateSave on
  SetOverwrite on

;--------------------------------
;Modern UI Configuration

  !define MUI_HEADERIMAGE
  !define MUI_HEADERIMAGE_RIGHT

  !define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\orange-install.ico"
  !define MUI_WELCOMEFINISHPAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Wizard\orange.bmp"
  !define MUI_HEADERIMAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Header\orange-r.bmp"

  !define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\orange-uninstall.ico"
  !define MUI_UNWELCOMEFINISHPAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Wizard\orange-uninstall.bmp"
  !define MUI_HEADERIMAGE_UNBITMAP "${NSISDIR}\Contrib\Graphics\Header\orange-uninstall-r.bmp"

  !define MUI_COMPONENTSPAGE_CHECKBITMAP "${NSISDIR}\Contrib\Graphics\Checks\simple-round2.bmp"

  !define MUI_ABORTWARNING
  !define MUI_COMPONENTSPAGE_SMALLDESC
  !define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\Plugins\Album List\readme.htm"
  !define MUI_FINISHPAGE_RUN  "$INSTDIR\winamp.exe"
  !define MUI_FINISHPAGE_RUN_TEXT "Run Winamp"
  !define MUI_FINISHPAGE_NOREBOOTSUPPORT

  !insertmacro MUI_PAGE_WELCOME
  !insertmacro MUI_PAGE_LICENSE "license.txt"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  !insertmacro MUI_PAGE_FINISH


  !insertmacro MUI_UNPAGE_WELCOME
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  !insertmacro MUI_UNPAGE_FINISH
  
;--------------------------------
;Languages
 
  !define MUI_BRANDINGTEXT " "
  !insertmacro MUI_LANGUAGE "English"
  
;--------------------------------
;Language Strings

  ;Description
  LangString DESC_SecCore ${LANG_ENGLISH} "The core files required to use Album List"
  LangString DESC_SecLanguages ${LANG_ENGLISH} "Optional languages files"

;--------------------------------
;Installer Sections

Section "Album List Plugin (required)" SecCore

  ;ADD YOUR OWN STUFF HERE!

  SectionIn 1 2 3 4 RO

  SetOutPath $INSTDIR\Plugins
  File gen_m3a.dll

  SetOutPath "$INSTDIR\Plugins\Album List"
  File Readme\readme.htm
  File Readme\walogo.gif
  File Readme\media.jpg
  File Readme\title.jpg
  File Readme\profile.jpg
  File History.txt

  SetOutPath "$INSTDIR\Plugins\Album List\Templates"
  File Templates\export.htt
  File Templates\cover.htt
  File Templates\wmp.jpg
  File Templates\vinyl.jpg

SectionEnd
 
!macro LangSection SecName SecTag LangFile
  Section "${SecName}" ${SecTag}
  SectionIn 1 2
  SetOutPath "$INSTDIR\Plugins\Album List\Language"
  File "Languages\${LangFile}.al"
  SectionEnd
!macroend

SubSection "Language Files" SecLanguages
  
  !insertmacro LangSection "Sample" SecSample sample
  !insertmacro LangSection "Bulgarian" SecBulgarian bulgarian
  !insertmacro LangSection "Catalan" SecCatalan catalan
  !insertmacro LangSection "Chinese (Simplified)" SecChineseS "simplified chinese"
  !insertmacro LangSection "Chinese (Traditional)" SecChineseT "traditional chinese"
  !insertmacro LangSection "Czech" SecCzech czech
  !insertmacro LangSection "Danish" SecDanish danish
  !insertmacro LangSection "Dutch" SecDutch dutch
  !insertmacro LangSection "Faroese" SecFaroese faroese
  !insertmacro LangSection "Finnish" SecFinnish finnish
  !insertmacro LangSection "French" SecFrench french
  !insertmacro LangSection "German" SecGerman german
  !insertmacro LangSection "Hungarian" SecHungarian hungarian
  !insertmacro LangSection "Italian" SecItalian italian
  !insertmacro LangSection "Japanese" SecJapanese japanese
  !insertmacro LangSection "Korean" SecKorean korean
  !insertmacro LangSection "Polish" SecPolish polish
  !insertmacro LangSection "Portuguese" SecPortuguese portuguese
  !insertmacro LangSection "Romanian" SecRomanian romanian
  !insertmacro LangSection "Russian" SecRussian russian
  !insertmacro LangSection "Spanish" SecSpanish spanish
  !insertmacro LangSection "Swedish" SecSwedish swedish
  !insertmacro LangSection "Thai" SecThai thai
  !insertmacro LangSection "Turkish" SecTurkish turkish

SubSectionEnd

SubSection "Additional File Format Support" SecFileFormat

  Section "OGG" SecFileFormatOGG
    SectionIn 1 2
    SetOutPath "$INSTDIR\Plugins\Album List\Plugins"
    File ogg.afp
    RegDLL "$INSTDIR\Plugins\Album List\Plugins\ogg.afp" Install
  SectionEnd
  
  Section "FLAC" SecFileFormatFLAC
    SectionIn 1 2
    SetOutPath "$INSTDIR\Plugins\Album List\Plugins"
    File flac.afp
    RegDLL "$INSTDIR\Plugins\Album List\Plugins\flac.afp" Install
  SectionEnd

SubSectionEnd
  
Section "Control Executables" SecControlExe
  SectionIn 1
  SetOutPath "$INSTDIR\Plugins\Album List\Control Executables"
  File "NextAlbum.exe"
  File "PrevAlbum.exe"
  File "PlayAlbum.exe"
  File "PlayAlbumName.exe"
  File "PlayRandomAlbum.exe"
SectionEnd

Section "ATI Remote Wonder Support" SecRemoteWonder
  SetOutPath "$INSTDIR\Plugins\Album List\ATI RW"
  File albumlist_rw.dll
  WriteRegDWORD HKCU "Software\ATI Technologies\Multimedia\Remote Control\Plug-Ins\Album List for Winamp" "A" 0
  WriteRegDWORD HKCU "Software\ATI Technologies\Multimedia\Remote Control\Plug-Ins\Album List for Winamp" "B" 0
  WriteRegDWORD HKCU "Software\ATI Technologies\Multimedia\Remote Control\Plug-Ins\Album List for Winamp" "C" 0
  WriteRegDWORD HKCU "Software\ATI Technologies\Multimedia\Remote Control\Plug-Ins\Album List for Winamp" "D" 0
  WriteRegDWORD HKCU "Software\ATI Technologies\Multimedia\Remote Control\Plug-Ins\Album List for Winamp" "E" 0
  WriteRegDWORD HKCU "Software\ATI Technologies\Multimedia\Remote Control\Plug-Ins\Album List for Winamp" "F" 0
  WriteRegDWORD HKCU "Software\ATI Technologies\Multimedia\Remote Control\Plug-Ins\Album List for Winamp" "Enabled" 1
  WriteRegStr   HKCU "Software\ATI Technologies\Multimedia\Remote Control\Plug-Ins\Album List for Winamp" "Filename" "$INSTDIR\Plugins\Album List\ATI RW\albumlist_rw.dll"
SectionEnd

Section -post
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Album List" \
                   "Contact" "tfma@hotmail.com"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Album List" \
                   "DisplayIcon" "$INSTDIR\Plugins\uninstall-AL.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Album List" \
                   "DisplayName" "Album List for Winamp v${VER_MAJOR}.${VER_MINOR} (remove only)"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Album List" \
                   "DisplayVersion" "${VER_MAJOR}.${VER_MINOR}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Album List" \
                   "HelpLink" "http://albumlist.sourceforge.net"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Album List" \
                   "Publisher" "Safai Ma"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Album List" \
                   "Readme" "$INSTDIR\Plugins\Album List\readme.htm"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Album List" \
                   "UninstallString" "$INSTDIR\Plugins\uninstall-AL.exe"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Album List" \
                   "VersionMajor" ${VER_MAJOR}
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Album List" \
                   "VersionMinor" ${VER_MINOR}
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Album List" \
                   "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Album List" \
                   "NoRepair" 1

  SetOutPath $INSTDIR
  Delete "$INSTDIR\Plugins\uninstall-AL.exe"
  WriteUninstaller "$INSTDIR\Plugins\uninstall-AL.exe"
SectionEnd

;--------------------------------
;Descriptions

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SecCore}         "The core files required to use Album List"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecLanguages}    "Optional language files"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecFileFormat}   "Provides support for reading file information"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecControlExe}   "These executables allows you to control Album List from the command line"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecRemoteWonder} "Provides support for ATI Remote Wonder (requires restart of ATI Remote Wonder software)"
!insertmacro MUI_FUNCTION_DESCRIPTION_END
 
;--------------------------------
;Uninstaller Section

Section "Uninstall"

  IfFileExists $INSTDIR\gen_m3a.dll skip_confirmation
    MessageBox MB_YESNO "It does not appear that Album List is installed in the directory '$INSTDIR'.$\r$\nContinue anyway (not recommended)" IDYES skip_confirmation
    Abort "Uninstall aborted by user"
  skip_confirmation:

  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Album List"
  DeleteRegKey HKCU "Software\LucidAmp\Album List"
  DeleteRegKey HKCU "Software\ATI Technologies\Multimedia\Remote Control\Plug-Ins\Album List for Winamp"

  ; delete languages
  Delete "$INSTDIR\Album List\Language\sample.al"
  Delete "$INSTDIR\Album List\Language\bulgarian.al"
  Delete "$INSTDIR\Album List\Language\catalan.al"
  Delete "$INSTDIR\Album List\Language\czech.al"
  Delete "$INSTDIR\Album List\Language\danish.al"
  Delete "$INSTDIR\Album List\Language\dutch.al"
  Delete "$INSTDIR\Album List\Language\faroese.al"
  Delete "$INSTDIR\Album List\Language\finnish.al"
  Delete "$INSTDIR\Album List\Language\french.al"
  Delete "$INSTDIR\Album List\Language\german.al"
  Delete "$INSTDIR\Album List\Language\hungarian.al"
  Delete "$INSTDIR\Album List\Language\italian.al"
  Delete "$INSTDIR\Album List\Language\japanese.al"
  Delete "$INSTDIR\Album List\Language\korean.al"
  Delete "$INSTDIR\Album List\Language\polish.al"
  Delete "$INSTDIR\Album List\Language\portuguese.al"
  Delete "$INSTDIR\Album List\Language\romanian.al"
  Delete "$INSTDIR\Album List\Language\russian.al"
  Delete "$INSTDIR\Album List\Language\spanish.al"
  Delete "$INSTDIR\Album List\Language\swedish.al"
  Delete "$INSTDIR\Album List\Language\thai.al"
  Delete "$INSTDIR\Album List\Language\turkish.al"
  Delete "$INSTDIR\Album List\Language\simplified chinese.al"
  Delete "$INSTDIR\Album List\Language\traditional chinese.al"
  RMDir  "$INSTDIR\Album List\Language"

  ; delete plugins
  Delete "$INSTDIR\Album List\Plugins\flac.afp"
  Delete "$INSTDIR\Album List\Plugins\ogg.afp"
  RMDir  "$INSTDIR\Album List\Plugins"

  ; delete control executables
  Delete "$INSTDIR\Album List\Control Executables\PlayAlbum.exe"
  Delete "$INSTDIR\Album List\Control Executables\PlayAlbumName.exe"
  Delete "$INSTDIR\Album List\Control Executables\NextAlbum.exe"
  Delete "$INSTDIR\Album List\Control Executables\PrevAlbum.exe"
  Delete "$INSTDIR\Album List\Control Executables\PlayRandomAlbum.exe"
  RMDir  "$INSTDIR\Album List\Control Executables"

  ; delete templates
  Delete "$INSTDIR\Album List\Templates\export.htt"
  Delete "$INSTDIR\Album List\Templates\cover.htt"
  RMDir  "$INSTDIR\Album List\Templates"

  ; delete remote wonder plugin
  Delete "$INSTDIR\Album List\ATI RW\albumlist_rw.dll"

  ; delete readme and stuff
  Delete "$INSTDIR\Album List\readme.htm"
  Delete "$INSTDIR\Album List\walogo.gif"
  Delete "$INSTDIR\Album List\media.jpg"
  Delete "$INSTDIR\Album List\title.jpg"
  Delete "$INSTDIR\Album List\profile.jpg"
  Delete "$INSTDIR\Album List\history.txt"
  RMDir  "$INSTDIR\Album List"


  ; delete plugin
  Delete "$INSTDIR\Gen_m3a.dll"
  Delete "$INSTDIR\uninstall-AL.exe"

  ; delete settings
  MessageBox MB_YESNO|MB_ICONQUESTION \
             "Do you want to delete Album List settings?$\r$\n(No if you're upgrading)" \
             IDNO SaveSetting

  Delete "$INSTDIR\Gen_m3a.dat"
  Delete "$INSTDIR\Gen_m3a.ini"

  SaveSetting:
 
SectionEnd

;--- Add/Remove system macros: ---
; (You may place them to include file)
Var AR_SecFlags
Var AR_RegFlags
 
!macro InitSection SecName
  ClearErrors
  ReadINIStr $AR_RegFlags "$INSTDIR\Plugins\Gen_m3a.ini" "Install" "${SecName}"
  IfErrors "default_${SecName}"
    IntOp $AR_RegFlags $AR_RegFlags & 0x0001          ;Turn off all other bits
    IntCmp $AR_RegFlags 0 "default_${SecName}"
      SectionGetInstTypes ${${SecName}} $AR_SecFlags  ;Reading default insttype flags
      IntOp $AR_SecFlags $AR_SecFlags | 8             ;add to insttype (bit4)
      SectionSetInstTypes ${${SecName}} $AR_SecFlags
 
 "default_${SecName}:"
!macroend

!macro FinishSection SecName
  SectionGetFlags ${${SecName}} $AR_SecFlags  ;Reading section flags
  IntOp $AR_SecFlags $AR_SecFlags & 0x0001
  IntCmp $AR_SecFlags 1 "leave_${SecName}"
    WriteINIStr "$INSTDIR\Plugins\Gen_m3a.ini" "Install" "${SecName}" 0
    Goto "exit_${SecName}"
 
 "leave_${SecName}:"
    WriteINIStr "$INSTDIR\Plugins\Gen_m3a.ini" "Install" "${SecName}" 1
 
 "exit_${SecName}:"
!macroend

!macro SectionList MacroName
  !insertmacro "${MacroName}" SecCore
  !insertmacro "${MacroName}" SecSample
  !insertmacro "${MacroName}" SecBulgarian
  !insertmacro "${MacroName}" SecCatalan
  !insertmacro "${MacroName}" SecChineseS
  !insertmacro "${MacroName}" SecChineseT
  !insertmacro "${MacroName}" SecCzech
  !insertmacro "${MacroName}" SecDanish
  !insertmacro "${MacroName}" SecDutch
  !insertmacro "${MacroName}" SecFaroese
  !insertmacro "${MacroName}" SecFinnish
  !insertmacro "${MacroName}" SecFrench
  !insertmacro "${MacroName}" SecGerman
  !insertmacro "${MacroName}" SecHungarian
  !insertmacro "${MacroName}" SecItalian
  !insertmacro "${MacroName}" SecJapanese
  !insertmacro "${MacroName}" SecKorean
  !insertmacro "${MacroName}" SecPolish
  !insertmacro "${MacroName}" SecPortuguese
  !insertmacro "${MacroName}" SecRomanian
  !insertmacro "${MacroName}" SecRussian
  !insertmacro "${MacroName}" SecSpanish
  !insertmacro "${MacroName}" SecSwedish
  !insertmacro "${MacroName}" SecThai
  !insertmacro "${MacroName}" SecTurkish
  !insertmacro "${MacroName}" SecFileFormatOGG
  !insertmacro "${MacroName}" SecFileFormatFLAC
  !insertmacro "${MacroName}" SecControlExe
  !insertmacro "${MacroName}" SecRemoteWonder
!macroend

;--------------------------------
;Functions

Function .onInit

  ; - Is Winamp Running? -
  FindWindow $0 "Winamp v1.x"
  IsWindow $0 0 End

  ; - Winamp is Running -
  MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION \
             "Winamp is currently running. Close it?" \
             IDCANCEL \
             TerminateInstall

  ; - Close Winamp -
  SendMessage $0 16 0 0

  ; - Set flag to re-start Winamp
  StrCpy $1 "Restart"  

  Goto End

  ; - User Decided to Abort -
  TerminateInstall:
  MessageBox MB_OK|MB_ICONSTOP "Winamp cannot be running during setup.$\r$\nAborting setup..."
  Abort

  ; - Carry on Install -  
  End:

  ; - Add previous installation settings
  ClearErrors
  ReadINIStr $AR_RegFlags "$INSTDIR\Plugins\Gen_m3a.ini" "Install" "SecCore"
  IfErrors NoPrevInstall
  InstTypeSetText 3 "Previous Installation"
  !insertmacro SectionList "InitSection"
  SetCurInstType 3

  NoPrevInstall:

FunctionEnd

Section -FinishComponents
  ;Removes unselected components and writes component status to registry
  !insertmacro SectionList "FinishSection"
SectionEnd

Function .onVerifyInstDir
  IfFileExists $INSTDIR\Winamp.exe PathGood
  Abort

  PathGood:
FunctionEnd

Function un.onInit

  ; - Is Winamp Running? -
  FindWindow $0 "Winamp v1.x"
  IsWindow $0 0 End

  ; - Winamp is Running -
  MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION \
             "Winamp is currently running. Close it?" \
             IDCANCEL \
             TerminateInstall

  ; - Close Winamp -
  SendMessage $0 16 0 0

  Goto End

  ; - User Decided to Abort -
  TerminateInstall:
  MessageBox MB_OK|MB_ICONSTOP "Winamp cannot be running during uninstall.$\r$\nAborting uninstall..."
  Abort

  ; - Carry on Uninstall -  
  End:
FunctionEnd
