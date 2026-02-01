# Contributing

Thanks for your interest in contributing to Bloch! Please check the issues tab for open work
or file a new issue if you spot a bug or have an idea.

## Start here
- `engineering/reference/how-to-contribute.md`
- `engineering/reference/codebase-overview.md`

## Building from source

### Linux/macOS
```bash
git clone https://github.com/bloch-labs/bloch.git
cd bloch
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Run the compiler against an example with `./build/bin/bloch examples/02_bell_state.bloch`.

### Windows (Visual Studio)
```powershell
git clone https://github.com/bloch-labs/bloch.git
cd bloch
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
ctest --test-dir build --config Release --output-on-failure
```

This produces a `build/Bloch.sln` you can open directly in Visual Studio for editing and
debugging. The `bloch.exe` binary is placed in `build/bin/Release/`.
