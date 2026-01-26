# Getting Started

If you want to try Bloch quickly, grab a release script or build from source, then run one of the included examples.

On Linux/macOS, install with:
```
curl -fsSL https://raw.githubusercontent.com/bloch-labs/bloch/HEAD/scripts/install.sh | bash -s -- latest
```
Replace `latest` with a tag like `v1.0.0` if you need a specific version. On Windows, use PowerShell:
```
powershell -NoProfile -ExecutionPolicy Bypass -Command "iwr https://raw.githubusercontent.com/bloch-labs/bloch/HEAD/scripts/install.ps1 -UseBasicParsing -OutFile $env:TEMP\\bloch-install.ps1; & $env:TEMP\\bloch-install.ps1 -Version latest"
```

Prefer source builds? Clone and build:
```
git clone https://github.com/bloch-labs/bloch.git
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

Run your first program:
```
./build/bin/bloch examples/02_bell_state.bloch --emit-qasm
```

Check the install with `bloch --version`, then move on to the language tour once you see output.
