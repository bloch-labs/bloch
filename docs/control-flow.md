# Control Flow

Bloch uses familiar C-style control flow. Conditions must be `boolean` or `bit` (official recommendation is to always use `boolean` for conditional logic).

## Branching
```bloch
if (condition) { ... } else { ... }
```

There is also a statement-form ternary:
```bloch
condition ? echo("yes"); : echo("no");
```

## Loops
```bloch
for (int i = 0; i < 3; i = i + 1) { ... }
while (keepGoing) { ... }
```

Bloch does not currently support `break` or `continue`, so structure loops accordingly.

## Returns
```bloch
function f() -> int { return 1; }
```

Non-void functions must return a value along all paths.
