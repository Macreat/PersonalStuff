@echo off
REM --- Configuracion forzada para MSYS2 ---
SET CMAKE_EXE=C:\msys64\mingw64\bin\cmake.exe
SET MAKE_EXE=C:\msys64\usr\bin\make.exe
SET CC=C:\msys64\mingw64\bin\gcc.exe
SET CXX=C:\msys64\mingw64\bin\g++.exe

SET WORK_DIR=%~dp0
SET BUILD_DIR=D:\bld\liquidSDR

echo [INFO] Iniciando build...

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
cd /d "%BUILD_DIR%"

echo [INFO] Configurando CMake...
REM Usamos el operador call para asegurar que los argumentos con comillas se pasen intactos
call "%CMAKE_EXE%" "%WORK_DIR%" -G "MSYS Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_MAKE_PROGRAM="%MAKE_EXE%" -DCMAKE_C_COMPILER="%CC%" -DCMAKE_CXX_COMPILER="%CXX%"
if %errorlevel% neq 0 goto :error

echo [INFO] Compilando...
"%MAKE_EXE%" -j4
if %errorlevel% neq 0 goto :error

echo [SUCCESS] Build finalizado.
pause
exit /b 0

:error
echo [ERROR] Fallo en la compilacion.
pause
exit /b 1
