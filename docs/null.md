## Null references

Bloch v1.1.x includes an explicit `null` literal to represent the absence of an object.

- Only class references may be `null`. Primitives (`int`, `float`, `bit`, `char`, `string`, `qubit`) and arrays are non-nullable.
- `null` can be assigned to and compared with class references using `==`/`!=`. It cannot be used with other operators or implicitly cast.
- Member access or method calls on `null` raise a runtime error: `"null reference"`.
- `destroy null` is a no-op to keep defensive cleanup safe.

### Example

```bloch
class Node {
    public Node next;
    public constructor() -> Node {
        this.next = null;  // initialise to “no next”
        return this;
    }
}

function main() -> void {
    Node head = new Node();
    if (head.next == null) {
        echo("list is empty");
    }
    head.next = new Node();
    if (head.next != null) {
        echo("list now has a next");
    }
}
```

See `examples/null_demo.bloch` for a runnable sample.
