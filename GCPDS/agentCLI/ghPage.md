
● Resumen rápido (pasos y comandos) para publicar tu index.html en GitHub Pages.

  Opción A — gh-pages (recomendado, no toca main)

   1. Desde la raíz del repo:
    - git worktree add /tmp/ghpages gh-pages || git checkout --orphan gh-pages
   2. Copia los archivos estáticos:
    - robocopy docs\doxygen\html /tmp/ghpages /MIR  (PowerShell/Windows)
    - cp -r docs/doxygen/html/* /tmp/ghpages/      (Linux/WSL/mac)
   3. En /tmp/ghpages:
    - git add . && git commit -m "Publish docs" && git push -f origin gh-pages
   4. Volver a tu rama de trabajo:
    - git checkout - && git worktree remove /tmp/ghpages

  Opción B — carpeta /docs en main (mantiene todo en la rama principal)

   1. Copia HTML a docs/:
    - robocopy docs\doxygen\html docs /MIR
   2. git add docs && git commit -m "Publish docs to /docs" && git push origin HEAD

  Activar Pages en GitHub:

   - Repo → Settings → Pages → Source: Branch = gh-pages (o main /docs) → Save.
   - Espera 1–5 min; URL: https://<usuario>.github.io/<repo> (o CNAME si lo configuras).

  Alternativa sin Git: desplegar a Netlify/Vercel (npm i -g netlify-cli; netlify deploy --prod --dir=docs/doxygen/html).

  Comprobaciones si falla:

   - Asegura index.html en la raíz del sitio publicado.
   - Repo público o Pages habilitado para privados.
   - Borra cache del navegador o usa curl -I <url>.