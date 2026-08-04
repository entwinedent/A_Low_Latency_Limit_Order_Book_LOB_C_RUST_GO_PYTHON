# Go Bindings

This folder contains the Go bindings for the order book engine.
It includes the wrapper code and tests used to expose the core functionality to
Go applications and to validate integration behavior.

## Requirements

- Go 1.22 or higher
- C++ compiler (for building the core library)
- CMake 3.20 or higher

## Building

### Using Makefile (Recommended)
```bash
make build
```

### Using Go directly
```bash
go build -o engine.so ./engine.go
```

## Testing

### Standard Tests
```bash
make test
# or
go test ./...
```

### Race Detector Tests
```bash
make test-race
# or
go test -race ./...
```

### Race Detector with Coverage
```bash
make test-race-coverage
```

### All Tests
```bash
make test-all
```

## Linting

### Run Linter
```bash
make lint
```

### Fix Linting Issues
```bash
make lint-fix
```

## Development Tools

### Install Tools
```bash
make install-tools
```

### Format Code
```bash
make fmt
```

### Run All Checks
```bash
make check
# or with race detector
make check-race
```

## Race Detector

The Go bindings include race detector integration to catch concurrency issues:

- **Standard Race Tests**: `make test-race`
- **Race with Coverage**: `make test-race-coverage`
- **Race with Benchmarks**: `make test-race-bench`

### Race Detector Notes

- Race detector tests may run slower than standard tests
- Run race detector tests in CI/CD pipeline
- Fix any race conditions before committing
- Use `-race` flag during development

## CI/CD

The project includes race detector tests in the CI/CD pipeline:
- Standard tests run on every commit
- Race detector tests run on every commit
- Separate race detector job to avoid slowing down standard tests

## Makefile Targets

- `all` - Build the application
- `build` - Build the application
- `test` - Run tests without race detector
- `test-race` - Run tests with race detector
- `test-race-coverage` - Run tests with race detector and coverage
- `test-race-bench` - Run tests with race detector and benchmarks
- `test-all` - Run all tests
- `deps` - Download dependencies
- `clean` - Clean build artifacts
- `fmt` - Format code
- `lint` - Run linter
- `lint-fix` - Fix linting issues
- `check` - Run all checks
- `check-race` - Run all checks with race detector
- `install-tools` - Install development tools
- `help` - Show help message
