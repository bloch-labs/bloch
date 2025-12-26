---
title: Getting Started
---

Install Bloch 1.1.x, verify your setup, and run your first hybrid program. This guide mirrors enterprise install guides (Linux/macOS/Windows), includes troubleshooting, and ends with a Hello World.

## Install a release build
Requires macOS or Linux with curl, or Windows PowerShell 5.1+.

### Linux & macOS
```bash
curl -fsSL https://raw.githubusercontent.com/bloch-labs/bloch/HEAD/scripts/install.sh | bash -s -- latest
```
Swap `latest` for a specific tag (for example, `v1.1.0`). The script downloads the archive, verifies checksums, installs `bloch`, and updates your shell profile. Set `INSTALL_DIR=/custom/path` before `bash` to override the destination.

### Windows
```powershell
powershell -NoProfile -ExecutionPolicy Bypass -Command "iwr https://raw.githubusercontent.com/bloch-labs/bloch/HEAD/scripts/install.ps1 -UseBasicParsing -OutFile $env:TEMP\bloch-install.ps1; & $env:TEMP\bloch-install.ps1 -Version latest"
```
Swap `-Version latest` for the tag you want. Installs to `%LOCALAPPDATA%\Programs\Bloch` by default and adds that directory to your user `PATH`.

### Verify the install
```bash
bloch --version
```
You should see `v1.1.x` plus a commit hash. Validate the runtime with a Bell state:
```bash
bloch examples/02_bell_state.bloch --shots=512
```
Expect roughly even counts for `00` and `11`.

## Hello World (single-qubit Hadamard)
```bloch
function main() -> void {
    @tracked qubit q;
    h(q);
    bit b = measure q;
    echo(b);
}
```
Save as `hello.bloch`, then run:
```bash
bloch hello.bloch --shots=256
```
You should see roughly equal counts for `0` and `1` in the tracked summary.

## Bell state (two qubits)
```bloch
function main() -> void {
    @tracked qubit[2] qreg;
    h(qreg[0]);           // Create superposition
    cx(qreg[0], qreg[1]); // Entangle
    bit[2] out = {measure qreg[0], measure qreg[1]};
    echo(out);
}
```
Save as `bell_state.bloch`, then run:
```bash
bloch bell_state.bloch
```
Useful flags:
- `--shots=N` to repeat and aggregate tracked outcomes.
- `--emit-qasm` to print the generated OpenQASM (also writes `<file>.qasm`).
- `--echo=all|none` to control `echo(...)` output (auto-suppressed when `--shots` is large).

## Build from source (for contributors or custom builds)

### Linux & macOS
```bash
git clone https://github.com/bloch-labs/bloch.git
cd bloch
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```
Need debug symbols? Switch `-DCMAKE_BUILD_TYPE=Debug`.

### Windows (Visual Studio)
```powershell
git clone https://github.com/bloch-labs/bloch.git
cd bloch
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
ctest --test-dir build --config Release --output-on-failure
```
This produces `build/Bloch.sln` and places `bloch.exe` under `build/bin/Release/`.

## Troubleshooting
- `bloch: command not found` — ensure install dir is on `PATH` (rerun installer or export `PATH` manually).
- `Permission denied` on install script — run `chmod +x` on the downloaded script or use `bash -s --`.
- Windows PowerShell execution policy errors — start PowerShell as Administrator and allow the script to run, then revert policy if desired.
- Build failures: confirm CMake ≥ 3.16, compiler supports C++20, and clean the `build/` directory before reconfiguring.

## Upgrade / uninstall
- Upgrade to the latest: rerun the installer with `latest` or run `bloch --update`.
- Switch versions: rerun the installer with the desired tag (for example, `v1.1.0`).
- Uninstall: remove the installed binary directory (`/usr/local/bin/bloch` or `%LOCALAPPDATA%\Programs\Bloch`) and delete any PATH edits the script added to your shell profile.
