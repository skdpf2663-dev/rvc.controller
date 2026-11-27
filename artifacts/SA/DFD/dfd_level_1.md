# DFD Level 1

Process 0 is decomposed into sensor capture, cleaning control, and actuator output. Process 2 is the control process. A dashed circle marks a control process.

```mermaid
flowchart LR
    FrontSensor["Front Sensor"]
    LeftSensor["Left Sensor"]
    RightSensor["Right Sensor"]
    DustSensor["Dust Sensor"]
    Motor["Motor"]
    Cleaner["Cleaner"]

    P1(("1 Capture Sensor Values"))
    P2(("2 Control Automatic Cleaning"))
    P3(("3 Issue Actuator Commands"))
    D1[["D1 Latest Sensor Values"]]

    style P2 stroke-dasharray: 5 5

    FrontSensor -->|Front Sensor Input| P1
    LeftSensor -->|Left Sensor Input| P1
    RightSensor -->|Right Sensor Input| P1
    DustSensor -->|Dust Sensor Input| P1
    P1 -->|Sensor Status| D1
    D1 -->|Latest Sensor Values| P2
    P2 -.->|Direction Command| P3
    P2 -.->|Clean Command| P3
    P3 -->|Direction| Motor
    P3 -->|Clean| Cleaner
```

## Processes and stores

| ID | Name | Kind |
|---|---|---|
| 1 | Capture Sensor Values | Data process |
| 2 | Control Automatic Cleaning | Control process |
| 3 | Issue Actuator Commands | Data process |
| D1 | Latest Sensor Values | Data store |

## Balancing with Level 0

| Level 0 flow | Level 1 preservation |
|---|---|
| Front Sensor Input | Front Sensor → Process 1 |
| Left Sensor Input | Left Sensor → Process 1 |
| Right Sensor Input | Right Sensor → Process 1 |
| Dust Sensor Input | Dust Sensor → Process 1 |
| Direction | Process 3 → Motor |
| Clean | Process 3 → Cleaner |

Internal flows added at Level 1:

| Flow | Category | Path |
|---|---|---|
| Sensor Status | Data | Process 1 → D1 |
| Latest Sensor Values | Data | D1 → Process 2 |
| Direction Command | Control | Process 2 → Process 3 |
| Clean Command | Control | Process 2 → Process 3 |

Tick is still inside Process 2. It becomes visible when Process 2 is decomposed.
