# Coverage 2회차: 리팩터링 + 단위 구조 테스트(structural) 추가

- 실행: `scripts/coverage.sh unit` (functional 56 + structural 20 = 76개, 모두 통과)

## 1회차 이후 한 일
1. **controller.c 리팩터링** (`df32914`): 명세 의사코드의 else-if 사슬에 있던 중복 조건(앞 분기에서 이미
   배제된 조건)을 제거. 장애물 회피를 `avoid_front_obstacle()`로 묶음. 동작 변화 없음은 기능 테스트 56개
   통과로 확인. 이 단계에서 기능 테스트만으로 controller.c MC/DC가 88% → **100%** 가 됨.
   (중복 조건은 테스트를 아무리 추가해도 MC/DC를 채울 수 없는 infeasible 조건이었음)
2. **구조 테스트 20개** (`tests/unit/structural/`): 코드의 결정문마다 MC/DC 독립 쌍(한 조건만 바꿔서 결과가
   바뀌는 두 실행)을 명시한 화이트박스 테스트.
   - `actuator.c:16, 23`: 포트는 있지만 콜백이 NULL인 경우 → 1회차의 미충족 조건 2개 해결.
   - `rvc_main.c` 루프 조건 `executed < max_ticks && source(...)` 의 세 경우.
   - `capture_sensor_values.c` 각 도착 여부 분기 양방향.
   - `controller.c` 결정문 전부(삼항 연산자 포함).

## 결과

| 파일 | Statement | Branch | MC/DC |
|---|---|---|---|
| actuator.c | 100% | 100% (8/8) | 100% (8/8) |
| capture_sensor_values.c | 100% | 100% | 100% |
| controller.c | 97% (107/110) | 98% (59/60) | 100% (54/54) |
| generate_tick.c | 100% | 분기 없음 | 조건 없음 |
| main.c | 0% (0/64) | 0% (0/34) | 0% (0/30) |
| rvc_main.c | 100% | 100% | 100% |
| sensor_store.c | 100% | 분기 없음 | 조건 없음 |
| **전체** | **73% (181/248)** | **70% (83/118)** | **72% (78/108)** |

## 무엇을 보았나
- main.c를 뺀 제어기 코드는 Statement 98.9%, Branch 98.8%, MC/DC 100%.
- 남은 controller.c 미커버: `switch` 의 `default` (170-173행, 분기 1개). 상태 변수가 파일 내부(static)에서
  7개 열거값으로만 대입되므로 어떤 입력으로도 도달 불가(방어 코드). 제거하지 않고 infeasible로 기록.
- 전체 수치를 막는 것은 main.c(호스트 어댑터) 0%. 단위 테스트 범위가 아니므로 다음 회차에
  실행 파일을 구동하는 **시스템 테스트**로 덮음.
