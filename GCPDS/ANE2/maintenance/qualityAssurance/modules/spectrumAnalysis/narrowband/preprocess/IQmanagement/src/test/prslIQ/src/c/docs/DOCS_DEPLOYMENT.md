# Despliegue de documentacion HTML (Doxygen)

## Objetivo
Generar una documentacion HTML basica y profesional del proyecto C usando los comentarios Doxygen ya integrados en el codigo.

## Salida esperada
- Carpeta de salida: `docs/_build/html`
- Entrada principal: `docs/_build/html/index.html`

## Opcion 1: Generacion directa
Desde la raiz de `src/c`:

```powershell
.\generate_docs.ps1
```

## Opcion 2: Instalar herramientas y generar
Si no tienes `doxygen` o `graphviz`:

```powershell
.\generate_docs.ps1 -InstallTools
```

## Opcion 3: Generar y abrir automaticamente en navegador

```powershell
.\generate_docs.ps1 -Open
```

## Notas de calidad (estado del arte)
- Incluye buscador, arbol lateral y navegacion por archivos/funciones.
- Incluye relacion de referencias y llamados entre funciones (si `dot` esta disponible).
- Incluye fuente navegable con enlaces cruzados.
- Toma como entrada:
  - `include/*.h`
  - `src/*.c`
  - `docs/SOURCE_REFERENCE.md`
  - `README.md`

## Buenas practicas para CI/CD
- Ejecutar `generate_docs.ps1` en cada release.
- Publicar `docs/_build/html` como artefacto.
- Versionar la configuracion `docs/Doxyfile`.
