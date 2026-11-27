# Test Report: 테스트 분류와 실행 결과

- 실행: `cmake -S . -B build -DCMAKE_C_COMPILER=gcc-14 -DCMAKE_CXX_COMPILER=g++-14 && cmake --build build && (cd build && ctest)`
- 결과: **97 / 97 통과** (실행 로그 `reports/test-execution.txt`)
- 커버리지: Statement 98.8%, Branch 99.2%, MC/DC 100% (`reports/coverage-3.md`)

## 분류 기준

| 종류 | 폴더 | 무엇을 보고 만들었나 | 개수 |
|---|---|---|---|
| Unit - Functional | `tests/unit/functional/` | 코드를 보지 않고 SA/SD 산출물(FSM 전이표 행, Process Spec, 구조도 인터페이스, PR-x)만으로 입력과 기대 출력을 정함 | 56 |
| Unit - Structural | `tests/unit/structural/` | 커버리지 1회차 결과와 소스 코드의 결정문을 보고, 결정문마다 MC/DC 독립 쌍과 NULL 포인터 분기를 겨냥함 | 20 |
| System | `tests/system/` | 전체 시스템(Main → 모든 모듈, 또는 빌드된 `rvc` 실행 파일)을 10x10 격자 시나리오/입력 스크립트로 구동하고 요구사항 수준 동작을 검사 | 21 |

`scripts/test_catalog.py` 로 아래 목록을 생성함.

## Unit - Functional (black-box, from SA/SD spec): 56 tests

| File | Test | Basis / requirement |
|---|---|---|
| test_controller_fsm.cpp | R00_InitialStateIsForwardCleanOn | FSM-R00: begin automatic cleaning -> Forward, forward, clean_on. |
| test_controller_fsm.cpp | R01_ForwardFrontLeftClearTurnsLeft | FSM-R01: Front and not Left -> TurnLeft (PR-3, PR-9 left priority). |
| test_controller_fsm.cpp | R01_ForwardFrontWithDustRecordsDustDuringTurn | FSM-R01 with dust: dust_during_turn receives the current Dust value. |
| test_controller_fsm.cpp | R02_ForwardFrontLeftBlockedTurnsRight | FSM-R02: Front and Left and not Right -> TurnRight. |
| test_controller_fsm.cpp | R03_ForwardAllBlockedGoesBackward | FSM-R03: Front and Left and Right -> Backward (PR-4). |
| test_controller_fsm.cpp | R04_ForwardDustPowersUpCleaning | FSM-R04: not Front and Dust -> CleanUpForward (PR-5). |
| test_controller_fsm.cpp | R05_ForwardClearKeepsGoing | FSM-R05: not Front and not Dust -> stay Forward (PR-2). |
| test_controller_fsm.cpp | R01_FrontHasPriorityOverDustInForward | Interpretation 5: front obstacle is handled before dust in Forward. |
| test_controller_fsm.cpp | R06_R08_TurnLeftLastsThreeTicksThenForward | FSM-R06 / PR-10: a turn lasts 3 ticks, then forward with clean_on (FSM-R08). |
| test_controller_fsm.cpp | R07_DustDuringTurnLeftFinishesTurnThenCleanUp | FSM-R07 / PR-17: dust during a turn -> the turn finishes, then CleanUpForward. |
| test_controller_fsm.cpp | R07_DustOnCompletionTickSelectsCleanUp | Interpretation 8: dust seen only on the completion tick still selects clean_up. |
| test_controller_fsm.cpp | R09_R11_TurnRightLastsThreeTicksThenForward | FSM-R09/R11: TurnRight lasts 3 ticks and returns to Forward. |
| test_controller_fsm.cpp | R10_DustDuringTurnRightThenCleanUp | FSM-R10: dust during TurnRight -> CleanUpForward after the turn. |
| test_controller_fsm.cpp | R12_BackwardLeftClearTurnsLeft | FSM-R12 / PR-14: Backward until left clears -> TurnLeft. |
| test_controller_fsm.cpp | R13_BackwardRightClearTurnsRight | FSM-R13: Backward with only right clear -> TurnRight. |
| test_controller_fsm.cpp | R14_BackwardDustStopsForCleanUp | FSM-R14 / PR-16: dust while moving backward -> stop and clean_up. |
| test_controller_fsm.cpp | R15_BackwardContinuesWhileSidesBlocked | FSM-R15 / PR-14: Backward continues while both sides are blocked. |
| test_controller_fsm.cpp | R16_R17_BackwardPauseIsOneActivationOfThreeTicks | FSM-R16/R17 / PR-11, PR-16: one activation of at most 3 ticks, then Backward again. |
| test_controller_fsm.cpp | R17_BackwardPauseEndsWhenDustGone | FSM-R17: the backward pause ends early when dust disappears. |
| test_controller_fsm.cpp | R14_LaterBackwardTickStartsAnotherActivation | Interpretation 10: a later Backward tick can start another single activation. |
| test_controller_fsm.cpp | R18_CleanUpForwardFrontStopsSameActivation | FSM-R18 / PR-15: front obstacle during clean_up -> stop, same activation continues. |
| test_controller_fsm.cpp | R19_CleanUpForwardContinues | FSM-R19: activation continues with no front obstacle. |
| test_controller_fsm.cpp | R20_CleanUpForwardEndsDustRemainsFrontStops | FSM-R20: activation ended (3 ticks), dust remains, front blocked -> new activation stopped. |
| test_controller_fsm.cpp | R21_CleanUpForwardRestartsActivationAfterThreeTicks | FSM-R21 / PR-11: clean_up never stays on longer than 3 ticks in one activation. |
| test_controller_fsm.cpp | R22_CleanUpForwardEndsThenTurnLeft | FSM-R22: activation ended, no dust, front, left clear -> TurnLeft. |
| test_controller_fsm.cpp | R23_CleanUpForwardEndsThenTurnRight | FSM-R23: activation ended, no dust, front, left blocked, right clear -> TurnRight. |
| test_controller_fsm.cpp | R24_CleanUpForwardEndsThenBackward | FSM-R24: activation ended, no dust, all blocked -> Backward. |
| test_controller_fsm.cpp | R25_CleanUpForwardEndsEarlyThenForward | FSM-R25 / PR-11: activation ends sooner when dust is no longer detected. |
| test_controller_fsm.cpp | R26_CleanUpStoppedContinues | FSM-R26: activation continues while stopped (even if the front clears, interp. 12). |
| test_controller_fsm.cpp | R27_CleanUpStoppedNextActivation | FSM-R27 / PR-15: activation ends with dust remaining -> next activation while stopped. |
| test_controller_fsm.cpp | R28_CleanUpStoppedThenTurnLeft | FSM-R28: dust gone, front, left clear -> TurnLeft. |
| test_controller_fsm.cpp | R29_CleanUpStoppedThenTurnRight | FSM-R29: dust gone, front, left blocked, right clear -> TurnRight. |
| test_controller_fsm.cpp | R30_CleanUpStoppedThenBackward | FSM-R30: dust gone, all blocked -> Backward. |
| test_controller_fsm.cpp | R31_CleanUpStoppedThenForward | FSM-R31: dust gone, front clear -> Forward. |
| test_controller_fsm.cpp | PR13_CleanOffOnlyWhileTurningOrBackward | PR-13: clean_off is issued only in TurnLeft, TurnRight, and Backward. |
| test_main.cpp | StartBeginsForwardCleanOn | UF-M01: starting automatic cleaning issues forward and clean_on and clears old state. |
| test_main.cpp | CycleCapturesBeforeDeciding | UF-M02: one cycle = one Tick, samples applied before the decision, one controller execution. |
| test_main.cpp | CycleWithoutSamplesStillTicks | UF-M03: a cycle with no serviced sample still runs the controller on the latest values. |
| test_main.cpp | RunStopsWhenInputEnds | UF-M04: Run executes one tick per input step and stops when input ends. |
| test_main.cpp | RunStopsAtTickLimit | UF-M05: Run never exceeds the tick limit. |
| test_main.cpp | RunWithoutSourceDoesNothing | UF-M06: Run without a source executes no tick (only the start commands). |
| test_sensor_capture.cpp | InitialValuesAreFalse | UF-S01 (D1, FSM interpretation 2): every value is false until its first sample. |
| test_sensor_capture.cpp | UpdateFrontChangesOnlyFront | UF-S02 (Process 1.1, PR-8): a front sample replaces only the front value. |
| test_sensor_capture.cpp | UpdateLeftChangesOnlyLeft | UF-S03 (Process 1.2): a left sample replaces only the left value. |
| test_sensor_capture.cpp | UpdateRightChangesOnlyRight | UF-S04 (Process 1.3): a right sample replaces only the right value. |
| test_sensor_capture.cpp | UpdateDustChangesOnlyDust | UF-S05 (Process 1.4): a dust sample replaces only the dust value. |
| test_sensor_capture.cpp | LatestSampleWins | UF-S06 (PR-8): the controller uses the latest value of each sensor. |
| test_sensor_capture.cpp | ReadDoesNotModifyStore | UF-S07 (Process 2.2.1): reading does not change the stored values. |
| test_sensor_capture.cpp | CaptureAppliesOnlyArrivedSamples | UF-S08 (Process 1): only sensors whose sample arrived are updated. |
| test_sensor_capture.cpp | CaptureAppliesAllFourSamples | UF-S09 (Process 1): all four samples arriving together update all four values. |
| test_sensor_capture.cpp | CaptureWithNothingArrivedKeepsValues | UF-S10 (Process 1): servicing with no arrived sample keeps every value. |
| test_tick_actuator.cpp | EachCallProducesOneTick | UF-T01 (Process 2.1): each GenerateTick call produces exactly one Tick. |
| test_tick_actuator.cpp | CommandDirectionSendsDirection | UF-A01 (Process 3.1): CommandDirection sends the received Direction Command unchanged. |
| test_tick_actuator.cpp | CommandCleanerModeSendsClean | UF-A02 (Process 3.2): CommandCleanerMode sends the received Clean Command unchanged. |
| test_tick_actuator.cpp | IssueSendsDirectionThenClean | UF-A03 (Process 3, structure chart order): both commands, direction first. |
| test_tick_actuator.cpp | NoPortDropsCommands | UF-A04 (c-coding rule: check pointers): with no port connected nothing is sent and nothing crashes. |

## Unit - Structural (white-box, from code decisions): 20 tests

| File | Test | Basis / requirement |
|---|---|---|
| test_controller_structural.cpp | DustDuringTurnFlag_TurnRightDustVsNoDust | Pair A (first condition): TurnLeft+dust sets the flag; Forward+dust does not start a turn flag. Pair B (second condition): TurnRight+dust sets; TurnRight+no dust keeps false. |
| test_controller_structural.cpp | DustDuringTurnFlag_NotSetOutsideTurnStates | - |
| test_controller_structural.cpp | StepForward_FrontTrueVsFalse | - |
| test_controller_structural.cpp | StepForward_DustTrueVsFalse | - |
| test_controller_structural.cpp | Avoid_FromForward_AllThreeOutcomes | - |
| test_controller_structural.cpp | Avoid_FromCleanUp_PassesFalseDustFlag | - |
| test_controller_structural.cpp | StartTurn_LeftAndRightCommands | - |
| test_controller_structural.cpp | StartCleanup_ForwardAndStopCommands | - |
| test_controller_structural.cpp | StepTurn_CountBelowLimitVsAtLimit | - |
| test_controller_structural.cpp | StepTurn_DustFlagTrueVsFalseAtLimit | - |
| test_controller_structural.cpp | StepBackward_AllFourOutcomes | - |
| test_controller_structural.cpp | BackwardPause_McdcPairs | MC/DC: (count<3, dust) -> stay; (count==3, dust) -> leave [cond 1]; (count<3, !dust) -> leave [cond 2]. |
| test_controller_structural.cpp | Cleanup_ActivationEnded_McdcPairs | - |
| test_controller_structural.cpp | Cleanup_Continuing_IsForwardAndFront_McdcPairs | - |
| test_controller_structural.cpp | Cleanup_NextActivation_McdcPairs | - |
| test_controller_structural.cpp | Cleanup_EndedNoDust_FrontVsNoFront | - |
| test_io_structural.cpp | CommandDirection_McdcPairs | actuator.c CommandDirection: port != NULL && port->send_direction != NULL MC/DC: (NULL, -) false [cond 1]; (port, NULL cb) false [cond 2]; (port, cb) true. |
| test_io_structural.cpp | CommandCleanerMode_McdcPairs | actuator.c CommandCleanerMode: port != NULL && port->send_clean != NULL |
| test_io_structural.cpp | Capture_EachArrivalDecisionBothWays | capture_sensor_values.c: each "arrived" decision taken true and false independently. |
| test_io_structural.cpp | Run_LoopCondition_McdcPairs | rvc_main.c RvcMain_Run loop: executed < max_ticks && source(...) MC/DC: (max 0) -> stop before calling the source [cond 1]; (max 5, source false) -> stop [cond 2]; (max 2, source true) -> run. |

## System (whole-system scenarios): 21 tests

| File | Test | Basis / requirement |
|---|---|---|
| test_system_cli.cpp | CLI01_ObstacleAvoidanceSequence | SYS-CLI-01 (PR-2, PR-3, PR-9, PR-10, PR-13): start forward/clean_on, front obstacle |
| test_system_cli.cpp | CLI02_CleanUpStopThenTurnRight | SYS-CLI-02 (PR-5, PR-15, PR-11, PR-8): dust -> clean_up; front interrupt during clean_up -> stop; dust gone -> right turn (left blocked). Comments/blank lines skipped. |
| test_system_cli.cpp | CLI03_BackwardDustPauseThenTurn | SYS-CLI-03 (PR-4, PR-16, PR-14): all blocked -> backward; dust -> stop + clean_up; one activation -> backward; right side clears -> turn right. |
| test_system_cli.cpp | CLI04_TickLimitArgument | SYS-CLI-04: the optional tick limit stops the run. |
| test_system_cli.cpp | CLI05_EmptyInput | SYS-CLI-05: empty input -> only the start line. |
| test_system_cli.cpp | CLI06_InvalidInputRejected | SYS-CLI-06: invalid input lines are rejected with exit code 2. |
| test_system_cli.cpp | CLI07_InvalidArgument | SYS-CLI-07: an invalid tick-limit argument prints usage and exits with 2. |
| test_system_scenarios.cpp | SYS01_OpenRoomStraightThenLeftTurnAtWall | SYS-01 (PR-2, PR-3, PR-9, PR-10, PR-13): open room, straight to the wall, left turn has priority, turn lasts 3 ticks with clean_off, then forward again. |
| test_system_scenarios.cpp | SYS02_LeftBlockedTurnsRight | SYS-02 (PR-3, PR-9, PR-10, PR-13): left side blocked -> turn right, then forward. |
| test_system_scenarios.cpp | SYS03_DeadEndBacksUpThenTurnsLeft | SYS-03 (PR-4, PR-14, PR-9, PR-13): dead-end corridor -> backward until a side is clear, then a left turn (both sides clear), then forward. |
| test_system_scenarios.cpp | SYS04_DustStripRestartsActivationEveryThreeTicks | SYS-04 (PR-5, PR-11, PR-2): long dust strip -> clean_up activations of at most 3 ticks restart while dust remains, then clean_on when the dust ends. |
| test_system_scenarios.cpp | SYS05_ShortDustEndsActivationEarly | SYS-05 (PR-5, PR-11): a single dusty cell -> clean_up ends sooner when dust is gone. |
| test_system_scenarios.cpp | SYS06_FrontObstacleDuringCleanUpStopsUntilDustGone | SYS-06 (PR-15, PR-11, PR-3, PR-13): front obstacle during clean_up -> stop in place and keep cleaning until dust is gone (new activations while stopped), no turn or backward before that, then turn left. |
| test_system_scenarios.cpp | SYS07_DustWhileBackingUpPausesForOneActivation | SYS-07 (PR-16, PR-14, PR-4, PR-11): dust while moving backward -> stop, one clean_up activation, then backward again, then a turn when a side clears. |
| test_system_scenarios.cpp | SYS08_DustDuringTurnFinishesTurnThenCleansUp | SYS-08 (PR-17, PR-10, PR-3, PR-5): dust found where a turn starts -> the turn finishes (3 ticks, clean_off) and the RVC then goes forward in clean_up. |
| test_system_scenarios.cpp | SYS09_CornerStartTurnsRightImmediately | SYS-09 (PR-3, PR-9, PR-10): corner start facing a wall with the left blocked |
| test_system_scenarios.cpp | SYS10_EnclosedKeepsBackingUp | SYS-10 (PR-4, PR-14, PR-13): fully enclosed -> keeps backing up (clean_off) because no side ever becomes clear. |
| test_system_scenarios.cpp | SYS11_InterruptFrontEqualsEveryTickFront | SYS-11 (PR-8): the front sensor arrives by interrupt (only on change); the controller uses latest values, so the run equals one where front arrives every tick. |
| test_system_scenarios.cpp | SYS12_ClutteredRoomHundredTicks | SYS-12 (PR-2, PR-3, PR-5, PR-10, PR-11, PR-13): 100-tick run in a cluttered room with dust; every requirement invariant holds and cleaning reaches several states. |
| test_system_scenarios.cpp | SYS13_SlowPeriodicSensorsKeepInvariants | SYS-13 (PR-8, PR-13, PR-11): periodic side and dust sensors sampled every 2 ticks; the controller keeps using the latest values and all invariants still hold. |
| test_system_scenarios.cpp | SYS14_TickLimitIsHundred | SYS-14 (simulator rule): a scenario never runs past 100 ticks. |

