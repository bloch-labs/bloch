---
title: float
---

Floating-point value (64-bit). Used for rotation angles and classical math that requires fractions.

### Literals
- Require a trailing `f`: `0.0f`, `3.14f`, `2f`.
- Scientific notation is not available in 1.0.x.

### Usage
```bloch
float theta = 1.570796f;   // \pi/2
rx(q, theta);
float amp = -0.25f;
float scaled = amp * 2.0f;
```

### Operators
Arithmetic and comparisons follow standard floating semantics. Mixed `int`/`float` expressions promote to `float`.

### Notes
- Rotation gates (`rx`, `ry`, `rz`) require `float` radians.
- There is no implicit cast from `float` to `int`; assign explicitly via intermediate logic if needed.
