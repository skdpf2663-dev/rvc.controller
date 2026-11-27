# Entire 체크포인트 목록

커밋할 때마다 Entire CLI(v0.13.4)가 그 사이의 프롬프트, 대화(transcript), 수정 파일을 `refs/entire/checkpoints/` 에 저장했습니다.
클라우드 환경의 git 프록시가 `refs/entire/*` 푸시를 막아서, 체크포인트 원본(대화 전문 포함)은 GitHub에 올라가지 않았습니다.
대신 각 체크포인트의 메타데이터(시간, 연결된 커밋, 수정한 파일, 해당 구간의 출력 토큰과 API 호출 수)를 여기에 남깁니다.
`ai-process-log.md` 가 단계별로 무엇을 보고 무엇을 결정했는지 사람이 읽을 수 있게 정리한 기록입니다.

| 시간 (UTC) | 체크포인트 | 커밋 | 수정 파일 | 출력 토큰 | API 호출 |
|---|---|---|---|---|---|
| 2026-11-27 02:56:21 | `01M8DEXAYHG8GYRBQWNKR61SZ7` | c02af59 docs: record environment check and coverage target (n=90%) | docs/ai-process-log.md | 978 | 1 |
| 2026-11-27 03:11:38 | `01M8DFSAKTRKGTJE7W751S11D4` | 670ee16 docs: import approved SA/SD artifacts and requirements as implementation input | artifacts/SA/DFD/dfd_level_0.md, artifacts/SA/DFD/dfd_level_1.md, artifacts/SA/DFD/dfd_level_2.md, artifacts/SA/DFD/dfd_level_3.md, artifacts/SA/DFD/dfd_leve... | 6812 | 16 |
| 2026-11-27 03:14:20 | `01M8DFY938CQMGVPD00WXAK9CR` | 5ab2a19 test(unit-functional): add spec-derived unit tests and module stubs (TDD red) | .gitignore, CMakeLists.txt, include/actuator.h, include/capture_sensor_values.h, include/controller.h, include/generate_tick.h, include/rvc_main.h, include/r... | 20489 | 7 |
| 2026-11-27 03:14:41 | `01M8DFYXGV3GN91QGH6XQK82NB` | 17fcb5c impl: sensor store, CaptureSensorValues, GenerateTick, actuator modules (TDD green for SA 1, 2.1, 3) | src/actuator.c, src/capture_sensor_values.c, src/generate_tick.c, src/sensor_store.c | 1905 | 3 |
| 2026-11-27 03:15:05 | `01M8DFZNAZNE9K4NDJZX9KFGZ4` | 6cedc9e impl: Controller FSM from Process 2.2.2 pseudocode and Level 4 transition table | src/controller.c | 2873 | 2 |
| 2026-11-27 03:15:40 | `01M8DG0PNR91PHDGXN49EPEM6G` | cc5702f impl: Main tick cycle and stdin/stdout host adapter (main.c) | src/main.c, src/rvc_main.c | 3657 | 2 |
| 2026-11-27 03:16:23 | `01M8DG21EX563T29TQJNWT1HE1` | ec1090b coverage: iteration 1 with unit functional tests only (73/69/70%) | docs/ai-process-log.md, reports/coverage-1.md, scripts/coverage.sh, scripts/mcdc_gaps.py | 3820 | 3 |
| 2026-11-27 03:16:59 | `01M8DG34216BKAXD01X2T2MXS8` | df32914 impl: refactor Controller to remove redundant else-if conditions (TDD refactor) | src/controller.c | 2814 | 3 |
| 2026-11-27 03:18:14 | `01M8DG5DNQRT92SR0Y99HAJAMM` | fa5b972 test(unit-structural): white-box MC/DC pair tests for controller, actuator, capture, main loop | tests/unit/structural/test_controller_structural.cpp, tests/unit/structural/test_io_structural.cpp | 7562 | 3 |
| 2026-11-27 03:18:17 | `01M8DG5G1NX24M6XAY2X8Y97DM` | efdcd1b coverage: iteration 2 after refactor and structural tests (73/70/72%, controller MC/DC 100%) | reports/coverage-2.md | 0 | 0 |
| 2026-11-27 03:20:43 | `01M8DG9YGHY5BSTGR2WJCH1KA9` | b7e3325 test(system): 14 grid-world scenarios through Main and 7 tests of the rvc executable | CMakeLists.txt, tests/system/grid_world.h, tests/system/test_system_cli.cpp, tests/system/test_system_scenarios.cpp | 16612 | 5 |
| 2026-11-27 03:21:11 | `01M8DGAT3EBSFBV6HAW5W70PA2` | 37f109a coverage: iteration 3 with system tests reaches 98.8/99.2/100% (target 90%) | reports/coverage-3.md, reports/test-execution.txt | 2186 | 3 |
| 2026-11-27 03:22:16 | `01M8DGCSW9BDMASRHQ0N51T0QH` | 79830b2 docs: test classification report, SD-to-code traceability, README and process log | README.md, artifacts/Implementation/module_traceability.md, artifacts/Implementation/test_report.md, docs/ai-process-log.md, scripts/test_catalog.py | 6356 | 7 |

이 파일을 추가한 커밋의 체크포인트는 커밋 후에 생기므로 목록에 없습니다. `git log` 의 `Entire-Checkpoint:` 트레일러로 확인할 수 있습니다.
