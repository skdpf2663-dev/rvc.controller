# RVC System Test Results

## Human evaluation

The system test oracle is the human evaluator (systemtesting-simulator rule: do not assign PASS/FAIL automatically).
Open `simulator/dist/rvc-simulator.html`, run each scenario, record PASS / FAIL / INCONCLUSIVE in the
"사람 판정" panel, and export "판정 결과 (.md)" to replace the table below.

| Scenario | Human Result | Evaluator | Observations |
|---|---|---|---|
| ST-01 Open room: straight to the wall, then left turn | Pending | | |
| ST-02 Left side blocked: right turn | Pending | | |
| ST-03 Single obstacle mid-room: left priority | Pending | | |
| ST-04 Dead-end corridor: backward until a side clears, then left | Pending | | |
| ST-05 Dead-end corridor: backward, then right turn | Pending | | |
| ST-06 Single dust cell: short clean_up | Pending | | |
| ST-07 Long dust strip: activations of at most 3 ticks | Pending | | |
| ST-08 Front obstacle during clean_up: stop and keep cleaning, then turn | Pending | | |
| ST-09 clean_up stopped in a dead end: backward after cleaning | Pending | | |
| ST-10 Dust while moving backward: one stop-and-clean activation | Pending | | |
| ST-11 Heavy dust while backing up: activation limit | Pending | | |
| ST-12 Dust detected during a turn | Pending | | |
| ST-13 Corner start: immediate right turn | Pending | | |
| ST-14 Fully enclosed: keeps moving backward | Pending | | |
| ST-15 Slow periodic sensors in a cluttered room | Pending | | |
| ST-16 Corridor with periodic side sensors (period 2) | Pending | | |
| ST-17 Start facing east | Pending | | |
| ST-18 Start facing south, left blocked at the wall | Pending | | |
| ST-19 Cluttered room, 100 ticks | Pending | | |
| ST-20 Scattered dust field | Pending | | |
| ST-21 Stopped clean_up, then right turn | Pending | | |

## AI pre-check (not a verdict)

`node simulator/run_scenarios.js simulator/dist/rvc.wasm --native build/rvc` ran every scenario headless on
the same WebAssembly controller the page uses, recorded every tick, and:
- counted the simulator's requirement-check flags (PR-2, PR-9, PR-10, PR-11, PR-13 and output = command) on every tick;
- fed the identical sensor samples to the native GCC build (`build/rvc`, src/main.c) and compared state,
  commands, turn_count, cleanup_count and dust_during_turn on every tick.

Result: 21 scenarios x 101 snapshots, **0 requirement-check flags**, **native and WebAssembly identical on every tick**.
Transitions per scenario (tick:new state, first 14 shown):

```
   native GCC build: identical on all 101 snapshots
ST-01 ticks=100 end=(7,0,W) states=Forward/TurnLeft flags=0
   transitions: t5:TurnLeft t8:Forward t12:TurnLeft t15:Forward t24:TurnLeft t27:Forward t36:TurnLeft t39:Forward t48:TurnLeft t51:Forward t60:TurnLeft t63:Forward t72:TurnLeft t75:Forward ...
   native GCC build: identical on all 101 snapshots
ST-02 ticks=100 end=(9,9,E) states=Forward/TurnRight/TurnLeft flags=0
   transitions: t3:TurnRight t6:Forward t11:TurnLeft t14:Forward t16:TurnLeft t19:Forward t28:TurnLeft t31:Forward t40:TurnLeft t43:Forward t52:TurnLeft t55:Forward t64:TurnLeft t67:Forward ...
   native GCC build: identical on all 101 snapshots
ST-03 ticks=100 end=(1,0,W) states=Forward/TurnLeft flags=0
   transitions: t2:TurnLeft t5:Forward t9:TurnLeft t12:Forward t18:TurnLeft t21:Forward t30:TurnLeft t33:Forward t42:TurnLeft t45:Forward t54:TurnLeft t57:Forward t66:TurnLeft t69:Forward ...
   native GCC build: identical on all 101 snapshots
ST-04 ticks=100 end=(6,6,S) states=Forward/Backward/TurnLeft flags=0
   transitions: t4:Backward t8:TurnLeft t11:Forward t15:TurnLeft t18:Forward t22:TurnLeft t25:Forward t34:TurnLeft t37:Forward t46:TurnLeft t49:Forward t53:Backward t54:TurnLeft t57:Forward ...
   native GCC build: identical on all 101 snapshots
ST-05 ticks=100 end=(9,0,N) states=Forward/Backward/TurnRight/TurnLeft flags=0
   transitions: t4:Backward t8:TurnRight t11:Forward t16:TurnLeft t19:Forward t24:TurnLeft t27:Forward t31:Backward t32:TurnLeft t35:Forward t44:TurnLeft t47:Forward t50:TurnLeft t53:Forward ...
   native GCC build: identical on all 101 snapshots
ST-06 ticks=100 end=(7,0,W) states=Forward/CleanUpForward/TurnLeft flags=0
   transitions: t3:CleanUpForward t4:Forward t5:TurnLeft t8:Forward t12:TurnLeft t15:Forward t24:TurnLeft t27:Forward t36:TurnLeft t39:Forward t48:TurnLeft t51:Forward t60:TurnLeft t63:Forward ...
   native GCC build: identical on all 101 snapshots
ST-07 ticks=100 end=(9,0,N) states=Forward/CleanUpForward/TurnLeft flags=0
   transitions: t2:CleanUpForward t7:Forward t9:TurnLeft t12:Forward t16:TurnLeft t19:Forward t28:TurnLeft t31:Forward t40:TurnLeft t43:Forward t52:TurnLeft t55:Forward t64:TurnLeft t67:Forward ...
   native GCC build: identical on all 101 snapshots
ST-08 ticks=100 end=(5,3,S) states=Forward/CleanUpForward/CleanUpStopped/TurnLeft flags=0
   transitions: t3:CleanUpForward t4:CleanUpStopped t8:TurnLeft t11:Forward t15:TurnLeft t18:Forward t26:TurnLeft t29:Forward t38:TurnLeft t41:Forward t50:TurnLeft t53:Forward t57:TurnLeft t60:Forward ...
   native GCC build: identical on all 101 snapshots
ST-09 ticks=100 end=(6,4,S) states=Forward/CleanUpForward/CleanUpStopped/Backward/TurnLeft flags=0
   transitions: t3:CleanUpForward t4:CleanUpStopped t6:Backward t7:TurnLeft t10:Forward t14:TurnLeft t17:Forward t24:TurnLeft t27:Forward t36:TurnLeft t39:Forward t48:TurnLeft t51:Forward t55:Backward ...
   native GCC build: identical on all 101 snapshots
ST-10 ticks=100 end=(6,4,S) states=Forward/CleanUpForward/Backward/CleanUpBackwardPause/TurnLeft flags=0
   transitions: t3:CleanUpForward t4:Backward t5:CleanUpBackwardPause t7:Backward t10:TurnLeft t13:Forward t17:TurnLeft t20:Forward t24:TurnLeft t27:Forward t36:TurnLeft t39:Forward t48:TurnLeft t51:Forward ...
   native GCC build: identical on all 101 snapshots
ST-11 ticks=100 end=(6,3,S) states=Forward/CleanUpForward/Backward/CleanUpBackwardPause/TurnLeft flags=0
   transitions: t3:CleanUpForward t4:Backward t5:CleanUpBackwardPause t8:Backward t11:TurnLeft t14:Forward t18:TurnLeft t21:Forward t25:TurnLeft t28:Forward t37:TurnLeft t40:Forward t49:TurnLeft t52:Forward ...
   native GCC build: identical on all 101 snapshots
ST-12 ticks=100 end=(7,0,W) states=Forward/TurnLeft/CleanUpForward flags=0
   transitions: t5:TurnLeft t8:CleanUpForward t9:Forward t12:TurnLeft t15:Forward t24:TurnLeft t27:Forward t36:TurnLeft t39:Forward t48:TurnLeft t51:Forward t60:TurnLeft t63:Forward t72:TurnLeft ...
   native GCC build: identical on all 101 snapshots
ST-13 ticks=100 end=(1,0,E) states=Forward/TurnRight flags=0
   transitions: t1:TurnRight t4:Forward t13:TurnRight t16:Forward t25:TurnRight t28:Forward t37:TurnRight t40:Forward t49:TurnRight t52:Forward t61:TurnRight t64:Forward t73:TurnRight t76:Forward ...
   native GCC build: identical on all 101 snapshots
ST-14 ticks=100 end=(4,4,N) states=Forward/Backward flags=0
   transitions: t1:Backward
   native GCC build: identical on all 101 snapshots
ST-15 ticks=100 end=(4,7,W) states=Forward/TurnLeft/CleanUpForward/CleanUpStopped/TurnRight/Backward flags=0
   transitions: t2:TurnLeft t5:Forward t7:CleanUpForward t9:CleanUpStopped t10:TurnLeft t13:Forward t19:TurnLeft t22:Forward t25:TurnLeft t28:Forward t31:CleanUpForward t34:Forward t37:TurnRight t40:Forward ...
   native GCC build: identical on all 101 snapshots
ST-16 ticks=100 end=(7,5,S) states=Forward/Backward/TurnLeft flags=0
   transitions: t4:Backward t9:TurnLeft t12:Forward t16:TurnLeft t19:Forward t22:TurnLeft t25:Forward t34:TurnLeft t37:Forward t46:TurnLeft t49:Forward t53:Backward t55:TurnLeft t58:Forward ...
   native GCC build: identical on all 101 snapshots
ST-17 ticks=100 end=(9,8,N) states=Forward/TurnLeft flags=0
   transitions: t6:TurnLeft t9:Forward t13:TurnLeft t16:Forward t25:TurnLeft t28:Forward t37:TurnLeft t40:Forward t49:TurnLeft t52:Forward t61:TurnLeft t64:Forward t73:TurnLeft t76:Forward ...
   native GCC build: identical on all 101 snapshots
ST-18 ticks=100 end=(4,6,N) states=Forward/TurnRight/TurnLeft flags=0
   transitions: t6:TurnRight t9:Forward t13:TurnRight t16:Forward t25:TurnRight t28:Forward t37:TurnRight t40:Forward t49:TurnRight t52:Forward t55:TurnRight t58:Forward t67:TurnLeft t70:Forward ...
   native GCC build: identical on all 101 snapshots
ST-19 ticks=100 end=(3,0,N) states=Forward/TurnLeft/CleanUpForward/TurnRight flags=0
   transitions: t2:TurnLeft t5:Forward t7:CleanUpForward t8:Forward t9:TurnLeft t12:Forward t18:TurnLeft t21:Forward t24:TurnLeft t27:Forward t30:CleanUpForward t31:Forward t36:TurnRight t39:Forward ...
   native GCC build: identical on all 101 snapshots
ST-20 ticks=100 end=(7,0,W) states=Forward/CleanUpForward/TurnLeft flags=0
   transitions: t2:CleanUpForward t3:Forward t4:CleanUpForward t5:TurnLeft t8:Forward t10:CleanUpForward t11:Forward t12:TurnLeft t15:Forward t24:TurnLeft t27:Forward t36:TurnLeft t39:Forward t48:TurnLeft ...
   native GCC build: identical on all 101 snapshots
ST-21 ticks=100 end=(5,0,W) states=Forward/CleanUpForward/CleanUpStopped/TurnRight/TurnLeft flags=0
   transitions: t3:CleanUpForward t4:CleanUpStopped t6:TurnRight t9:Forward t14:TurnLeft t17:Forward t18:TurnLeft t21:Forward t25:TurnLeft t28:Forward t37:TurnLeft t40:Forward t44:TurnLeft t47:Forward ...
21 scenarios, 0 with requirement-check flags
```
