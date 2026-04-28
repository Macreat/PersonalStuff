# Guía: Entornos colaborativos para desarrollo en C

Resumen
      
Esta guía explica cómo montar entornos reproducibles y colaborativos para proyectos en C, tomando como referencia las prácticas de virtual environments (venv/conda) en Python. Se describen herramientas análogas (Conan, vcpkg, Docker), flujos de trabajo, integración con editores, y ejemplos prácticos para Windows/WSL/Linux.

Principios clave

- Reproducibilidad: usar contenedores o gestores de dependencias para que todos tengan las mismas versiones.
- Aislamiento: mantener dependencias de proyecto fuera del sistema global.
- Determinismo en compilación: fijar toolchains, flags y perfiles.
- Integración con CI: el entorno local debe poder reproducirse en CI (GitHub Actions, GitLab CI).

Herramientas esenciales

- Compiladores: gcc, clang, MSVC (Windows).
- Build systems: CMake (recomendado), Make, Meson.
- Package managers / env managers (equivalentes a conda/venv):
  - Conan (gestor de paquetes para C/C++ con perfiles por proyecto).
  - vcpkg (gestor de Microsoft, integración con CMake mediante toolchain file).
  - Conda (conda-forge puede proveer toolchains y bibliotecas; útil si ya usas Conda).
- Contenedores: Docker (recomendado para reproducibilidad absoluta).
- Herramientas auxiliares: pkg-config, cmake, ninja, ccache, direnv.

Equivalencias: venv/conda para C

- venv (Python) ≈ perfiles y cachés locales en Conan + directorios de build separados.
- conda (Python) ≈ Conda envs que instalan compilers/libs (conda-forge) — útil cuando el proyecto mezcla Python y C/Fortran.
- Recomendación práctica: usar Conan para dependencias C/C++; usar Conda cuando se necesiten paquetes científicos o toolchains disponibles en conda-forge. Para estabilidad absoluta, usar Docker.

Flujo recomendado (Conan + CMake)

1. Instalar conan (requiere Python):

   - PowerShell / WSL / Linux:
     - python -m pip install --user conan
     - conan --version

2. Crear perfil de conan (detecta toolchain):

   - conan profile new default --detect
   - conan profile update settings.compiler.libcxx=libstdc++11 default  # si aplica

3. Estructura mínima del proyecto:

   - CMakeLists.txt
   - src/
   - include/
   - conanfile.txt (o conanfile.py)

4. Flujo de build (desde la raíz del repositorio):

   mkdir build
   cd build
   conan install .. --build=missing
   cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
   cmake --build . -- -j$(nproc)

   - En Windows+MSVC cambia el generador y opciones acorde.

5. Compartir perfiles: añade `conan/profile/default` al repo o documenta cómo crear el perfil.

vcpkg + CMake (alternativa)

1. Clonar vcpkg y bootstrap:

   git clone https://github.com/microsoft/vcpkg.git
   .\vcpkg\bootstrap-vcpkg.bat  # Windows
   ./vcpkg/bootstrap-vcpkg.sh     # Linux/WSL

2. Integrar con CMake:

   cmake .. -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake

Docker para C (ejemplo básico)

Dockerfile minimal:

```
FROM ubuntu:22.04
RUN apt-get update && apt-get install -y build-essential cmake git python3 python3-pip
RUN pip3 install conan
WORKDIR /workspace
```

Build y run:

- docker build -t cdev:latest .
- docker run --rm -it -v "%CD%":/workspace cdev:latest /bin/bash

Conda como proveedor de toolchains

- Conda puede instalar gcc/clang y muchas bibliotecas (conda-forge).
- Ejemplo:
  - conda create -n cdev -c conda-forge cmake pkg-config ninja gcc_linux-64
  - conda activate cdev
- Útil cuando el proyecto mezcla componentes Python y C (extensiones). No sustituye a Conan/vcpkg para gestión de paquetes C++ moderna.

Integración con editores y depuración

- VSCode: usar extensiones C/C++ (Microsoft), CMake Tools, Remote - WSL, Remote - Containers.
- Configurar clangd o cquery para navegación.
- Debug: gdb (Linux/WSL), lldb (macOS), WinDbg/MSVC (Windows).

CI/CD

- Reproducir el mismo flujo de instalación en CI: ejecutar conan install y cmake, o usar la misma imagen Docker.
- Cache de conan/vcpkg entre runs para acelerar builds.

Buenas prácticas

- Documentar profiles y toolchain files en el repo.
- Evitar instalar dependencias globales en la máquina de desarrollo.
- Mantener scripts reproducibles: scripts/setup.sh o Makefile que ejecuten pasos de bootstrap.
- Versionar los manifests de conan/vcpkg (conan.lock, vcpkg lockfile).

Ejemplo mínimo de README snippet (para el repo)

1. Instala dependencias locales (WSL/Linux):
   - sudo apt install build-essential cmake python3-pip
   - pip3 install --user conan
2. Build:
   - mkdir build && cd build
   - conan install .. --build=missing
   - cmake ..
   - cmake --build .

Conclusión

Para C, el mejor equivalente de venv/conda es combinar un gestor de paquetes C (Conan o vcpkg) con builds fuera del árbol (build/), perfiles de compilador y, cuando se necesite máxima reproducibilidad, contenedores Docker. Conda es útil como proveedor de toolchains cuando ya se usa Conda en el equipo.

Referencias rápidas

- https://conan.io
- https://github.com/microsoft/vcpkg
- https://cmake.org
- https://docs.docker.com

