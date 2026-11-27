# Event List

| Input/Output Event | Description |
|---|---|
| Front Sensor Input | Interrupt input from the front sensor. Boolean value. The controller keeps the latest value and uses it to decide whether a front obstacle is present. |
| Left Sensor Input | Periodic input from the left sensor. Boolean value. The controller keeps the latest value and uses it to decide whether the left side is blocked. |
| Right Sensor Input | Periodic input from the right sensor. Boolean value. The controller keeps the latest value and uses it to decide whether the right side is blocked. |
| Dust Sensor Input | Periodic input from the dust sensor. Boolean value. The controller keeps the latest value and uses it to decide whether dust is present. |
| Tick | Internal temporal event of the controller. Turn duration and each clean_up activation are counted in ticks. Tick does not cross the system boundary. |
| Direction | Movement command sent to the motor. The supported commands are forward, backward, turn_left, turn_right, and stop. |
| Clean | Cleaner-mode command sent to the cleaner. The supported modes are clean_off, clean_on, and clean_up. |

## Traceability

| Input/Output Event | Requirement basis |
|---|---|
| Front Sensor Input | Front sensor input arrives by interrupt. Every sensor value is boolean. The controller uses the latest value. |
| Left Sensor Input | Left sensor input arrives periodically. Every sensor value is boolean. The controller uses the latest value. |
| Right Sensor Input | Right sensor input arrives periodically. Every sensor value is boolean. The controller uses the latest value. |
| Dust Sensor Input | Dust sensor input arrives periodically. Every sensor value is boolean. The controller uses the latest value. Dust detection powers up cleaning. |
| Tick | The controller has an internal tick. A turn lasts 3 ticks. One clean_up activation lasts at most 3 ticks. |
| Direction | The RVC goes forward, turns left or right, moves backward, or stops in place. |
| Clean | The cleaner mode is clean_off, clean_on, or clean_up. clean_off is used only while moving backward or turning. |

## Boundary note

Front Sensor Input, Left Sensor Input, Right Sensor Input, Dust Sensor Input, Direction, and Clean cross the system boundary. Tick is an internal temporal event and is not an external context flow.
