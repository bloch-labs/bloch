# Installation

Bloch ships as tagged release archives plus source builds. These instructions match the latest docs version (`v1.0.2` shown as `latest` in the switcher).

## Prerequisites
- **Linux/macOS:** recent Bash, `curl`, and a writable install directory on `PATH`.
- **Windows:** PowerShell 5+, execution policy allowing the install script; write access to `%LOCALAPPDATA%`.
- To build from source: CMake, a C++17 compiler, and optionally Ninja.

## Install a tagged release (recommended)

### Linux/macOS
```bash
curl -fsSL https://raw.githubusercontent.com/bloch-labs/bloch/HEAD/scripts/install.sh | bash -s -- v1.0.2
```
Notes:
- Replace `v1.0.2` with another tag if you need an older version.
- Override the install directory with `INSTALL_DIR=/custom/path`.
- Use `bloch --update` later to pick up a newer release.

### Windows (PowerShell)
```powershell
iwr https://raw.githubusercontent.com/bloch-labs/bloch/HEAD/scripts/install.ps1 -UseBasicParsing -OutFile $env:TEMP\\bloch-install.ps1; `
  & $env:TEMP\\bloch-install.ps1 -Version v1.0.2
```
Notes:
- Installs to `%LOCALAPPDATA%\\Programs\\Bloch` and adds it to your user `PATH`.
- Swap `v1.0.2` for another tag if needed.

## Build from source
```bash
git clone https://github.com/bloch-labs/bloch.git
cd bloch
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Run a quick smoke test:
```bash
./build/bin/bloch examples/02_bell_state.bloch
```

## Verify your install
```bash
bloch --version
```

If that works, run the quickstart program in {doc}`quickstart`.

## Troubleshooting
- **Binary not found:** ensure the install directory is on `PATH`; restart the shell after install.
- **Permission errors:** retry with a user-writable install directory (`INSTALL_DIR`) or an elevated shell on Windows if required.
- **CMake build issues:** check compiler version (C++17), clear `build/`, and retry with `-DCMAKE_BUILD_TYPE=Release`.
