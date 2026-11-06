@echo off
setlocal
rem Override these two variables when using a different matching Qt/MinGW kit.
if not defined QT_APP_BASE_QT_ROOT set "QT_APP_BASE_QT_ROOT=F:\Apps\Qt\6.9.3\mingw_64"
if not defined QT_APP_BASE_MINGW_BIN set "QT_APP_BASE_MINGW_BIN=F:\Apps\Qt\Tools\mingw1310_64\bin"

if not exist "%~dp0build\qt_app_base_demo.exe" (
    echo Demo executable not found. Follow the build instructions in README.md.
    goto :failed
)
if not exist "%QT_APP_BASE_QT_ROOT%\bin\Qt6Widgets.dll" (
    echo Qt runtime not found. Set QT_APP_BASE_QT_ROOT to the matching Qt kit directory.
    goto :failed
)
if not exist "%QT_APP_BASE_QT_ROOT%\plugins\platforms\qwindows.dll" (
    echo Qt Windows platform plugin not found. Check QT_APP_BASE_QT_ROOT.
    goto :failed
)
if not exist "%QT_APP_BASE_MINGW_BIN%\libstdc++-6.dll" (
    echo MinGW runtime not found. Set QT_APP_BASE_MINGW_BIN to the matching compiler bin directory.
    goto :failed
)

set "PATH=%QT_APP_BASE_MINGW_BIN%;%QT_APP_BASE_QT_ROOT%\bin;%PATH%"
set "QT_QPA_PLATFORM_PLUGIN_PATH=%QT_APP_BASE_QT_ROOT%\plugins\platforms"
set "QT_QPA_PLATFORM=windows"
start "" /D "%~dp0" "%~dp0build\qt_app_base_demo.exe" %*
if errorlevel 1 goto :failed
exit /b 0

:failed
echo.
pause
exit /b 1
