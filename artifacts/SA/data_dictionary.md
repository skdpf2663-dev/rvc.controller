# Data Dictionary

This dictionary was initialized from the Event List and then extended as DFD Levels 0 through 3 were added. Earlier names are retained. Tick length and the left, right, and dust sampling periods are unspecified and remain TBD.

Boolean true means detected. Boolean false means not detected. This polarity is an interpretation of the boolean sensor requirement and is recorded in the Level 4 FSM.

## 1. External Input/Output Events

| Input/Output Event | Description | Format / Type |
|---|---|---|
| Front Sensor Input | Interrupt input. Latest boolean front-obstacle sample. | Boolean |
| Left Sensor Input | Periodic input. Latest boolean left-obstacle sample. | Boolean |
| Right Sensor Input | Periodic input. Latest boolean right-obstacle sample. | Boolean |
| Dust Sensor Input | Periodic input. Latest boolean dust sample. | Boolean |
| Tick | Internal temporal event. Not an external context flow. Turn and clean_up durations are counted in ticks. | Event. Period TBD. |
| Direction | Movement command to the motor. | Enumeration: forward, backward, turn_left, turn_right, stop |
| Clean | Cleaner mode command. | Enumeration: clean_off, clean_on, clean_up |

## 2. Internal Data and Control Flows

| Data Name | Description | Format / Type | Flow Category | Source | Destination | DFD Level |
|---|---|---|---|---|---|---|
| Front Sensor Input | External front sample entering the system. | Boolean | Data | Front Sensor | Process 0, Process 1, Process 1.1 | 0-2 |
| Left Sensor Input | External left sample entering the system. | Boolean | Data | Left Sensor | Process 0, Process 1, Process 1.2 | 0-2 |
| Right Sensor Input | External right sample entering the system. | Boolean | Data | Right Sensor | Process 0, Process 1, Process 1.3 | 0-2 |
| Dust Sensor Input | External dust sample entering the system. | Boolean | Data | Dust Sensor | Process 0, Process 1, Process 1.4 | 0-2 |
| Direction | External movement command. | Enumeration | Data | Process 0, Process 3, Process 3.1 | Motor | 0-2 |
| Clean | External cleaner-mode command. | Enumeration | Data | Process 0, Process 3, Process 3.2 | Cleaner | 0-2 |
| Tick | Internal tick that causes the controller to evaluate one decision. | Event | Control | Process 2.1 | Process 2.2, Process 2.2.2 | 2-3 |
| Sensor Status | Composite latest sensor status written to D1. | Front Obstacle, Left Obstacle, Right Obstacle, Dust Detected | Data | Process 1 | D1 Latest Sensor Values | 1 |
| Latest Sensor Values | Composite latest sensor status read from D1. | Front Obstacle, Left Obstacle, Right Obstacle, Dust Detected | Data | D1 Latest Sensor Values | Process 2, Process 2.2, Process 2.2.1 | 1-3 |
| Front Obstacle | Stored latest front-obstacle value, and the same value passed to the controller. | Boolean | Data | Process 1.1, Process 2.2.1 | D1 Latest Sensor Values, Process 2.2.2 | 2-3 |
| Left Obstacle | Stored latest left-obstacle value, and the same value passed to the controller. | Boolean | Data | Process 1.2, Process 2.2.1 | D1 Latest Sensor Values, Process 2.2.2 | 2-3 |
| Right Obstacle | Stored latest right-obstacle value, and the same value passed to the controller. | Boolean | Data | Process 1.3, Process 2.2.1 | D1 Latest Sensor Values, Process 2.2.2 | 2-3 |
| Dust Detected | Stored latest dust value, and the same value passed to the controller. | Boolean | Data | Process 1.4, Process 2.2.1 | D1 Latest Sensor Values, Process 2.2.2 | 2-3 |
| Direction Command | Controller decision for the movement command. | Enumeration: forward, backward, turn_left, turn_right, stop | Control | Process 2, Process 2.2, Process 2.2.2 | Process 3, Process 3.1 | 1-3 |
| Clean Command | Controller decision for the cleaner mode. | Enumeration: clean_off, clean_on, clean_up | Control | Process 2, Process 2.2, Process 2.2.2 | Process 3, Process 3.2 | 1-3 |

### Composition preserved across levels

| Composite flow | Components |
|---|---|
| Sensor Status | Front Obstacle, Left Obstacle, Right Obstacle, Dust Detected |
| Latest Sensor Values | Front Obstacle, Left Obstacle, Right Obstacle, Dust Detected |

## 3. Data Store Definitions

| Data Store | Description | Stored Data | Format / Type | DFD Level |
|---|---|---|---|---|
| D1 Latest Sensor Values | Holds the latest boolean sample of each sensor. A new sample replaces only that sensor's value. The controller reads this store on each tick. | Front Obstacle, Left Obstacle, Right Obstacle, Dust Detected | Four booleans. Initial value of each field is false until the first sample arrives. | 1-3 |

## 4. Detailed Data Definitions

### Front Sensor Input
- Description: Interrupt-delivered front sensor sample.
- Format / Type: Boolean
- Possible Values: true (obstacle detected), false (obstacle not detected)
- Source: Front Sensor
- Destination: Process 0, Process 1, Process 1.1
- DFD Level: 0-2
- Requirement Reference: Front sensor input arrives by interrupt and is boolean. The controller uses the latest value.

### Left Sensor Input
- Description: Periodically delivered left sensor sample.
- Format / Type: Boolean
- Possible Values: true (obstacle detected), false (obstacle not detected)
- Source: Left Sensor
- Destination: Process 0, Process 1, Process 1.2
- DFD Level: 0-2
- Requirement Reference: Left sensor input arrives periodically and is boolean. The controller uses the latest value.

### Right Sensor Input
- Description: Periodically delivered right sensor sample.
- Format / Type: Boolean
- Possible Values: true (obstacle detected), false (obstacle not detected)
- Source: Right Sensor
- Destination: Process 0, Process 1, Process 1.3
- DFD Level: 0-2
- Requirement Reference: Right sensor input arrives periodically and is boolean. The controller uses the latest value.

### Dust Sensor Input
- Description: Periodically delivered dust sensor sample.
- Format / Type: Boolean
- Possible Values: true (dust detected), false (dust not detected)
- Source: Dust Sensor
- Destination: Process 0, Process 1, Process 1.4
- DFD Level: 0-2
- Requirement Reference: Dust sensor input arrives periodically and is boolean. Dust detection powers up cleaning.

### Tick
- Description: Internal timing event used to count a turn and a clean_up activation.
- Format / Type: Event
- Possible Values: Occurrence. The time between occurrences is TBD.
- Source: Process 2.1 Generate Tick
- Destination: Process 2.2, Process 2.2.2
- DFD Level: 2-3. Hidden inside Process 0 and Process 2 at earlier levels.
- Requirement Reference: The controller has an internal tick. A turn lasts 3 ticks. One clean_up activation lasts at most 3 ticks.

### Direction
- Description: Movement command delivered to the motor.
- Format / Type: Enumeration
- Possible Values: forward, backward, turn_left, turn_right, stop
- Source: Process 3.1 Command Direction
- Destination: Motor
- DFD Level: 0-2
- Requirement Reference: The RVC goes forward, turns left or right, moves backward, or stops in place.

### Clean
- Description: Cleaner mode delivered to the cleaner.
- Format / Type: Enumeration
- Possible Values: clean_off, clean_on, clean_up
- Source: Process 3.2 Command Cleaner Mode
- Destination: Cleaner
- DFD Level: 0-2
- Requirement Reference: The cleaner mode is clean_off, clean_on, or clean_up. clean_off is used only while moving backward or turning.

### Sensor Status
- Description: Level 1 name for the four latest sensor values written into D1.
- Format / Type: Composite of four booleans
- Possible Values: Each component is true or false
- Source: Process 1 Capture Sensor Values
- Destination: D1 Latest Sensor Values
- DFD Level: 1
- Requirement Reference: Sensor values keep arriving, and the controller uses the latest value of each sensor.

### Latest Sensor Values
- Description: Level 1–3 name for the four latest sensor values read from D1.
- Format / Type: Composite of four booleans
- Possible Values: Each component is true or false
- Source: D1 Latest Sensor Values
- Destination: Process 2, Process 2.2, Process 2.2.1
- DFD Level: 1-3
- Requirement Reference: The controller uses the latest value of each sensor.

### Front Obstacle
- Description: Latest front-obstacle value.
- Format / Type: Boolean
- Possible Values: true, false
- Source: Process 1.1 writes it. Process 2.2.1 reads it and passes it on.
- Destination: D1 Latest Sensor Values, then Process 2.2.2
- DFD Level: 2-3
- Requirement Reference: Front obstacle detection determines turning, backward movement, and the stop-during-clean_up rule.

### Left Obstacle
- Description: Latest left-obstacle value.
- Format / Type: Boolean
- Possible Values: true, false
- Source: Process 1.2 writes it. Process 2.2.1 reads it and passes it on.
- Destination: D1 Latest Sensor Values, then Process 2.2.2
- DFD Level: 2-3
- Requirement Reference: The left side is used for turn choice, left-turn priority, and the end of backward movement.

### Right Obstacle
- Description: Latest right-obstacle value.
- Format / Type: Boolean
- Possible Values: true, false
- Source: Process 1.3 writes it. Process 2.2.1 reads it and passes it on.
- Destination: D1 Latest Sensor Values, then Process 2.2.2
- DFD Level: 2-3
- Requirement Reference: The right side is used for turn choice and the end of backward movement.

### Dust Detected
- Description: Latest dust value.
- Format / Type: Boolean
- Possible Values: true, false
- Source: Process 1.4 writes it. Process 2.2.1 reads it and passes it on.
- Destination: D1 Latest Sensor Values, then Process 2.2.2
- DFD Level: 2-3
- Requirement Reference: Dust starts or ends clean_up according to the current motion.

### Direction Command
- Description: Control decision that Process 3.1 converts into the external Direction output.
- Format / Type: Enumeration
- Possible Values: forward, backward, turn_left, turn_right, stop
- Source: Process 2.2.2 Control Cleaning Behavior
- Destination: Process 3.1 Command Direction
- DFD Level: 1-3
- Requirement Reference: Same movement behaviors as Direction.

### Clean Command
- Description: Control decision that Process 3.2 converts into the external Clean output.
- Format / Type: Enumeration
- Possible Values: clean_off, clean_on, clean_up
- Source: Process 2.2.2 Control Cleaning Behavior
- Destination: Process 3.2 Command Cleaner Mode
- DFD Level: 1-3
- Requirement Reference: Same cleaner modes as Clean.

## 5. Controller state kept inside Process 2.2.2

These items are not flows and are not stored in D1. They are defined here so the FSM counters stay traceable.

| Name | Description | Format / Type | Possible Values |
|---|---|---|---|
| Controller state | Current FSM state | Enumeration | Forward, CleanUpForward, CleanUpStopped, TurnLeft, TurnRight, Backward, CleanUpBackwardPause |
| turn_count | Turn-command ticks already issued for the current turn | Integer | 0 before a turn, then 1, 2, or 3 |
| cleanup_count | clean_up-command ticks already issued for the current activation | Integer | 0 before an activation, then 1, 2, or 3 |
| dust_during_turn | Whether dust was detected on a tick of the current turn | Boolean | true, false |

## 6. Consistency check

| Check | Result |
|---|---|
| Every context and Level 0–3 flow has a definition | Defined in sections 1 and 2. |
| Every data store has a definition | D1 is defined in section 3. |
| Event List names are unchanged | Front Sensor Input, Left Sensor Input, Right Sensor Input, Dust Sensor Input, Tick, Direction, and Clean are unchanged. |
| Composite flows match their detailed flows | Sensor Status and Latest Sensor Values are the four stored booleans. |
| Conflicting types | None. Unspecified timing remains TBD. |
