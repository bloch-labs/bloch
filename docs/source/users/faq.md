# FAQ & Troubleshooting

Common issues and quick fixes.

## Install script can’t write to the install directory
Set `INSTALL_DIR` to a user-writable path before running the script:
```bash
INSTALL_DIR=$HOME/.local/bin curl -fsSL https://raw.githubusercontent.com/bloch-labs/bloch/HEAD/scripts/install.sh | bash -s -- v1.0.2
```
Ensure that directory is on your `PATH`.

## `bloch` command not found after install
Open a new shell so your updated `PATH` is picked up. On Windows, log out/in or start a new PowerShell session.

## CMake configure/build failures
- Confirm a C++17 compiler is available.
- Clear the build directory and retry:
  ```bash
  rm -rf build
  cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
  cmake --build build --parallel
  ```

## Reporting bugs
- Include platform, Bloch version (`bloch --version`), the command you ran, and a minimal program or example that reproduces the issue.
- File issues on GitHub with logs and expected vs. actual behavior.
