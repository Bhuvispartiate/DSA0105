# Task 01 – Understand the Problem and Identify the Objects

**Marks:** 4

## Objective
Identify the key objects in the Namma Metro smart-card fare system and apply basic OOP principles.

## Objects Identified

| Object     | Data                                      | Behaviour                          |
|------------|-------------------------------------------|------------------------------------|
| SmartCard  | balance, cardId, cardType                 | canTravel(), deduct(), recharge()  |
| Passenger  | name, age, category                       | isEligibleForConcession()          |
| Journey    | entryZone, exitZone, hour, isWeekend      | computeFare(), isPeak()            |
| Zone       | zoneId, baseFarePaise                     | getBaseFare()                      |
| FareTable  | base fares, peak %, concession rates      | lookupFare()                       |

## OOP Principles Applied

- **Encapsulation** – balance is private inside SmartCard
- **Abstraction** – the gate only needs to know whether the card can travel
- **Inheritance** – future StudentCard / SeniorCard can inherit from SmartCard
- **Polymorphism** – different card types can respond to the same method differently

## Rules Every Journey Must Satisfy

1. After a successful journey the balance must be at least ₹50 (5000 paise)
2. Fare is calculated from the farthest zone travelled
3. Peak surcharge (20 %) applies only in the intervals [8,10) and [17,20)
4. A journey is refused if the resulting balance would fall below the minimum
5. Concession is applied only when the passenger category qualifies

## How to Run

```bash
g++ -std=c++17 -Wall -Wextra -o task01 task01.cpp
./task01
```
