# Coverage 1회차: 단위 기능 테스트(functional)만 실행

- 목표: Statement, Branch, MC/DC 모두 **90% 이상** (사용자 선택)
- 도구: GCC 14 `--coverage -fcondition-coverage`, gcov-14 `-g`(MC/DC), gcovr 8.8
  (Statement = gcovr line, Branch = gcovr branch, MC/DC = gcovr condition / gcov `-g` condition outcomes)
- 실행: `scripts/coverage.sh unit_functional` (56개 테스트 모두 통과)

## 결과

| 파일 | Statement | Branch | MC/DC |
|---|---|---|---|
| actuator.c | 100% (15/15) | 75% (6/8) | 75% (6/8) |
| capture_sensor_values.c | 100% | 100% | 100% |
| controller.c | 97% (115/118) | 88% (88/100) | 88% (83/94) |
| generate_tick.c | 100% | 분기 없음 | 조건 없음 |
| main.c | 0% (0/64) | 0% (0/34) | 0% (0/30) |
| rvc_main.c | 100% | 100% | 100% |
| sensor_store.c | 100% | 분기 없음 | 조건 없음 |
| **전체** | **73% (189/256)** | **69% (110/158)** | **70% (105/148)** |

목표 미달 (세 지표 모두 90% 미만).

## 무엇을 보았나
1. `main.c`(stdin/stdout 호스트 어댑터)는 단위 테스트가 호출하지 않아 0%. 실제 실행 파일을 돌리는
   **시스템 테스트**로 덮어야 함.
2. `actuator.c:16, 23`: `output_port != NULL && output_port->send_xxx != NULL` 에서 두 번째 조건이
   false인 경우(포트는 있지만 콜백이 NULL)가 없음 → 구조 테스트 필요.
3. `controller.c` 에서 MC/DC 미충족 조건 11개. 대부분 **명세 의사코드를 그대로 옮긴 else-if 사슬의 중복 조건**:
   - `:52 in->front && in->left && !in->right` 의 `in->left` 는 앞 분기(`front && !left`)를 지나면 항상 true.
   - `:54 front && left && right` 의 `left`, `right` 도 앞 분기들 때문에 false가 될 수 없음.
   - `:56 !in->front && in->dust` 의 `!in->front`, `:81 in->left && !in->right` 의 `in->left`,
     `:110/:113/:118/:122/:124` 도 같은 이유.
   이 조건들은 어떤 입력으로도 독립적으로 결과를 바꿀 수 없으므로 테스트 추가로는 MC/DC를 채울 수 없음(infeasible).
4. `controller.c:167-170` `switch` 의 `default`: 상태가 7개 열거값 외의 값이 될 수 없어 도달 불가(방어 코드).

## 다음 회차 계획
- controller.c: 동작을 바꾸지 않고 중복 조건을 제거하는 리팩터링(TDD Refactor 단계).
  기능 테스트 35개가 그대로 통과하는지로 동작 보존을 확인.
- `tests/unit/structural/`: 콜백 NULL 포트, 남은 분기/조건을 겨냥한 화이트박스 테스트 추가.
- `tests/system/`: 실행 파일 `rvc`를 시나리오 입력으로 구동하는 시스템 테스트 추가(main.c 커버).
