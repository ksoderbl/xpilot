@echo off
REM -- First make map file from Microsoft Visual C++ generated resource.h
echo // MAKEHELP.BAT generated Help Map file.  Used by XPILOT.HPJ. >"hlp\xpilot.hm"
echo. >>"hlp\xpilot.hm"
echo // Commands (ID_* and IDM_*) >>"hlp\xpilot.hm"
makehm ID_,HID_,0x10000 IDM_,HIDM_,0x10000 resource.h >>"hlp\xpilot.hm"
echo. >>"hlp\xpilot.hm"
echo // Prompts (IDP_*) >>"hlp\xpilot.hm"
makehm IDP_,HIDP_,0x30000 resource.h >>"hlp\xpilot.hm"
echo. >>"hlp\xpilot.hm"
echo // Resources (IDR_*) >>"hlp\xpilot.hm"
makehm IDR_,HIDR_,0x20000 resource.h >>"hlp\xpilot.hm"
echo. >>"hlp\xpilot.hm"
echo // Dialogs (IDD_*) >>"hlp\xpilot.hm"
makehm IDD_,HIDD_,0x20000 resource.h >>"hlp\xpilot.hm"
echo. >>"hlp\xpilot.hm"
echo // Frame Controls (IDW_*) >>"hlp\xpilot.hm"
makehm IDW_,HIDW_,0x50000 resource.h >>"hlp\xpilot.hm"
REM -- Make help for Project XPILOT


echo Building Win32 Help files
start /wait hcrtf -x "hlp\xpilot.hpj"
echo.
if exist Debug\nul copy "hlp\xpilot.hlp" Debug
if exist Debug\nul copy "hlp\xpilot.cnt" Debug
if exist Release\nul copy "hlp\xpilot.hlp" Release
if exist Release\nul copy "hlp\xpilot.cnt" Release
echo.


