@echo off
setlocal EnableExtensions

if "%~1"=="" (
  echo Usage: patch_esi.bat input.xml
  pause
  exit /b 1
)

python "%~dp0patch_esi.py" --input "%~f1" --config "%~dp0esi_patch_config.yaml"

if errorlevel 1 (
  echo.
  echo ESI patching failed.
  pause
  exit /b 1
)

echo.
echo ESI patching completed successfully.
pause
