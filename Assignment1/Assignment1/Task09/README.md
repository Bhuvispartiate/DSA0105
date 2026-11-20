# Task 09 – Implement and Test the Fare Logic

**Marks:** 4

## Objective
Implement the fare calculation and minimum-balance check, then verify them against a test table.

## Test Cases Covered

| # | Zone   | Peak | Balance | Expected Fare | Expected Result      |
|---|--------|------|---------|---------------|----------------------|
| 1 | Inner  | No   | 10000   | 1000          | Accept               |
| 2 | Outer  | Yes  | 9000    | 3600          | Accept               |
| 3 | Middle | No   | 5400    | 2000          | Deny                 |
| 4 | Inner  | Yes  | 10000   | 1200          | Accept               |
| 5 | Outer  | No   | 8000    | 3000          | Accept (exactly ₹50) |
| 8 | Inner  | No   | 6000    | 1000          | Accept (exactly ₹50) |

## How to Run

```bash
g++ -std=c++17 -Wall -Wextra -o task09 task09.cpp
./task09
```
