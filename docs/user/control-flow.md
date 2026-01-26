# Control Flow

Bloch sticks to familiar C-style flow. Use `if/else` for branching:
```bloch
if (bitCondition) { ... } else { ... }
```
There is also a statement-form ternary:
```bloch
condition ? echo("yes"); : echo("no");
```

Loops come in `for` and `while` flavours:
```bloch
for (int i = 0; i < 3; i = i + 1) { ... }
while (keepGoing) { ... }
```

Functions return values explicitly:
```bloch
function f() -> int { return 1; }
```

Conditions are evaluated as `bit`/`int` truthiness; any non-zero is true. Non-void functions must return a value. There is no `break` or `continue`, so structure loops accordingly. See `functions-and-modules.md` for organising logic across functions and modules.
