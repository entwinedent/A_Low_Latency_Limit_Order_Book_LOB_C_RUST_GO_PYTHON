# Setup Guide for Low-Latency Order Book Engine

## Prerequisites Installation

### Windows Setup

#### 1. Install CMake
Download and install CMake from: https://cmake.org/download/
- Choose the Windows installer
- Add CMake to your PATH during installation

#### 2. Install C++ Compiler

**Option A: Visual Studio (Recommended)**
- Download Visual Studio Community: https://visualstudio.microsoft.com/downloads/
- Install "Desktop development with C++" workload
- Includes MSVC compiler and Windows SDK

**Option B: MinGW-w64**
- Download from: https://www.mingw-w64.org/
- Or use MSYS2: https://www.msys2.org/

#### 3. Install Go
- Download Go 1.22+ from: https://go.dev/dl/
- Install and add Go to your PATH

#### 4. Install Rust
- Download rustup-init.exe from: https://rustup.rs/
- Run the installer
- Restart your terminal

#### 5. Install Python 3.12+
- Download Python 3.12+ from: https://www.python.org/downloads/
- Install and add Python to your PATH
- Verify installation: `python --version`

#### 6. Install Python Packages
```bash
pip install pytest mypy
```

#### 7. Install Package Manager (Optional but Recommended)

**Option A: Conan (Recommended)**
```bash
# Install Conan
pip install conan

# Initialize Conan (first time only)
conan profile detect
```

**Option B: vcpkg (Alternative)**
```bash
# Clone vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat  # Windows
./bootstrap-vcpkg.sh  # Linux/macOS

# Integrate with CMake
.\vcpkg integrate install  # Windows
./vcpkg integrate install  # Linux/macOS
```

## Building the Project

### Build C++ Core with Conan (Recommended)
```bash
# Install dependencies with Conan
conan install . --output-folder=build --build=missing

# Configure and build
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

### Build C++ Core with vcpkg (Alternative)
```bash
# Configure with vcpkg toolchain
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]/scripts/buildsystems/vcpkg.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

### Build C++ Core with FetchContent (Fallback)
```bash
# Use FetchContent (no package manager required)
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

### Run Tests
```bash
ctest --test-dir build --config Release
```

### Run Benchmarks
```bash
.\build\bench\Benchmark_OrderBook.exe
```

### Build Language Bindings

#### Go
```bash
cd bindings/go
go mod download
go build -o engine.so ./engine.go
go test ./...

# Run tests with race detector
go test -race ./...

# Or use the Makefile
make test-race
```

#### Rust
```bash
cd bindings/rust
cargo build --release
cargo test
```

#### Python
```bash
cd bindings/python
pip install -e .
python -m pytest tests/

# Type checking with mypy
mypy lob/ --strict
```

## Troubleshooting

### CMake not found
- Ensure CMake is installed and added to PATH
- Restart your terminal after installation

### Compiler not found
- Ensure Visual Studio or MinGW is installed
- For Visual Studio, run from "Developer Command Prompt for VS"

### Python binding errors
- Ensure the C++ library is built first
- Check that the shared library is in the expected location

## Alternative: Docker Build

If you have Docker installed, you can build in a container:
```bash
docker build -t lob-engine .
docker run --rm lob-engine
```

## Next Steps

Once dependencies are installed:
1. Build the C++ core
2. Run unit tests
3. Run benchmarks
4. Build and test language bindings
5. Run the CLI application

## Updating Dependencies

### Update All Dependencies
```bash
python scripts/update_dependencies.py all
```

### Update Specific Component
```bash
# Update Conan dependencies
python scripts/update_dependencies.py conan

# Update Go dependencies
python scripts/update_dependencies.py go

# Update Rust dependencies
python scripts/update_dependencies.py rust

# Update Python dependencies
python scripts/update_dependencies.py python
```

### Manual Updates

#### Conan
```bash
conan install . --output-folder=build --build=missing
```

#### vcpkg
```bash
vcpkg upgrade
```

#### Go
```bash
cd bindings/go
go get -u ./...
go mod tidy
```

#### Rust
```bash
cd bindings/rust
cargo update
```

#### Python
```bash
pip install --upgrade -r requirements.txt
```
