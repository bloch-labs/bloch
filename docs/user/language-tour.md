# Language Tour

This is a fast walk through Bloch syntax with small, runnable snippets.

Start with the entry point:
```bloch
function main() -> void {
    echo("Hello, Bloch!");
}
```

Declare common types in one place:
```bloch
int i = 1;
float f = 3.14f;
string s = "hi";
bit b = 1b;
qubit q;
```

Arrays and indexing look familiar:
```bloch
int[] xs = {1, 2, 3};
int[4] ys;
xs[0] = 5;
```

Control flow mirrors C-style languages:
```bloch
if (b) { echo("yes"); } else { echo("no"); }
for (int i = 0; i < 3; i = i + 1) { echo(i); }
while (i < 3) { i = i + 1; }
```

Functions are typed and explicit:
```bloch
function add(int a, int b) -> int { return a + b; }
```

Imports pull in other modules:
```bloch
import mylib.Utils;
```

Classes combine fields, methods, and constructors:
```bloch
class Point {
    public int x;
    public int y;
    public constructor(int x, int y) -> Point { this.x = x; this.y = y; }
    public function sum() -> int { return this.x + this.y; }
}
```

Quantum code stays explicit:
```bloch
@quantum function flip() -> bit {
    qubit q;
    h(q);
    return measure q;
}
@tracked qubit[2] qreg;
bit[2] out = {measure qreg[0], measure qreg[1]};
```

When you want more detail on types, control flow, classes, or quantum features, jump to the dedicated guides in this folder.
