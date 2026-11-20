# Task 02 – Q2 – Return by Reference and Its Dangers

**Marks:** 4  
**Topic:** Examination Marks Register

## Returning a Reference to an Array Element

```cpp
int& highest(int marks[], int n) {
    // ... find index of maximum ...
    return marks[idx];   // valid – storage still exists
}
```

The caller can both read and write the element:
```cpp
highest(m, n) = highest(m, n) + 2;
```

## Dangling Reference

```cpp
int& bad() {
    int local = 42;
    return local;   // undefined behaviour – local is destroyed
}
```

## Reference vs const Reference vs Copy

| Return type   | Caller can modify? | Cost          | Safety when storage may disappear |
|---------------|--------------------|---------------|-----------------------------------|
| `int&`        | Yes                | O(1)          | Low                               |
| `const int&`  | No                 | O(1)          | Medium                            |
| `int`         | No (copy)          | O(size)       | High                              |

## How to Run

```bash
g++ -std=c++17 -Wall -Wextra -o q2 q2.cpp
./q2
```
