# REGENERAR la documentación Doxygen (index.html)

Este archivo explica cómo volver a ejecutar Doxygen para regenerar el índice HTML (index.html) de la documentación del proyecto.

Rápido — Windows (PowerShell):

1. Abre PowerShell en la carpeta del proyecto:
   D:\wnOs\wsp\CODE\work\PersonalStuff\GCPDS\agentCLI

2. Ejecuta el script incluido:
   .\\generate_docs.ps1

   - El script crea la carpeta de salida docs\doxygen si no existe y ejecuta `doxygen Doxyfile`.
   - Requiere: Doxygen instalado y disponible en PATH.

Rápido — Linux / WSL / macOS:

1. Abre una terminal en la raíz del proyecto (la misma ruta que arriba).
2. Asegúrate de tener doxygen instalado (ej. `sudo apt install doxygen`, `brew install doxygen`).
3. Ejecuta:
   doxygen Doxyfile

Notas importantes y solución de problemas:

- Instalación de Doxygen:
  - Windows: choco install doxygen  (Chocolatey) o scoop install doxygen
  - macOS: brew install doxygen
  - Ubuntu/WSL: sudo apt update && sudo apt install -y doxygen

- Si Doxygen da warnings sobre referencias internas (\ref) o etiquetas HTML no soportadas, la generación HTML suele completarse, pero revisa los archivos .md por enlaces rotos o tags especiales (ej. `@github`, `<feature>`). Corrige o elimina esos marcadores para limpiar la salida.

- Si la salida HTML no aparece en la ruta esperada, verifica `OUTPUT_DIRECTORY` en el Doxyfile. Por defecto aquí:
  D:/wnOs/wsp/CODE/work/PersonalStuff/GCPDS/agentCLI/docs/doxygen/html

- Para regenerar después de editar README.md u otros .md: guarda los cambios, ejecuta el script (o `doxygen Doxyfile`) y luego abre `docs/doxygen/html/index.html` en el navegador.

Commit y publicar los cambios en git (opcional):

1. git add docs/doxygen/html -A
2. git add Doxyfile generate_docs.ps1 README.md C-Collab-Env-Guide.md
3. git commit -m "docs: regenerate Doxygen HTML" -m "Co-authored-by: Copilot <223556219+Copilot@users.noreply.github.com>"
4. git push origin <branch>

Consejos:
- Mantén Doxyfile relativo o con rutas absolutas coherentes con tu entorno para evitar problemas al ejecutar desde CI o desde otra carpeta.
- Para CI: instala doxygen en la imagen runner o usa un contenedor Docker que ya incluya doxygen y ejecuta `doxygen Doxyfile` en la job.

Si quieres, puedo añadir un job de GitHub Actions que haga la generación automática y publique los artefactos o los despliegue a GitHub Pages.
