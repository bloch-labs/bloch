---
title: float
---

Floating-point value (64-bit) for angles and classical math.

### Literals
- Require trailing `f`: `0.5f`, `3.141592f`, `2f`.

### Usage
```bloch
float theta = 1.570796f;  // \pi/2
rx(q, theta);
float scale = -0.25f;
float shifted = scale + 0.75f;
```

### Operators
Standard arithmetic and comparisons. Mixed `int`/`float` expressions promote to `float`.

### Notes
- Rotation gates (`rx`, `ry`, `rz`) consume radians as `float`.
- There is no implicit cast from `float` to `int`.
