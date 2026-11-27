# RVC System Test Scenarios

Generated from `simulator/web/scenarios.js` by `node simulator/gen_docs.js`.
Run them in the web simulator (`simulator/dist/rvc-simulator.html`).

## 1. Test Design Policy
- 21 scenarios, 10x10 grid, at most 100 ticks each.
- Each scenario names at least 3 essential system functions, its requirement IDs (PR-x from
  `artifacts/SA/process_specification.md`) and the SA artifacts it exercises.
- The test oracle is the human evaluator. The simulator shows requirement-check flags as
  hints only and never assigns PASS or FAIL.
- Coordinates: x = 0..9 (left to right), y = 0..9 (top to bottom). Heading N means toward y = 0.
- Default start (4,4) heading N. The requirements give no initial heading, so N was chosen.

## 2. Environment model (simulator only, not controller behavior)
- Front sensor = the cell ahead is an obstacle or outside the grid; delivered by interrupt
  (only when its value changes). Left/right sensors = the cell to the left/right of the heading;
  dust sensor = dust amount on the current cell > 0; delivered every *side period* ticks (default 1).
- forward / backward move one cell when that cell is free; stop does not move.
- A turn rotates the heading 90 degrees on the turn's 3rd tick (turn angle is not specified by the requirements).
- Each tick with Cleaner = clean_up removes 1 unit of dust from the current cell (dust amounts 1-9 let a
  cell need several clean_up ticks; the requirements do not specify dust amounts).

## 3. Scenarios

### ST-01 Open room: straight to the wall, then left turn

**Initial Configuration**
- Grid: 10x10
- RVC Start: (4,4)
- Initial Direction: N
- Obstacles: None
- Dust: None
- Left/right/dust sampling period: 1 tick(s); front by interrupt
- Maximum Ticks: 100

**Map**
```
..........
..........
..........
..........
..........
..........
..........
..........
..........
..........
```

**Test Objectives**
- Essential Function 1: Forward cleaning
- Essential Function 2: Obstacle avoidance by turning
- Essential Function 3: Left-turn priority
- Essential Function 4: 3-tick turn

**Requirement Traceability**
- Related Requirement IDs: PR-2, PR-3, PR-9, PR-10, PR-13
- Related SA Artifacts: FSM Forward/TurnLeft rows, Process 2.2.2

**Expected Behavior**
Goes north with forward + clean_on until the wall at y=0. With the front blocked and both sides clear it turns LEFT (left priority) for exactly 3 ticks with clean_off, then goes forward (west) with clean_on again.

**Human Evaluation**
- Result: Pending
- Evaluator:
- Observations:
- Related Tick(s):

### ST-02 Left side blocked: right turn

**Initial Configuration**
- Grid: 10x10
- RVC Start: (4,4)
- Initial Direction: N
- Obstacles: (4,1), (3,2)
- Dust: None
- Left/right/dust sampling period: 1 tick(s); front by interrupt
- Maximum Ticks: 100

**Map**
```
..........
....#.....
...#......
..........
..........
..........
..........
..........
..........
..........
```

**Test Objectives**
- Essential Function 1: Obstacle avoidance by turning
- Essential Function 2: Right turn when left is blocked
- Essential Function 3: 3-tick turn
- Essential Function 4: clean_off only while turning

**Requirement Traceability**
- Related Requirement IDs: PR-3, PR-9, PR-10, PR-13
- Related SA Artifacts: FSM row Forward->TurnRight, Process 2.2.2

**Expected Behavior**
At (4,2) the front (4,1) and the left (3,2) are blocked, the right is clear, so it turns RIGHT for 3 ticks with clean_off, then goes east with clean_on.

**Human Evaluation**
- Result: Pending
- Evaluator:
- Observations:
- Related Tick(s):

### ST-03 Single obstacle mid-room: left priority

**Initial Configuration**
- Grid: 10x10
- RVC Start: (4,4)
- Initial Direction: N
- Obstacles: (4,2)
- Dust: None
- Left/right/dust sampling period: 1 tick(s); front by interrupt
- Maximum Ticks: 100

**Map**
```
..........
..........
....#.....
..........
..........
..........
..........
..........
..........
..........
```

**Test Objectives**
- Essential Function 1: Forward cleaning
- Essential Function 2: Left-turn priority
- Essential Function 3: 3-tick turn

**Requirement Traceability**
- Related Requirement IDs: PR-2, PR-3, PR-9, PR-10
- Related SA Artifacts: FSM row Forward->TurnLeft

**Expected Behavior**
Stops cleaning in front of (4,2) and turns LEFT although the right is also clear. After 3 turn ticks it continues west with clean_on.

**Human Evaluation**
- Result: Pending
- Evaluator:
- Observations:
- Related Tick(s):

### ST-04 Dead-end corridor: backward until a side clears, then left

**Initial Configuration**
- Grid: 10x10
- RVC Start: (4,4)
- Initial Direction: N
- Obstacles: (4,0), (3,1), (5,1), (3,2), (5,2), (3,3), (5,3), (3,4), (5,4)
- Dust: None
- Left/right/dust sampling period: 1 tick(s); front by interrupt
- Maximum Ticks: 100

**Map**
```
....#.....
...#.#....
...#.#....
...#.#....
...#.#....
..........
..........
..........
..........
..........
```

**Test Objectives**
- Essential Function 1: Backward when front, left and right are blocked
- Essential Function 2: Backward continues until a side is clear
- Essential Function 3: Left-turn priority after backward
- Essential Function 4: clean_off while backward

**Requirement Traceability**
- Related Requirement IDs: PR-4, PR-14, PR-9, PR-13
- Related SA Artifacts: FSM Backward rows, Process 2.2.2

**Expected Behavior**
Drives up the corridor to (4,1). Front, left and right are blocked, so it moves BACKWARD with clean_off. Backward continues while both sides stay blocked; at (4,5) both sides are clear and it turns LEFT, then goes forward with clean_on.

**Human Evaluation**
- Result: Pending
- Evaluator:
- Observations:
- Related Tick(s):

### ST-05 Dead-end corridor: backward, then right turn

**Initial Configuration**
- Grid: 10x10
- RVC Start: (4,4)
- Initial Direction: N
- Obstacles: (4,0), (3,1), (5,1), (3,2), (5,2), (3,3), (5,3), (3,4), (5,4), (3,5), (3,6)
- Dust: None
- Left/right/dust sampling period: 1 tick(s); front by interrupt
- Maximum Ticks: 100

**Map**
```
....#.....
...#.#....
...#.#....
...#.#....
...#.#....
...#......
...#......
..........
..........
..........
```

**Test Objectives**
- Essential Function 1: Backward when all blocked
- Essential Function 2: Backward until a side is clear
- Essential Function 3: Right turn when only the right is clear

**Requirement Traceability**
- Related Requirement IDs: PR-4, PR-14, PR-9, PR-10
- Related SA Artifacts: FSM rows Backward->Backward, Backward->TurnRight

**Expected Behavior**
Backs out of the corridor. At (4,5) the left (3,5) is still blocked and the right is clear, so it turns RIGHT for 3 ticks, then goes forward.

**Human Evaluation**
- Result: Pending
- Evaluator:
- Observations:
- Related Tick(s):

### ST-06 Single dust cell: short clean_up

**Initial Configuration**
- Grid: 10x10
- RVC Start: (4,4)
- Initial Direction: N
- Obstacles: None
- Dust: (4,2)x1
- Left/right/dust sampling period: 1 tick(s); front by interrupt
- Maximum Ticks: 100

**Map**
```
..........
..........
....1.....
..........
..........
..........
..........
..........
..........
..........
```

**Test Objectives**
- Essential Function 1: Dust detection powers up cleaning
- Essential Function 2: clean_up ends when dust is gone
- Essential Function 3: Forward cleaning

**Requirement Traceability**
- Related Requirement IDs: PR-5, PR-11, PR-2, PR-12
- Related SA Artifacts: FSM rows Forward->CleanUpForward, CleanUpForward->Forward

**Expected Behavior**
On the dusty cell it switches to clean_up while still moving forward. On the next cell the dust is no longer detected, so the activation ends early (less than 3 ticks) and clean_on resumes.

**Human Evaluation**
- Result: Pending
- Evaluator:
- Observations:
- Related Tick(s):

### ST-07 Long dust strip: activations of at most 3 ticks

**Initial Configuration**
- Grid: 10x10
- RVC Start: (4,8)
- Initial Direction: N
- Obstacles: None
- Dust: (4,3)x1, (4,4)x1, (4,5)x1, (4,6)x1, (4,7)x1
- Left/right/dust sampling period: 1 tick(s); front by interrupt
- Maximum Ticks: 100

**Map**
```
..........
..........
..........
....1.....
....1.....
....1.....
....1.....
....1.....
..........
..........
```

**Test Objectives**
- Essential Function 1: Dust powers up cleaning
- Essential Function 2: One activation lasts at most 3 ticks
- Essential Function 3: Next activation starts while dust remains
- Essential Function 4: Forward cleaning

**Requirement Traceability**
- Related Requirement IDs: PR-5, PR-11, PR-2
- Related SA Artifacts: FSM rows CleanUpForward (activation ends, dust remains)

**Expected Behavior**
Over the 5 dusty cells it moves forward with clean_up. cleanup_count goes 1,2,3 and then restarts at 1 because dust is still detected (no activation longer than 3 ticks). After the strip it returns to clean_on.

**Human Evaluation**
- Result: Pending
- Evaluator:
- Observations:
- Related Tick(s):

### ST-08 Front obstacle during clean_up: stop and keep cleaning, then turn

**Initial Configuration**
- Grid: 10x10
- RVC Start: (4,4)
- Initial Direction: N
- Obstacles: (4,0)
- Dust: (4,1)x4, (4,2)x1
- Left/right/dust sampling period: 1 tick(s); front by interrupt
- Maximum Ticks: 100

**Map**
```
....#.....
....4.....
....1.....
..........
..........
..........
..........
..........
..........
..........
```

**Test Objectives**
- Essential Function 1: Dust powers up cleaning
- Essential Function 2: Stop in place when front is blocked during clean_up
- Essential Function 3: Repeated activations while stopped
- Essential Function 4: Turn after cleaning ends

**Requirement Traceability**
- Related Requirement IDs: PR-15, PR-11, PR-5, PR-3, PR-13
- Related SA Artifacts: FSM rows CleanUpForward->CleanUpStopped, CleanUpStopped rows

**Expected Behavior**
Enters clean_up on (4,2), moves to (4,1) where the front is blocked and dust is present: direction becomes stop with clean_up and it stays at (4,1). It keeps cleaning (new activation when 3 ticks end) and does not turn or move backward until the dust is gone; then it turns LEFT.

**Human Evaluation**
- Result: Pending
- Evaluator:
- Observations:
- Related Tick(s):

### ST-09 clean_up stopped in a dead end: backward after cleaning

**Initial Configuration**
- Grid: 10x10
- RVC Start: (4,4)
- Initial Direction: N
- Obstacles: (4,0), (3,1), (5,1)
- Dust: (4,1)x2, (4,2)x1
- Left/right/dust sampling period: 1 tick(s); front by interrupt
- Maximum Ticks: 100

**Map**
```
....#.....
...#2#....
....1.....
..........
..........
..........
..........
..........
..........
..........
```

**Test Objectives**
- Essential Function 1: Stop and keep cleaning during clean_up
- Essential Function 2: Backward when all sides are blocked after cleaning
- Essential Function 3: clean_off only while backward

**Requirement Traceability**
- Related Requirement IDs: PR-15, PR-4, PR-13, PR-11
- Related SA Artifacts: FSM row CleanUpStopped->Backward

**Expected Behavior**
Stops at (4,1) with clean_up until the dust is gone. Then, with front, left and right blocked, it moves backward with clean_off, and turns when a side becomes clear.

**Human Evaluation**
- Result: Pending
- Evaluator:
- Observations:
- Related Tick(s):

### ST-10 Dust while moving backward: one stop-and-clean activation

**Initial Configuration**
- Grid: 10x10
- RVC Start: (4,4)
- Initial Direction: N
- Obstacles: (4,0), (3,1), (5,1), (3,2), (5,2), (3,3), (5,3), (3,4), (5,4)
- Dust: (4,2)x3
- Left/right/dust sampling period: 1 tick(s); front by interrupt
- Maximum Ticks: 100

**Map**
```
....#.....
...#.#....
...#3#....
...#.#....
...#.#....
..........
..........
..........
..........
..........
```

**Test Objectives**
- Essential Function 1: Backward when all blocked
- Essential Function 2: Dust while backward: stop and clean_up
- Essential Function 3: Return to backward after one activation

**Requirement Traceability**
- Related Requirement IDs: PR-16, PR-4, PR-14, PR-11
- Related SA Artifacts: FSM rows Backward->CleanUpBackwardPause->Backward

**Expected Behavior**
After reaching the dead end it moves backward. On the dusty cell it stops with clean_up for one activation (at most 3 ticks, shorter if dust disappears), then continues backward with clean_off, then turns when a side is clear.

**Human Evaluation**
- Result: Pending
- Evaluator:
- Observations:
- Related Tick(s):

### ST-11 Heavy dust while backing up: activation limit

**Initial Configuration**
- Grid: 10x10
- RVC Start: (4,4)
- Initial Direction: N
- Obstacles: (4,0), (3,1), (5,1), (3,2), (5,2), (3,3), (5,3), (3,4), (5,4)
- Dust: (4,2)x9
- Left/right/dust sampling period: 1 tick(s); front by interrupt
- Maximum Ticks: 100

**Map**
```
....#.....
...#.#....
...#9#....
...#.#....
...#.#....
..........
..........
..........
..........
..........
```

**Test Objectives**
- Essential Function 1: Dust while backward
- Essential Function 2: Activation limited to 3 ticks
- Essential Function 3: Backward resumes even if dust remains

**Requirement Traceability**
- Related Requirement IDs: PR-16, PR-11, PR-14
- Related SA Artifacts: FSM row CleanUpBackwardPause (cleanup_count = 3) -> Backward

**Expected Behavior**
While backing up over the heavy dust cell it stops for exactly 3 clean_up ticks, then moves backward again although dust is still detected (one activation only).

**Human Evaluation**
- Result: Pending
- Evaluator:
- Observations:
- Related Tick(s):

### ST-12 Dust detected during a turn

**Initial Configuration**
- Grid: 10x10
- RVC Start: (4,4)
- Initial Direction: N
- Obstacles: None
- Dust: (4,0)x1
- Left/right/dust sampling period: 1 tick(s); front by interrupt
- Maximum Ticks: 100

**Map**
```
....1.....
..........
..........
..........
..........
..........
..........
..........
..........
..........
```

**Test Objectives**
- Essential Function 1: 3-tick turn is not interrupted
- Essential Function 2: Dust during turn: forward in clean_up afterwards
- Essential Function 3: Left-turn priority

**Requirement Traceability**
- Related Requirement IDs: PR-17, PR-10, PR-5, PR-9
- Related SA Artifacts: FSM rows TurnLeft->CleanUpForward, dust_during_turn

**Expected Behavior**
At the wall cell (4,0) there is dust. It still turns LEFT for 3 ticks with clean_off (dust_during_turn = 1), and after the turn it goes forward with clean_up.

**Human Evaluation**
- Result: Pending
- Evaluator:
- Observations:
- Related Tick(s):

### ST-13 Corner start: immediate right turn

**Initial Configuration**
- Grid: 10x10
- RVC Start: (0,0)
- Initial Direction: N
- Obstacles: None
- Dust: None
- Left/right/dust sampling period: 1 tick(s); front by interrupt
- Maximum Ticks: 100

**Map**
```
..........
..........
..........
..........
..........
..........
..........
..........
..........
..........
```

**Test Objectives**
- Essential Function 1: Obstacle avoidance from the first tick
- Essential Function 2: Right turn when left is blocked
- Essential Function 3: 3-tick turn

**Requirement Traceability**
- Related Requirement IDs: PR-3, PR-9, PR-10, PR-8
- Related SA Artifacts: FSM row Forward->TurnRight, Interpretation 2 (initial sensor values)

**Expected Behavior**
Front (outside the grid) and left (outside) are blocked on tick 1, so it turns RIGHT for 3 ticks, then goes east with clean_on.

**Human Evaluation**
- Result: Pending
- Evaluator:
- Observations:
- Related Tick(s):

### ST-14 Fully enclosed: keeps moving backward

**Initial Configuration**
- Grid: 10x10
- RVC Start: (4,4)
- Initial Direction: N
- Obstacles: (4,3), (3,4), (5,4), (4,5)
- Dust: None
- Left/right/dust sampling period: 1 tick(s); front by interrupt
- Maximum Ticks: 100

**Map**
```
..........
..........
..........
....#.....
...#.#....
....#.....
..........
..........
..........
..........
```

**Test Objectives**
- Essential Function 1: Backward when all blocked
- Essential Function 2: Backward continues until a side is clear
- Essential Function 3: clean_off only while backward

**Requirement Traceability**
- Related Requirement IDs: PR-4, PR-14, PR-13
- Related SA Artifacts: FSM row Backward->Backward

**Expected Behavior**
All four neighbors are obstacles. It commands backward with clean_off on every tick (the cell behind is blocked, so it does not move) and never turns because no side becomes clear.

**Human Evaluation**
- Result: Pending
- Evaluator:
- Observations:
- Related Tick(s):

### ST-15 Slow periodic sensors in a cluttered room

**Initial Configuration**
- Grid: 10x10
- RVC Start: (4,4)
- Initial Direction: N
- Obstacles: (2,0), (8,1), (0,2), (4,2), (7,3), (8,3), (6,4), (1,5), (5,6), (6,7), (1,8), (4,9)
- Dust: (5,1)x3, (2,3)x1, (9,5)x2, (3,6)x2, (7,8)x1
- Left/right/dust sampling period: 3 tick(s); front by interrupt
- Maximum Ticks: 100

**Map**
```
..#.......
.....3..#.
#...#.....
..1....##.
......#...
.#.......2
...2.#....
......#...
.#.....1..
....#.....
```

**Test Objectives**
- Essential Function 1: Latest sensor values are used
- Essential Function 2: Obstacle avoidance
- Essential Function 3: Dust power-up
- Essential Function 4: Forward cleaning

**Requirement Traceability**
- Related Requirement IDs: PR-8, PR-3, PR-5, PR-2
- Related SA Artifacts: Process 1 Capture Sensor Values, D1 Latest Sensor Values

**Expected Behavior**
Left, right and dust samples arrive only every 3 ticks; the front arrives by interrupt when it changes. The controller decides from the latest stored values, so a turn direction may use a side value up to 2 ticks old. The RVC never enters an obstacle cell.

**Human Evaluation**
- Result: Pending
- Evaluator:
- Observations:
- Related Tick(s):

### ST-16 Corridor with periodic side sensors (period 2)

**Initial Configuration**
- Grid: 10x10
- RVC Start: (4,4)
- Initial Direction: N
- Obstacles: (4,0), (3,1), (5,1), (3,2), (5,2), (3,3), (5,3), (3,4), (5,4)
- Dust: None
- Left/right/dust sampling period: 2 tick(s); front by interrupt
- Maximum Ticks: 100

**Map**
```
....#.....
...#.#....
...#.#....
...#.#....
...#.#....
..........
..........
..........
..........
..........
```

**Test Objectives**
- Essential Function 1: Interrupt front / periodic sides
- Essential Function 2: Backward until a side is clear
- Essential Function 3: Left-turn priority

**Requirement Traceability**
- Related Requirement IDs: PR-8, PR-14, PR-9, PR-4
- Related SA Artifacts: Process 1.1-1.4, FSM Backward rows

**Expected Behavior**
Same path as ST-04, but backward may last one tick longer because the side values are refreshed every 2 ticks. It still turns LEFT when both sides read clear.

**Human Evaluation**
- Result: Pending
- Evaluator:
- Observations:
- Related Tick(s):

### ST-17 Start facing east

**Initial Configuration**
- Grid: 10x10
- RVC Start: (4,4)
- Initial Direction: E
- Obstacles: None
- Dust: None
- Left/right/dust sampling period: 1 tick(s); front by interrupt
- Maximum Ticks: 100

**Map**
```
..........
..........
..........
..........
..........
..........
..........
..........
..........
..........
```

**Test Objectives**
- Essential Function 1: Forward cleaning
- Essential Function 2: Obstacle avoidance by turning
- Essential Function 3: Left-turn priority
- Essential Function 4: 3-tick turn

**Requirement Traceability**
- Related Requirement IDs: PR-2, PR-3, PR-9, PR-10
- Related SA Artifacts: FSM Forward/TurnLeft rows

**Expected Behavior**
Goes east to x=9, turns LEFT (to the north) for 3 ticks, then continues north with clean_on.

**Human Evaluation**
- Result: Pending
- Evaluator:
- Observations:
- Related Tick(s):

### ST-18 Start facing south, left blocked at the wall

**Initial Configuration**
- Grid: 10x10
- RVC Start: (4,4)
- Initial Direction: S
- Obstacles: (5,9)
- Dust: None
- Left/right/dust sampling period: 1 tick(s); front by interrupt
- Maximum Ticks: 100

**Map**
```
..........
..........
..........
..........
..........
..........
..........
..........
..........
.....#....
```

**Test Objectives**
- Essential Function 1: Obstacle avoidance
- Essential Function 2: Right turn when left is blocked
- Essential Function 3: 3-tick turn
- Essential Function 4: clean_off only while turning

**Requirement Traceability**
- Related Requirement IDs: PR-3, PR-9, PR-10, PR-13
- Related SA Artifacts: FSM row Forward->TurnRight

**Expected Behavior**
Goes south to (4,9). The front (wall) and its left (east, (5,9)) are blocked, so it turns RIGHT (to the west) for 3 ticks, then goes west.

**Human Evaluation**
- Result: Pending
- Evaluator:
- Observations:
- Related Tick(s):

### ST-19 Cluttered room, 100 ticks

**Initial Configuration**
- Grid: 10x10
- RVC Start: (4,4)
- Initial Direction: N
- Obstacles: (2,0), (8,1), (0,2), (4,2), (7,3), (8,3), (6,4), (1,5), (5,6), (6,7), (1,8), (4,9)
- Dust: (5,1)x3, (2,3)x1, (9,5)x2, (3,6)x2, (7,8)x1
- Left/right/dust sampling period: 1 tick(s); front by interrupt
- Maximum Ticks: 100

**Map**
```
..#.......
.....3..#.
#...#.....
..1....##.
......#...
.#.......2
...2.#....
......#...
.#.....1..
....#.....
```

**Test Objectives**
- Essential Function 1: Forward cleaning
- Essential Function 2: Obstacle avoidance
- Essential Function 3: Backward escape
- Essential Function 4: Dust power-up

**Requirement Traceability**
- Related Requirement IDs: PR-2, PR-3, PR-4, PR-5, PR-10, PR-11, PR-13
- Related SA Artifacts: Level 4 FSM, Process 2.2.2

**Expected Behavior**
Runs the whole 100 ticks without entering an obstacle. Every turn lasts 3 ticks, every clean_up activation at most 3 ticks, and clean_off appears only while turning or backing up.

**Human Evaluation**
- Result: Pending
- Evaluator:
- Observations:
- Related Tick(s):

### ST-20 Scattered dust field

**Initial Configuration**
- Grid: 10x10
- RVC Start: (4,4)
- Initial Direction: N
- Obstacles: None
- Dust: (2,0)x1, (4,1)x2, (1,2)x1, (8,2)x1, (4,3)x1, (7,4)x2, (2,5)x3, (6,6)x1, (1,7)x1, (4,8)x2, (8,8)x1
- Left/right/dust sampling period: 1 tick(s); front by interrupt
- Maximum Ticks: 100

**Map**
```
..1.......
....2.....
.1......1.
....1.....
.......2..
..3.......
......1...
.1........
....2...1.
..........
```

**Test Objectives**
- Essential Function 1: Dust powers up cleaning
- Essential Function 2: Activation limit / early end
- Essential Function 3: Return to clean_on
- Essential Function 4: Obstacle (wall) avoidance

**Requirement Traceability**
- Related Requirement IDs: PR-5, PR-11, PR-2, PR-3
- Related SA Artifacts: FSM CleanUpForward rows

**Expected Behavior**
Each time a dusty cell is under the RVC it switches to clean_up while moving; the activation ends when dust is no longer detected or after 3 ticks, and clean_on resumes. Walls are avoided by turns.

**Human Evaluation**
- Result: Pending
- Evaluator:
- Observations:
- Related Tick(s):

### ST-21 Stopped clean_up, then right turn

**Initial Configuration**
- Grid: 10x10
- RVC Start: (4,4)
- Initial Direction: N
- Obstacles: (4,0), (3,1)
- Dust: (4,1)x2, (4,2)x1
- Left/right/dust sampling period: 1 tick(s); front by interrupt
- Maximum Ticks: 100

**Map**
```
....#.....
...#2.....
....1.....
..........
..........
..........
..........
..........
..........
..........
```

**Test Objectives**
- Essential Function 1: Stop and keep cleaning during clean_up
- Essential Function 2: Turn selection after cleaning ends
- Essential Function 3: Right turn when left is blocked

**Requirement Traceability**
- Related Requirement IDs: PR-15, PR-9, PR-3, PR-10
- Related SA Artifacts: FSM row CleanUpStopped->TurnRight

**Expected Behavior**
Stops at (4,1) with clean_up until the dust is gone; then, with the front and left blocked and the right clear, it turns RIGHT for 3 ticks.

**Human Evaluation**
- Result: Pending
- Evaluator:
- Observations:
- Related Tick(s):
