# RVC 웹 시뮬레이터

`dist/rvc-simulator.html` 을 브라우저로 열면 바로 실행됩니다 (서버나 설치 불필요, 파일 하나).

## 구조
| 파일 | 역할 |
|---|---|
| `wasm/rvc_wasm.c` | 실제 C 제어기(`src/` 중 main.c 제외, 수정 없음)를 WebAssembly로 내보내는 다리 |
| `web/sim-core.js` | 환경 모델: 지도, 센서 샘플 생성, 이동, 먼지 제거, tick 스냅숏. 제어 결정은 하지 않음 |
| `web/scenarios.js` | 미리 정의된 시나리오 21개 (ST-01 ~ ST-21) |
| `web/index.html`, `style.css`, `simulator.js` | 화면 (지도 편집, 실행 제어, 상태 확인, 기록, 사람 판정) |
| `build.sh` | clang `--target=wasm32` 로 C를 컴파일하고 HTML 하나에 모두 넣음 |
| `run_scenarios.js` | 모든 시나리오를 브라우저 없이 실행, 요구사항 점검 표시 수와 네이티브 GCC 빌드와의 일치 여부 출력 |
| `gen_docs.js` | `artifacts/SystemTest/test_scenarios.md`, `requirement_traceability.md` 생성 |

## 실행 인터페이스 (명세의 initialize/step/get_state/reset 대응)
| 명세 | C/WASM | 실제 호출 |
|---|---|---|
| initialize(configuration), reset() | `rvc_reset()` | `RvcMain_Start` → SensorStore_Reset, GenerateTick_Reset, Controller_Init |
| step(sensor_inputs) | `rvc_step(front_arrived, front, left_arrived, left, right_arrived, right, dust_arrived, dust)` | `RvcMain_Cycle` → GenerateTick, CaptureSensorValues, Controller |
| get_state() | `rvc_get_state/turn_count/cleanup_count/dust_during_turn/direction_command/clean_command/motor_output/cleaner_output/sensors/tick` | `Controller_GetStatus`, `ReadLatestSensorValues`, 출력 포트에 실제로 전달된 값 |

1 시뮬레이션 tick = `rvc_step` 1회 = 구조도의 tick 1회(Main → GenerateTick → CaptureSensorValues → Controller → IssueActuatorCommands).

## tick 처리 순서
1. 현재 지도와 RVC 위치로 센서 값 계산 (Front: 바뀔 때만 인터럽트로 전달, Left/Right/Dust: 주기마다 전달)
2. `rvc_step` 으로 제어기 1회 실행
3. 출력 포트로 나간 Motor/Cleaner 값 읽기
4. 이동과 청소 효과 적용 (forward/backward 한 칸, 회전은 3번째 tick에 90도, clean_up tick마다 먼지 1 감소)
5. 스냅숏 기록 + 요구사항 점검(참고용) + 화면 갱신

## 환경 가정 (요구사항에 없는 부분, 시뮬레이터 전용)
- 시작 방향 기본값 N, 시작 위치 기본값 (4,4)
- 회전 각도 90도, 회전 완료 시점은 3번째 회전 tick
- 먼지 양 1~9 (clean_up tick마다 1 감소)
- 뒤 칸이 막혀 있으면 backward 명령이어도 제자리 (뒤쪽 센서는 요구사항에 없음)

## 다시 빌드
```bash
simulator/build.sh                                            # clang, wasm-ld, python3 필요
node simulator/run_scenarios.js simulator/dist/rvc.wasm --native build/rvc
node simulator/gen_docs.js
```

## 한계
- 판정은 사람이 합니다. 요구사항 점검 표시는 일부 불변식(PR-2, 9, 10, 11, 13)만 봅니다.
- 판정과 사용자 시나리오는 브라우저 저장소(localStorage)에 저장되므로 다른 컴퓨터와 공유하려면 내보내기(.md/.json)를 쓰세요.
- Step Back은 저장된 스냅숏을 보여줄 뿐 C 제어기를 되돌리지 않습니다.
