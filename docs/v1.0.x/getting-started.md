---
title: Getting Started
---

Install Bloch 1.0.x, verify the CLI, and run a first program.

## Install a release build
Requires macOS or Linux with curl, or Windows PowerShell 5.1+.

### Linux & macOS
```bash
curl -fsSL https://raw.githubusercontent.com/bloch-labs/bloch/HEAD/scripts/install.sh | bash -s -- v1.0.2
```
Swap `v1.0.2` for another 1.0.x tag if needed. Set `INSTALL_DIR=/custom/path` before `bash` to override the install location.

### Windows
```powershell
powershell -NoProfile -ExecutionPolicy Bypass -Command "iwr https://raw.githubusercontent.com/bloch-labs/bloch/HEAD/scripts/install.ps1 -UseBasicParsing -OutFile $env:TEMP\bloch-install.ps1; & $env:TEMP\bloch-install.ps1 -Version v1.0.2"
```
Installs to `%LOCALAPPDATA%\Programs\Bloch` by default and adds the directory to your user `PATH`.

### Verify the install
```bash
bloch --version
```
Then run an example:
```bash
bloch examples/01_hadamard.bloch --shots=512
```

## Hello World (single-qubit Hadamard)
```bloch
function main() -> void {
    @tracked qubit q;
    h(q);
    measure q;
}
```
Save as `hello.bloch`, then run:
```bash
bloch hello.bloch --shots=256
```
You should see roughly equal counts for `0` and `1` in the tracked summary.

## Bell pair (two qubits)

```bloch
@quantum
function createBellState(qubit a, qubit b) -> void {
    h(a);
    cx(a, b);
}

function main() -> void {
    @tracked qubit[2] qreg;

    createBellState(qreg[0], qreg[1]);

    bit[2] out = {measure qreg[0], measure qreg[1]};

    echo(out);
}
```

Save this to `bell_state.bloch`, then run:

```bash
bloch bell_state.bloch
```
Helpful flags:
- `--shots=N` to repeat and aggregate tracked outcomes.
- `--emit-qasm` to print the generated OpenQASM.
- `--echo=all|none` to control `echo(...)` output (suppressed automatically when `--shots` is large).

## Troubleshooting
- `bloch: command not found` — ensure the install directory is on `PATH` (rerun installer or export `PATH`).
- Permission errors running the installer — `chmod +x` the script or invoke via `bash -s --`.
- PowerShell execution policy issues — run PowerShell as Administrator to allow the installer, then revert policy if desired.
- Build failures — confirm CMake ≥ 3.16, a C++20 compiler, and clean the `build/` directory before reconfiguring.

## Upgrade / uninstall
- Upgrade to the latest 1.0.x: rerun the installer with `v1.0.2` (or the desired tag) or run `bloch --update`.
- Uninstall: remove the installed binary directory (`/usr/local/bin/bloch` or `%LOCALAPPDATA%\Programs\Bloch`) and delete any PATH edits the script added to your shell profile.
