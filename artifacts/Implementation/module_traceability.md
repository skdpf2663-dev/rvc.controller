# SD-to-Implementation Traceability

Structure Chart(`artifacts/SD/structure_chart.md`)의 12개 모듈과 C 구현의 대응.

| SA Process | SD Module | C 함수 | 파일 | 호출하는 모듈 (구조도와 동일) |
|---|---|---|---|---|
| 0 / 2 | Main | `RvcMain_Start`, `RvcMain_Cycle`, `RvcMain_Run` (+ C `main()` 어댑터) | `src/rvc_main.c`, `src/main.c` | GenerateTick, CaptureSensorValues, Controller |
| 2.1 | GenerateTick | `GenerateTick` | `src/generate_tick.c` | 없음 |
| 1 | CaptureSensorValues | `CaptureSensorValues` | `src/capture_sensor_values.c` | Update*Sensor (도착한 샘플만) |
| 1.1 | UpdateFrontSensor | `UpdateFrontSensor` | `src/sensor_store.c` | 없음 |
| 1.2 | UpdateLeftSensor | `UpdateLeftSensor` | `src/sensor_store.c` | 없음 |
| 1.3 | UpdateRightSensor | `UpdateRightSensor` | `src/sensor_store.c` | 없음 |
| 1.4 | UpdateDustSensor | `UpdateDustSensor` | `src/sensor_store.c` | 없음 |
| 2.2.1 | ReadLatestSensorValues | `ReadLatestSensorValues` | `src/sensor_store.c` | 없음 |
| 2.2.2 | Controller | `Controller` (+ `Controller_Init`) | `src/controller.c` | ReadLatestSensorValues, IssueActuatorCommands |
| 3 | IssueActuatorCommands | `IssueActuatorCommands` | `src/actuator.c` | CommandDirection, CommandCleanerMode (이 순서) |
| 3.1 | CommandDirection | `CommandDirection` | `src/actuator.c` | 없음 |
| 3.2 | CommandCleanerMode | `CommandCleanerMode` | `src/actuator.c` | 없음 |
| D1 | (숨겨진 저장소) | `static SensorValues latest` | `src/sensor_store.c` | 위 5개 함수만 접근 |

틱 실행 순서(구조도 3절)는 `RvcMain_Cycle`: GenerateTick → CaptureSensorValues → Controller
(→ ReadLatestSensorValues → FSM 전이 1회 → IssueActuatorCommands → CommandDirection → CommandCleanerMode).

## 구조도와 다른 점 (설계 편차)

명세가 하드웨어를 범위 밖(PR-6)으로 두었기 때문에 생긴 인터페이스 보완이며, 새 동작은 추가하지 않았습니다.
원래 프로젝트 규칙상 편차는 사람의 승인이 필요하므로 PR 리뷰에서 확인을 요청합니다.

1. `CaptureSensorValues(const SensorSamples *)`: 구조도는 입력 None. 어떤 샘플이 도착했는지를 알려줄
   하드웨어 계층이 없어서, 도착 여부와 값을 구조체 인자로 받음. NULL이면 도착 샘플 없음.
2. `CommandDirection` / `CommandCleanerMode`: 모터·청소기 출력을 `ActuatorPort` 콜백 2개로 보냄
   (`Actuator_SetPort`). 포트나 콜백이 NULL이면 명령을 버림(C 코딩 규칙: 포인터 확인).
3. 초기화 함수 `SensorStore_Reset`, `GenerateTick_Reset`, `Controller_Init`: 데이터 사전의 초기값
   (센서 false, 시작 상태 Forward + forward/clean_on)을 만들기 위함. `Controller_Init`은
   "begin automatic cleaning / forward, clean_on" 전이에 따라 시작 명령을 한 번 내보냄.
4. `Controller_GetStatus`, `GenerateTick_Count`: 상태 확인(테스트, 시뮬레이터)용 읽기 전용 접근자.
   FSM 상태를 다른 모듈이 바꿀 수는 없음.
5. `src/main.c`: 구조도의 Main은 `rvc_main.c` 의 함수들이고, C `main()` 은 stdin/stdout으로 센서 입력과
   명령 출력을 연결하는 호스트 어댑터(시스템 테스트·시뮬레이터 연결용).
6. Controller 내부: Process 2.2.2 의사코드의 else-if 사슬에서 앞 분기가 이미 배제한 조건을 제거하고,
   장애물 회피를 `avoid_front_obstacle()` 로 묶음. 전이표의 모든 행과 동작이 같음(기능 테스트 35개로 확인).

## FSM 기록된 해석(dfd_level_4_fsm.md "Recorded interpretations") 처리
1~12번 해석을 그대로 구현했고, 각각 기능 테스트 이름/주석에 `Interpretation n` 으로 표시했습니다.
13번(명령 이름/인코딩)은 C 열거형 `Direction`, `CleanMode` 로, 14번(틱 길이, 센서 주기)은 미정 그대로
(GenerateTick 호출 1번 = 1 tick)로 두었습니다.
