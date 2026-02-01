# Language Tour

This is a fast walk through Bloch syntax with small, runnable snippets.

Start with the entry point:
```bloch
function main() -> void {
    echo("Hello, Bloch!");
}
```

Common types:
```bloch
int i = 1;
long big = 123L;
float f = 3.14f;
boolean ok = true;
bit b = 1b;
string s = "hi";
char c = 'x';
qubit q;
```

Arrays and indexing:
```bloch
int[] xs = {1, 2, 3};
int[4] ys;
xs[0] = 5;
```

Control flow mirrors C-style languages:
```bloch
if (ok) { echo("yes"); } else { echo("no"); }
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
    public constructor(int x, int y) -> Point { this.x = x; this.y = y; return this; }
    public function sum() -> int { return this.x + this.y; }
}
```

Quantum code stays explicit:
```bloch
@quantum
function flip() -> bit {
    qubit q;
    h(q);
    return measure q;
}

function main() -> void {
    @tracked qubit[2] qreg;
    bit[2] out = {measure qreg[0], measure qreg[1]};
    echo(out);
}
```

For deeper coverage, jump to the guides or the [Language Specification](./language-spec.md).
