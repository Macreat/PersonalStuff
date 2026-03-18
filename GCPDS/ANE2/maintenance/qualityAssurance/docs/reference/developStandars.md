
# DEVELOPMENT STANDARDS
## Code Quality & Best Practices

### 1. ESTRUCTURA DE ARCHIVOS
```bash

rf_spectrum_analysis/
├── src/
│ ├── rf_spectrum/
│ │ ├── init.py # Exports públicos
│ │ ├── config.py # Configuración
│ │ ├── logger.py # Logging setup
│ │ ├── data/
│ │ │ ├── init.py
│ │ │ ├── loader.py # CSV loading
│ │ │ └── validator.py # Data validation
│ │ ├── processing/
│ │ │ ├── init.py
│ │ │ ├── noise_floor.py # Noise floor estimation
│ │ │ ├── normalization.py # PSD normalization
│ │ │ └── correlation.py # Correlation analysis
│ │ └── visualization/
│ │ ├── init.py
│ │ ├── spectrum_plots.py # Spectrum visualization
│ │ └── correlation_plots.py # Correlation heatmaps
│ │
├── tests/
│ ├── conftest.py # Pytest fixtures
│ ├── test_data/
│ │ ├── init.py
│ │ └── test_loader.py
│ ├── test_processing/
│ │ ├── test_noise_floor.py
│ │ ├── test_normalization.py
│ │ └── test_correlation.py
│ └── test_integration.py
│
├── notebooks/
│ ├── 01_data_exploration.ipynb # Exploración datos
│ ├── 02_pipeline_demo.ipynb # Demo del pipeline
│ └── 03_results_analysis.ipynb # Análisis resultados
│
├── docs/
│ ├── architecture.md
│ ├── api_reference.md
│ ├── user_guide.md
│ ├── examples/
│ │ ├── basic_usage.py
│ │ └── advanced_analysis.py
│ └── migration/
│
├── data/
│ ├── raw/
│ └── processed/
│
├── results/
│ └── outputs/
│
├── scripts/
│ ├── validate_data.py
│ ├── run_pipeline.py
│ └── generate_reports.py
│
├── config/
│ ├── default.yaml # Config default
│ ├── development.yaml
│ ├── production.yaml
│ └── test.yaml
│
├── .github/
│ └── workflows/
│ ├── tests.yml # CI/CD tests
│ └── release.yml # Release automation
│
├── pyproject.toml
├── requirements.txt
├── requirements-dev.txt
├── setup.py
├── README.md
├── CHANGELOG.md
└── .gitignore
```

### 2. ESTÁNDARES DE CODIGO


#### **2.1 PEP 8 - Formatting**

```python

#  CORRECTO
class SpectrumAnalyzer:
    """Analyzer for RF spectrum data."""
    
    def __init__(self, config: Config) -> None:
        self.config = config
        self.data: Dict[str, pd.DataFrame] = {}
    
    def load_data(self, path: str) -> None:
        """Load CSV data from specified path."""
        pass

#  INCORRECTO
class spectrum_analyzer:
    def __init__(self,config):
        self.cfg=config
        self.d={}
    def load_data(self,p):
        pass
```

**Reglas:** 

- Líneas máx 100 caracteres (89 para docstrings)
- 2 líneas en blanco entre funciones/clases
- Snake_case para variables/funciones
- UPPER_CASE para constantes
- Import ordering: stdlib → third-party → local


#### 2.2 TYPE HINTS (PEP 484)
```python
# ✓ CORRECTO: Todo tipado
from typing import Dict, List, Optional, Tuple
import numpy as np

def estimate_noise_floor(
    pxx_array: np.ndarray,
    method: str = "histogram"
) -> float:
    """Estimate noise floor from PSD array."""
    pass

def process_multiple_nodes(
    nodes_data: Dict[str, pd.DataFrame],
    exclude: Optional[set] = None
) -> Tuple[Dict[str, float], np.ndarray]:
    """Process multiple sensor nodes."""
    pass

#  INCORRECTO: Sin type hints
def estimate_noise_floor(pxx_array, method="histogram"):
    pass
```



#### 2.3 DOCSTRINGS (PEP 257 + Google Style)

```python 
def normalize_psd(
    pxx: np.ndarray,
    noise_floor: float,
    method: str = "zscore"
) -> np.ndarray:
    """Normalize power spectral density by noise floor.
    
    Applies offset correction and z-score normalization to align spectra
    from heterogeneous hardware to a common baseline.
    
    Args:
        pxx: Power spectral density array [shape: (n_freq_bins,)]
        noise_floor: Reference noise floor in dB
        method: Normalization method ('zscore', 'minmax', 'robust')
        
    Returns:
        Normalized PSD array with same shape as input
        
    Raises:
        ValueError: If pxx is empty or noise_floor is invalid
        TypeError: If inputs are not numpy arrays
        
    Example:
        >>> pxx = np.array([-85, -84, -83, -82])
        >>> pxx_norm = normalize_psd(pxx, noise_floor=-85.0)
        >>> pxx_norm.mean()
        0.0     
        
    Note:
        This function performs in-place operations for memory efficiency.
        For high-frequency data (>10k records), consider batching.
        
    See Also:
        estimate_noise_floor: Estimate baseline noise level
        apply_offset_correction: Apply per-node calibration
    """
    # Validación
    if not isinstance(pxx, np.ndarray):
        raise TypeError(f"Expected np.ndarray, got {type(pxx)}")
    if pxx.size == 0:
        raise ValueError("pxx array is empty")
    
    # Implementación
    ...

```

### 3. TESTING REQUIREMENTS


#### 3.1 Unit Test Template


```python 


from ... tests/test_processing/test_normalization.py : 

import pytest
import numpy as np
from src.rf_spectrum.processing import normalize_psd

class TestNormalizePSD:
    """Test suite for normalize_psd function."""
    
    @pytest.fixture
    def sample_pxx(self):
        """Fixture: sample PSD array."""
        return np.array([-85, -84, -83, -82, -81, -80])
    
    def test_zero_mean_output(self, sample_pxx):
        """Output should have zero mean after normalization."""
        result = normalize_psd(sample_pxx, noise_floor=-85.0)
        assert np.abs(np.mean(result)) < 1e-10
    
    def test_unit_variance(self, sample_pxx):
        """Output should have unit variance."""
        result = normalize_psd(sample_pxx, noise_floor=-85.0)
        assert np.abs(np.std(result) - 1.0) < 1e-10
    
    def test_invalid_input_raises_error(self):
        """Should raise TypeError for invalid input."""
        with pytest.raises(TypeError):
            normalize_psd("not_an_array", noise_floor=-85.0)
    
    def test_shape_preserves(self, sample_pxx):
        """Output shape should match input shape."""
        result = normalize_psd(sample_pxx, noise_floor=-85.0)
        assert result.shape == sample_pxx.shape
    
    @pytest.mark.parametrize("method", ["zscore", "minmax", "robust"])
    def test_all_methods_supported(self, sample_pxx, method):
        """All declared methods should work."""
        result = normalize_psx(sample_pxx, noise_floor=-85.0, method=method)
        assert result is not None

 Run with:
 - pytest tests/test_processing/test_normalization.py -v --cov

```


#### 3.2 Integration Test Template

tests/test_integration.py

```python 

class TestEndToEndPipeline:
    """Test complete pipeline from load to results."""
    
    def test_full_analysis_run(self, tmp_path):
        """Test entire pipeline produces valid output."""
        from src.rf_spectrum.pipeline import SpectrumAnalysisPipeline
        
        # Setup
        config = Config.from_yaml("config/test.yaml")
        pipeline = SpectrumAnalysisPipeline(config)
        
        # Execute
        results = pipeline.run(
            data_dir="tests/data/fixtures",
            output_dir=tmp_path
        )
        
        # Validate
        assert len(results['nodes']) == 6
        assert results['corr_matrix'].shape == (6, 6)
        assert np.allclose(np.diag(results['corr_matrix']), 1.0)
        assert (tmp_path / "results.json").exists()

```



### TEST MATRIX

```
┌──────────────────┬──────────┬──────┬─────────┬─────────┐
│ Test Type        │ Location │ Tool │ Coverage│ Trigger │ 
├──────────────────┼──────────┼──────┼─────────┼─────────┤
│ Unit Tests       │  tests/  │pytest│ ≥80%    │ commit  │ 
│ Integration Test │tests/    │Pytest│ ≥70%    │ PR      │ 
│ Data Validation  │scripts/  │custom│ 100%    │ weekly  │ 
│ Performance      │bench/    │custom│ -       │ release │ 
│ Documentation    │docs/     │check │ 100%    │    PR   │  
└──────────────────┴──────────┴──────┴─────────┴─────────┘
```


### UNIT TEST CHECKLIST

- [ ] Cada función pública tiene ≥2 tests (happy path + edge case)
- [ ] Casos de error probados (ValueError, TypeError, etc.)
- [ ] Boundaries testados (empty input, max size, etc.)
- [ ] Fixtures reutilizables en conftest.py
- [ ] Parametrized tests para múltiples inputs
- [ ] Reproducibilidad con seed=42

### INTEGRATION TEST CHECKLIST

- [ ] Pipeline completo (load → process → output)
- [ ] Todas las etapas se ejecutan sin errores
- [ ] Output tiene estructura esperada
- [ ] Datos no se pierden entre etapas
- [ ] Mismo resultado con ejecuciones múltiples

### PERFORMANCE TEST CHECKLIST

- [ ] Full pipeline < 2 segundos (6 nodes, 104 records)
- [ ] Memory usage < 500 MB
- [ ] File I/O no bottleneck
- [ ] Correlation computation optimizado

---
### 4. ERROR HANDLING

```python

# ✓ CORRECTO: Manejo exhaustivo de errores

from src.rf_spectrum.logger import get_logger

logger = get_logger(__name__)

def load_and_validate(filepath: str) -> pd.DataFrame:
    """Load CSV with comprehensive error handling."""
    try:
        if not Path(filepath).exists():
            raise FileNotFoundError(f"File not found: {filepath}")
        
        df = pd.read_csv(filepath, encoding='utf-8-sig')
        
        if df.empty:
            logger.warning(f"Loaded empty DataFrame from {filepath}")
            return df
        
        # Validar estructura
        required_cols = ['timestamp', 'pxx', 'latitude', 'longitude']
        missing = set(required_cols) - set(df.columns)
        if missing:
            raise ValueError(f"Missing columns: {missing}")
        
        logger.info(f"Successfully loaded {len(df)} rows from {filepath}")
        return df
        
    except FileNotFoundError as e:
        logger.error(f"File error: {e}")
        raise
    except pd.errors.ParserError as e:
        logger.error(f"CSV parsing error: {e}")
        raise ValueError(f"Invalid CSV format: {e}") from e
    except Exception as e:
        logger.critical(f"Unexpected error loading {filepath}: {e}")
        raise
```

### 5. LOGGING STANDARDS

_config/logging.yaml_


```bash

version: 1
disable_existing_loggers: false

formatters:
  standard:
    format: '%(asctime)s | %(name)s | %(levelname)-8s | %(message)s'
    datefmt: '%Y-%m-%d %H:%M:%S'
  
  detailed:
    format: '%(asctime)s | %(name)s:%(lineno)d | %(levelname)-8s | %(message)s'

handlers:
  console:
    class: logging.StreamHandler
    formatter: standard
    level: INFO
  
  file:
    class: logging.handlers.RotatingFileHandler
    filename: logs/app.log
    formatter: detailed
    level: DEBUG
    maxBytes: 10485760  # 10MB
    backupCount: 5

loggers:
  src.rf_spectrum:
    level: DEBUG
    handlers: [console, file]
  
  src.rf_spectrum.processing:
    level: DEBUG

root:
  level: INFO
  handlers: [console, file]



Uso:

import logging
from src.rf_spectrum.logger import get_logger

logger = get_logger(__name__)

logger.debug("Detailed diagnostic info")
logger.info("General informational message")
logger.warning("Warning about deprecated usage")
logger.error("Error that doesn't stop execution")
logger.critical("Critical error")

´´´
