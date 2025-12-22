# Quickstart (5 minutes)

Walk through installing Bloch and running your first circuit.

## 1) Install Bloch
- **Linux/macOS:**
  ```bash
  curl -fsSL https://raw.githubusercontent.com/bloch-labs/bloch/HEAD/scripts/install.sh | bash -s -- v1.0.2
  ```
- **Windows (PowerShell):**
  ```powershell
  iwr https://raw.githubusercontent.com/bloch-labs/bloch/HEAD/scripts/install.ps1 -UseBasicParsing -OutFile $env:TEMP\\bloch-install.ps1; `
    & $env:TEMP\\bloch-install.ps1 -Version v1.0.2
  ```
These scripts install the tagged release and place `bloch` on your `PATH`. Swap `v1.0.2` for another tag if needed.

## 2) Verify the CLI
```bash
bloch --version
```

## 3) Create and run a minimal program
Create `hello.bloch`:

```bloch
@quantum
function applyHadamard(qubit q) -> void {
    h(q);
}

function main() -> void {
    qubit q0;
    applyHadamard(q0);
    bit result = measure q0;
    echo(result);
}
```

Run it:

```bash
bloch hello.bloch
```

You should see roughly even distribution of `0` and `1` across runs when executed repeatedly.

## 4) Try a built-in example

Run the Bell pair example from the repo:

```bash
bloch examples/02_bell_state.bloch
```

This prepares an entangled state and measures correlated outcomes (`00` / `11`). Explore more examples in {doc}`examples/index`.
