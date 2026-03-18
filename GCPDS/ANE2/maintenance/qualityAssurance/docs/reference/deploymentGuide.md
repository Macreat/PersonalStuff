# DEPLOYMENT & RELEASE MANAGEMENT

### VERSION SCHEME

```

v[MAJOR].[MINOR].[PATCH]
v0.1.0 = Initial release
v0.2.0 = New feature (backward compatible)
v0.2.1 = Bug fix
v1.0.0 = Production ready


```

### RELEASE CHECKLIST

- [ ] Todos tests passing
- [ ] Code coverage ≥80%
- [ ] Documentación actualizada
- [ ] CHANGELOG.md updated
- [ ] Version bumped en `setup.py`
- [ ] Git tags creado
- [ ] Release notes en GitHub/GitLab
- [ ] Archive histórico guardado

### DEPLOYMENT STEPS

```bash
# 1. Prepare release
git checkout main
git pull origin main
bumpversion patch  # or minor/major

# 2. Test
pytest tests/ --cov=src --cov-fail-under=80

# 3. Build
python setup.py sdist bdist_wheel

# 4. Create tag
git tag v$(python setup.py --version)
git push origin v$(python setup.py --version)

# 5. Create release notes
echo "Release v$(python setup.py --version)" > RELEASE.md

# 6. Archive
aws s3 cp dist/ s3://rf-spectrum-releases/v$(python setup.py --version)/

```

# QUICK REFERENCE - CONTROL VERSION 

##  PARA EMPEZAR UN NUEVO FEATURE

```bash
# 1. Actualizar main
git checkout main && git pull

# 2. Crear rama
git checkout -b feature/nombre-descriptivo

# 3. Hacer cambios, commit
git add .
git commit -m "feat(module): descripción clara"

# 4. Subir
git push origin feature/nombre-descriptivo

# 5. Crear Pull Request
# → Description
# → Checklist
# → Request review

```

## BEFORE COMMIT 

```bash

# Ejecutar tests
pytest tests/ -v

# Coverage check
pytest --cov=src --cov-report=html

# Linting
flake8 src/
black --check src/

# Type checking
mypy src/

```

## ESTÁNDARES RÁPIDOS


- Aspecto	Estándar
- Line length	100 chars
- Imports	stdlib → 3rd party → local
- Docstrings	Google format
- Type hints	Siempre
- Test coverage	≥80%
- Commits	Conventional




## merge & release 

```bash

# Merge feature (asumiendo PR approved)
git checkout main
git merge --squash feature/nombre
git push origin main

# Tag release
git tag -a v0.2.0 -m "Release v0.2.0"
git push origin v0.2.0

# Cleanup
git branch -d feature/nombre
```

## checklist

- [] Ejecutar full test suite
- [] Revisar datos (validate_data.py ejecutado)
- [x] Documentación actualizada
- [] Performance benchmarks OK
- [] Sync con equipo (meeting)



