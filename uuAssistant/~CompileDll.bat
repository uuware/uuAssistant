@ECHO OFF
@set PROJ=uuAssistant
set CPP=..\uuSpy\uuSpyDll.cpp
set DLLDEF=..\uuSpy\uuSpyDll.def

set OTHERSPARAM= /D _NOCCLOG /D _CCTHINFORM /D _UUASSISTANT

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
