# API Documentation

This directory contains API documentation for the Low-Latency Order Book Engine across all supported languages.

## Languages

### C++ API
- **Location**: `../../core/include/lob/`
- **Documentation**: Inline Doxygen comments
- **Generated Docs**: To be generated from source

### Go API
- **Location**: `../../bindings/go/`
- **Documentation**: Go doc comments
- **Generated Docs**: `godoc` or `pkgsite`

### Rust API
- **Location**: `../../bindings/rust/`
- **Documentation**: Rust doc comments
- **Generated Docs**: `cargo doc`

### Python API
- **Location**: `../../bindings/python/`
- **Documentation**: Python docstrings
- **Generated Docs**: Sphinx with autodoc

## API Categories

### Core Operations
- Order management (add, cancel, modify)
- Market data queries (best bid/ask, depth)
- Trade callbacks and events

### Advanced Features
- Risk management checks
- Multi-symbol operations
- Metrics and monitoring

### Order Types
- Basic orders (limit, market)
- Advanced orders (stop-loss, take-profit, iceberg, etc.)

## Generating Documentation

### C++ Documentation
```bash
# Install Doxygen
# Generate documentation
doxygen Doxyfile
```

### Go Documentation
```bash
# Generate and view documentation
cd bindings/go
godoc -http=:6060
```

### Rust Documentation
```bash
# Generate and view documentation
cd bindings/rust
cargo doc --open
```

### Python Documentation
```bash
# Generate documentation
cd bindings/python
sphinx-build -b html docs/ docs/_build/
```

## API Examples

See the main README.md for usage examples in each language.