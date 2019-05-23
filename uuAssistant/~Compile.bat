@ECHO OFF
set LIB_ZLIB=1
set LIB_PNG=1
set LIB_JPEG=1
set LIB_RAW=1
set LIB_TIFF=1
set LIB_CXIMAGE=1

set PROJ=uuAssistant
set RC=main.rc
set CPP=MainForm.cpp NoteForm.cpp MiniIEForm.cpp ..\ccSample\DesktopCalendar.cpp

set OTHERSPARAM= /D _CC_ASSISTANT
rem #define _WIN32_WINNT 0x0500, 0x0501 for Windows XP, 0x0502 for Windows Server 2003, 0x0600 for Windows Vista, and 0x0601 for Windows 7.
set WINVER=0x0501

set PARAM0=%0%
set PARAM1=%1%
set PARAM2=%2%
set PARAM3=%3%
set PARAM4=%4%
IF NOT '%PARAM1%'=='' (
  FOR /F "usebackq delims=~" %%i IN (`@ECHO %0`) DO %%~di
  FOR /F "usebackq delims=~" %%i IN (`@ECHO %0`) DO CD %%~i
)
CALL ..\cc\~CompileBase.bat
