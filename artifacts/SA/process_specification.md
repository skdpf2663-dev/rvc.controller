# Process Specification

Specifications are grouped by DFD level. Process 2.2.2 is specified here because it is a Level 3 process. The Level 4 FSM is the state model of that same process and does not have a separate process specification.

The transition table in `artifacts/SA/DFD/dfd_level_4_fsm.md` and the pseudocode in Process 2.2.2 describe the same decisions.

## Requirement labels

| Label | Requirement statement |
|---|---|
| PR-1 | The RVC automatically cleans and mops a household surface. |
| PR-2 | It goes straight forward while cleaning. |
| PR-3 | If a sensor finds an obstacle, it stops cleaning, turns left or right, and then goes forward while cleaning. |
| PR-4 | If the front, left, and right are all blocked, it moves backward, then turns left or right, and then goes forward. |
| PR-5 | If it detects dust, it powers up cleaning for a while. |
| PR-6 | Hardware control detail is out of scope. |
| PR-7 | The scope is the automatic cleaning function. |
| PR-8 | Front input arrives by interrupt. Left, right, and dust inputs arrive periodically. Every sensor value is boolean. The controller uses the latest value of each sensor. |
| PR-9 | If both a left turn and a right turn are possible, the left turn has priority. |
| PR-10 | The controller has an internal tick. A turn lasts 3 ticks. |
| PR-11 | One clean_up activation lasts at most 3 ticks and ends sooner when dust is no longer detected. |
| PR-12 | The cleaner mode is clean_off, clean_on, or clean_up. |
| PR-13 | clean_off is used only while the RVC is moving backward or turning. Direction can command a stop. |
| PR-14 | Backward continues until the left side or the right side is clear. The RVC then turns, using left-turn priority when both sides are clear. |
| PR-15 | If a front obstacle is found during clean_up, the RVC stops and keeps cleaning until dust is no longer detected. It does not turn or move backward until that cleaning ends. Each activation is still at most 3 ticks. If dust remains when an activation ends, the next activation starts while the RVC stays stopped. |
| PR-16 | If dust is detected while moving backward, the RVC stops, runs one clean_up activation, and then moves backward again. |
| PR-17 | If dust is detected during a turn, the turn finishes. The RVC then goes forward in clean_up. |

---

# Level 0

## Process 0 — RVC Controller

| Field | Description |
|---|---|
| Process ID | 0 |
| Process Name | RVC Controller |
| DFD Level | 0 |
| Parent Process | None |

### Purpose
Perform automatic cleaning by keeping the latest sensor values, deciding motion and cleaner mode on each internal tick, and issuing Direction and Clean.

### Inputs
- Front Sensor Input
- Left Sensor Input
- Right Sensor Input
- Dust Sensor Input

### Outputs
- Direction
- Clean

### Trigger / Execution Condition
The process runs continuously while automatic cleaning is active. Sensor inputs are accepted when they arrive. Motion and cleaner decisions are made on each Tick.

### Processing Logic
1. Accept each arriving sensor sample and retain the latest value of that sensor.
2. On each internal tick, select Direction and Clean from the latest sensor values and from the current cleaning state.
3. Send Direction to the motor and Clean to the cleaner.

### Relevant Requirements
PR-1, PR-2, PR-3, PR-4, PR-5, PR-6, PR-7, PR-8, PR-9, PR-10, PR-11, PR-12, PR-13, PR-14, PR-15, PR-16, PR-17.

---

# Level 1

## Process 1 — Capture Sensor Values

| Field | Description |
|---|---|
| Process ID | 1 |
| Process Name | Capture Sensor Values |
| DFD Level | 1 |
| Parent Process | 0 |

### Purpose
Receive sensor inputs and store the latest boolean value of each sensor.

### Inputs
- Front Sensor Input
- Left Sensor Input
- Right Sensor Input
- Dust Sensor Input

### Outputs
- Sensor Status, written to D1 Latest Sensor Values

### Trigger / Execution Condition
The matching input is processed when that sensor value arrives. Arrival of one sensor does not change the stored values of the other sensors.

### Processing Logic
1. When Front Sensor Input arrives, store it as the latest front value.
2. When Left Sensor Input arrives, store it as the latest left value.
3. When Right Sensor Input arrives, store it as the latest right value.
4. When Dust Sensor Input arrives, store it as the latest dust value.

### Relevant Requirements
PR-8.

## Process 2 — Control Automatic Cleaning

| Field | Description |
|---|---|
| Process ID | 2 |
| Process Name | Control Automatic Cleaning |
| DFD Level | 1 |
| Parent Process | 0 |

### Purpose
Generate the internal tick and, on each tick, decide the next Direction Command and Clean Command from the latest sensor values.

### Inputs
- Latest Sensor Values

### Outputs
- Direction Command
- Clean Command

### Trigger / Execution Condition
One decision is produced for each Tick.

### Processing Logic
1. Produce the next Tick.
2. Read the latest front, left, right, and dust values.
3. Apply the automatic-cleaning decision rules refined by Process 2.2.2.
4. Emit one Direction Command and one Clean Command.

### Relevant Requirements
PR-2, PR-3, PR-4, PR-5, PR-9, PR-10, PR-11, PR-12, PR-13, PR-14, PR-15, PR-16, PR-17.

## Process 3 — Issue Actuator Commands

| Field | Description |
|---|---|
| Process ID | 3 |
| Process Name | Issue Actuator Commands |
| DFD Level | 1 |
| Parent Process | 0 |

### Purpose
Deliver the controller's movement and cleaner decisions as the external Direction and Clean outputs.

### Inputs
- Direction Command
- Clean Command

### Outputs
- Direction
- Clean

### Trigger / Execution Condition
The process runs when Process 2 issues a Direction Command and a Clean Command.

### Processing Logic
1. Accept Direction Command and send it as Direction.
2. Accept Clean Command and send it as Clean.
3. Do not change the commanded value and do not add hardware-control behavior.

### Relevant Requirements
PR-6, PR-12, PR-13.

---

# Level 2

## Process 1.1 — Update Front Sensor

| Field | Description |
|---|---|
| Process ID | 1.1 |
| Process Name | Update Front Sensor |
| DFD Level | 2 |
| Parent Process | 1 |

### Purpose
Store the latest front-sensor sample.

### Inputs
- Front Sensor Input

### Outputs
- Front Obstacle, written to D1

### Trigger / Execution Condition
A front-sensor interrupt delivers a new Front Sensor Input.

### Processing Logic
1. Read Front Sensor Input.
2. Store that boolean as Front Obstacle in D1, replacing only the previous front value.

### Pseudocode

```
ON Front Sensor Input:
    D1.Front Obstacle = Front Sensor Input
```

### Relevant Requirements
PR-8.

## Process 1.2 — Update Left Sensor

| Field | Description |
|---|---|
| Process ID | 1.2 |
| Process Name | Update Left Sensor |
| DFD Level | 2 |
| Parent Process | 1 |

### Purpose
Store the latest left-sensor sample.

### Inputs
- Left Sensor Input

### Outputs
- Left Obstacle, written to D1

### Trigger / Execution Condition
A periodic left-sensor sample arrives.

### Processing Logic
1. Read Left Sensor Input.
2. Store that boolean as Left Obstacle in D1, replacing only the previous left value.

### Pseudocode

```
ON Left Sensor Input:
    D1.Left Obstacle = Left Sensor Input
```

### Relevant Requirements
PR-8.

## Process 1.3 — Update Right Sensor

| Field | Description |
|---|---|
| Process ID | 1.3 |
| Process Name | Update Right Sensor |
| DFD Level | 2 |
| Parent Process | 1 |

### Purpose
Store the latest right-sensor sample.

### Inputs
- Right Sensor Input

### Outputs
- Right Obstacle, written to D1

### Trigger / Execution Condition
A periodic right-sensor sample arrives.

### Processing Logic
1. Read Right Sensor Input.
2. Store that boolean as Right Obstacle in D1, replacing only the previous right value.

### Pseudocode

```
ON Right Sensor Input:
    D1.Right Obstacle = Right Sensor Input
```

### Relevant Requirements
PR-8.

## Process 1.4 — Update Dust Sensor

| Field | Description |
|---|---|
| Process ID | 1.4 |
| Process Name | Update Dust Sensor |
| DFD Level | 2 |
| Parent Process | 1 |

### Purpose
Store the latest dust-sensor sample.

### Inputs
- Dust Sensor Input

### Outputs
- Dust Detected, written to D1

### Trigger / Execution Condition
A periodic dust-sensor sample arrives.

### Processing Logic
1. Read Dust Sensor Input.
2. Store that boolean as Dust Detected in D1, replacing only the previous dust value.

### Pseudocode

```
ON Dust Sensor Input:
    D1.Dust Detected = Dust Sensor Input
```

### Relevant Requirements
PR-5, PR-8.

## Process 2.1 — Generate Tick

| Field | Description |
|---|---|
| Process ID | 2.1 |
| Process Name | Generate Tick |
| DFD Level | 2 |
| Parent Process | 2 |

### Purpose
Produce the controller's internal Tick.

### Inputs
- None

### Outputs
- Tick

### Trigger / Execution Condition
The process repeats for as long as automatic cleaning is active. The time between ticks is TBD.

### Processing Logic
1. Produce one Tick occurrence.
2. Send that Tick to Process 2.2.

### Relevant Requirements
PR-10.

## Process 2.2 — Determine Control Action

| Field | Description |
|---|---|
| Process ID | 2.2 |
| Process Name | Determine Control Action |
| DFD Level | 2 |
| Parent Process | 2 |

### Purpose
On each tick, read the latest sensor values and select the Direction Command and Clean Command.

### Inputs
- Latest Sensor Values
- Tick

### Outputs
- Direction Command
- Clean Command

### Trigger / Execution Condition
The process executes once for each Tick.

### Processing Logic
1. Read Front Obstacle, Left Obstacle, Right Obstacle, and Dust Detected from D1.
2. Evaluate Process 2.2.2 with those values and the current controller state.
3. Emit the resulting Direction Command and Clean Command.

### Relevant Requirements
PR-2, PR-3, PR-4, PR-5, PR-9, PR-10, PR-11, PR-12, PR-13, PR-14, PR-15, PR-16, PR-17.

## Process 3.1 — Command Direction

| Field | Description |
|---|---|
| Process ID | 3.1 |
| Process Name | Command Direction |
| DFD Level | 2 |
| Parent Process | 3 |

### Purpose
Send the selected movement command to the motor.

### Inputs
- Direction Command

### Outputs
- Direction

### Trigger / Execution Condition
A Direction Command is available.

### Processing Logic
1. Set Direction to the value of Direction Command.
2. Send Direction to the motor.

### Pseudocode

```
ON Direction Command:
    Direction = Direction Command
    SEND Direction TO Motor
```

### Relevant Requirements
PR-6, PR-13.

## Process 3.2 — Command Cleaner Mode

| Field | Description |
|---|---|
| Process ID | 3.2 |
| Process Name | Command Cleaner Mode |
| DFD Level | 2 |
| Parent Process | 3 |

### Purpose
Send the selected cleaner mode to the cleaner.

### Inputs
- Clean Command

### Outputs
- Clean

### Trigger / Execution Condition
A Clean Command is available.

### Processing Logic
1. Set Clean to the value of Clean Command.
2. Send Clean to the cleaner.

### Pseudocode

```
ON Clean Command:
    Clean = Clean Command
    SEND Clean TO Cleaner
```

### Relevant Requirements
PR-6, PR-12, PR-13.

---

# Level 3

## Process 2.2.1 — Read Latest Sensor Values

| Field | Description |
|---|---|
| Process ID | 2.2.1 |
| Process Name | Read Latest Sensor Values |
| DFD Level | 3 |
| Parent Process | 2.2 |

### Purpose
Read the four stored sensor values and pass them to the controller without changing them.

### Inputs
- Latest Sensor Values from D1

### Outputs
- Front Obstacle
- Left Obstacle
- Right Obstacle
- Dust Detected

### Trigger / Execution Condition
Process 2.2.2 requests the latest values on a Tick.

### Processing Logic
1. Read the four fields from D1.
2. Pass each field to Process 2.2.2 under the same name.

### Pseudocode

```
ON request from Process 2.2.2:
    Front Obstacle = D1.Front Obstacle
    Left Obstacle = D1.Left Obstacle
    Right Obstacle = D1.Right Obstacle
    Dust Detected = D1.Dust Detected
```

### Relevant Requirements
PR-8.

## Process 2.2.2 — Control Cleaning Behavior

| Field | Description |
|---|---|
| Process ID | 2.2.2 |
| Process Name | Control Cleaning Behavior |
| DFD Level | 3 |
| Parent Process | 2.2 |

### Purpose
Select one movement command and one cleaner mode for the current tick. This process is the Controller.

### Inputs
- Tick
- Front Obstacle
- Left Obstacle
- Right Obstacle
- Dust Detected

### Outputs
- Direction Command
- Clean Command

### Trigger / Execution Condition
The process executes once for each Tick. Automatic cleaning begins in Forward before the first tick, with Direction Command = forward and Clean Command = clean_on.

### Processing Logic
The controller keeps `state`, `turn_count`, `cleanup_count`, and `dust_during_turn` internally.

On a tick that starts a turn, `turn_count` becomes 1 and `dust_during_turn` receives the current Dust Detected value. On each later tick still inside that turn, Dust Detected true sets `dust_during_turn` true. While `turn_count` is less than 3, the tick increments the count and repeats the turn command. The tick that finds `turn_count` already equal to 3 leaves the turn and does not issue another turn command.

On a tick that starts a clean_up activation, `cleanup_count` becomes 1. While the count is less than 3 and dust remains, a continuing tick increments the count and repeats clean_up. The activation ends when `cleanup_count` is already 3 or Dust Detected is false.

A front obstacle in Forward is handled before dust. A clear side in Backward is handled before dust. Left has priority over right whenever both turns are possible.

### Pseudocode

```
ON start of automatic cleaning:
    state = Forward
    turn_count = 0
    cleanup_count = 0
    dust_during_turn = false
    Direction Command = forward
    Clean Command = clean_on

ON Tick:
    F = Front Obstacle
    L = Left Obstacle
    R = Right Obstacle
    D = Dust Detected

    IF state = TurnLeft OR state = TurnRight THEN
        IF D = true THEN
            dust_during_turn = true
        END IF
    END IF

    IF state = Forward THEN
        IF F = true AND L = false THEN
            state = TurnLeft
            turn_count = 1
            dust_during_turn = D
            Direction Command = turn_left
            Clean Command = clean_off
        ELSE IF F = true AND L = true AND R = false THEN
            state = TurnRight
            turn_count = 1
            dust_during_turn = D
            Direction Command = turn_right
            Clean Command = clean_off
        ELSE IF F = true AND L = true AND R = true THEN
            state = Backward
            Direction Command = backward
            Clean Command = clean_off
        ELSE IF F = false AND D = true THEN
            state = CleanUpForward
            cleanup_count = 1
            Direction Command = forward
            Clean Command = clean_up
        ELSE
            Direction Command = forward
            Clean Command = clean_on
        END IF

    ELSE IF state = TurnLeft OR state = TurnRight THEN
        turn_command = turn_left
        IF state = TurnRight THEN
            turn_command = turn_right
        END IF
        IF turn_count < 3 THEN
            turn_count = turn_count + 1
            Direction Command = turn_command
            Clean Command = clean_off
        ELSE IF dust_during_turn = true THEN
            state = CleanUpForward
            cleanup_count = 1
            dust_during_turn = false
            Direction Command = forward
            Clean Command = clean_up
        ELSE
            state = Forward
            dust_during_turn = false
            Direction Command = forward
            Clean Command = clean_on
        END IF

    ELSE IF state = Backward THEN
        IF L = false THEN
            state = TurnLeft
            turn_count = 1
            dust_during_turn = D
            Direction Command = turn_left
            Clean Command = clean_off
        ELSE IF L = true AND R = false THEN
            state = TurnRight
            turn_count = 1
            dust_during_turn = D
            Direction Command = turn_right
            Clean Command = clean_off
        ELSE IF D = true THEN
            state = CleanUpBackwardPause
            cleanup_count = 1
            Direction Command = stop
            Clean Command = clean_up
        ELSE
            Direction Command = backward
            Clean Command = clean_off
        END IF

    ELSE IF state = CleanUpBackwardPause THEN
        IF cleanup_count = 3 OR D = false THEN
            state = Backward
            Direction Command = backward
            Clean Command = clean_off
        ELSE
            cleanup_count = cleanup_count + 1
            Direction Command = stop
            Clean Command = clean_up
        END IF

    ELSE IF state = CleanUpForward OR state = CleanUpStopped THEN
        activation_ended = (cleanup_count = 3 OR D = false)
        IF activation_ended = false AND state = CleanUpForward AND F = true THEN
            state = CleanUpStopped
            cleanup_count = cleanup_count + 1
            Direction Command = stop
            Clean Command = clean_up
        ELSE IF activation_ended = false AND state = CleanUpForward AND F = false THEN
            cleanup_count = cleanup_count + 1
            Direction Command = forward
            Clean Command = clean_up
        ELSE IF activation_ended = false AND state = CleanUpStopped THEN
            cleanup_count = cleanup_count + 1
            Direction Command = stop
            Clean Command = clean_up
        ELSE IF D = true AND (state = CleanUpStopped OR F = true) THEN
            state = CleanUpStopped
            cleanup_count = 1
            Direction Command = stop
            Clean Command = clean_up
        ELSE IF D = true AND F = false THEN
            state = CleanUpForward
            cleanup_count = 1
            Direction Command = forward
            Clean Command = clean_up
        ELSE IF F = true AND L = false THEN
            state = TurnLeft
            turn_count = 1
            dust_during_turn = false
            Direction Command = turn_left
            Clean Command = clean_off
        ELSE IF F = true AND L = true AND R = false THEN
            state = TurnRight
            turn_count = 1
            dust_during_turn = false
            Direction Command = turn_right
            Clean Command = clean_off
        ELSE IF F = true AND L = true AND R = true THEN
            state = Backward
            Direction Command = backward
            Clean Command = clean_off
        ELSE
            state = Forward
            Direction Command = forward
            Clean Command = clean_on
        END IF
    END IF
```

The clean_up branch uses one order. A continuing activation is handled first. When the activation has ended and dust remains, the next activation starts immediately: stopped if the controller is already stopped or the front is blocked, and forward otherwise. When the activation has ended and dust is absent, the latest front and side values select a turn, backward movement, or forward cleaning.

### Relevant Requirements
PR-2, PR-3, PR-4, PR-5, PR-9, PR-10, PR-11, PR-12, PR-13, PR-14, PR-15, PR-16, PR-17.

---

# Hierarchical consistency

| Parent | Children | Consistency |
|---|---|---|
| 0 | 1, 2, 3 | Children cover sensor capture, tick-based decisions, and actuator output. External inputs and outputs are unchanged. |
| 1 | 1.1, 1.2, 1.3, 1.4 | Each child stores one sensor. Together they produce Sensor Status. |
| 2 | 2.1, 2.2 | Tick generation and the control decision stay inside Process 2. |
| 3 | 3.1, 3.2 | Each child delivers one parent output unchanged. |
| 2.2 | 2.2.1, 2.2.2 | The reader supplies the four latest values. The controller produces both commands. |

Every process in DFD Levels 0 through 3 has one specification above. Process names, inputs, and outputs use the DFD flow names.
