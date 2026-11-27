/* Predefined system test scenarios (ST-01 .. ST-21).
 * Map rows: y = 0 (top) .. 9 (bottom); '.' empty, '#' obstacle, '1'-'9' dust amount.
 * Default start (4,4) heading N (no initial heading is given by the requirements).
 * Expected behavior is written from the requirements (PR-x) and the Level 4 FSM;
 * the human evaluator decides PASS / FAIL / INCONCLUSIVE. */
(function (root) {
  "use strict";
  const E = "..........";
  const blank = () => [E, E, E, E, E, E, E, E, E, E];
  const put = (rows, cells) => {
    const g = rows.map((r) => r.split(""));
    for (const [x, y, c] of cells) g[y][x] = c;
    return g.map((r) => r.join(""));
  };
  const corridor = (extra) => put(blank(), [[4, 0, "#"], [3, 1, "#"], [3, 2, "#"], [3, 3, "#"], [3, 4, "#"],
    [5, 1, "#"], [5, 2, "#"], [5, 3, "#"], [5, 4, "#"]].concat(extra || []));
  const cluttered = [
    "..#.......", ".....3..#.", "#...#.....", "..1....##.", "......#...",
    ".#.......2", "...2.#....", "......#...", ".#.....1..", "....#.....",
  ];

  const S = (o) => Object.assign({ start: { x: 4, y: 4, heading: "N" }, sidePeriod: 1, maxTicks: 100 }, o);

  const scenarios = [
    S({ id: "ST-01", name: "Open room: straight to the wall, then left turn",
      map: blank(), functions: ["Forward cleaning", "Obstacle avoidance by turning", "Left-turn priority", "3-tick turn"],
      requirements: ["PR-2", "PR-3", "PR-9", "PR-10", "PR-13"], artifacts: ["FSM Forward/TurnLeft rows", "Process 2.2.2"],
      expected: "Goes north with forward + clean_on until the wall at y=0. With the front blocked and both sides clear it turns LEFT (left priority) for exactly 3 ticks with clean_off, then goes forward (west) with clean_on again." }),
    S({ id: "ST-02", name: "Left side blocked: right turn",
      map: put(blank(), [[4, 1, "#"], [3, 2, "#"]]), functions: ["Obstacle avoidance by turning", "Right turn when left is blocked", "3-tick turn", "clean_off only while turning"],
      requirements: ["PR-3", "PR-9", "PR-10", "PR-13"], artifacts: ["FSM row Forward->TurnRight", "Process 2.2.2"],
      expected: "At (4,2) the front (4,1) and the left (3,2) are blocked, the right is clear, so it turns RIGHT for 3 ticks with clean_off, then goes east with clean_on." }),
    S({ id: "ST-03", name: "Single obstacle mid-room: left priority",
      map: put(blank(), [[4, 2, "#"]]), functions: ["Forward cleaning", "Left-turn priority", "3-tick turn"],
      requirements: ["PR-2", "PR-3", "PR-9", "PR-10"], artifacts: ["FSM row Forward->TurnLeft"],
      expected: "Stops cleaning in front of (4,2) and turns LEFT although the right is also clear. After 3 turn ticks it continues west with clean_on." }),
    S({ id: "ST-04", name: "Dead-end corridor: backward until a side clears, then left",
      map: corridor(), functions: ["Backward when front, left and right are blocked", "Backward continues until a side is clear", "Left-turn priority after backward", "clean_off while backward"],
      requirements: ["PR-4", "PR-14", "PR-9", "PR-13"], artifacts: ["FSM Backward rows", "Process 2.2.2"],
      expected: "Drives up the corridor to (4,1). Front, left and right are blocked, so it moves BACKWARD with clean_off. Backward continues while both sides stay blocked; at (4,5) both sides are clear and it turns LEFT, then goes forward with clean_on." }),
    S({ id: "ST-05", name: "Dead-end corridor: backward, then right turn",
      map: corridor([[3, 5, "#"], [3, 6, "#"]]), functions: ["Backward when all blocked", "Backward until a side is clear", "Right turn when only the right is clear"],
      requirements: ["PR-4", "PR-14", "PR-9", "PR-10"], artifacts: ["FSM rows Backward->Backward, Backward->TurnRight"],
      expected: "Backs out of the corridor. At (4,5) the left (3,5) is still blocked and the right is clear, so it turns RIGHT for 3 ticks, then goes forward." }),
    S({ id: "ST-06", name: "Single dust cell: short clean_up",
      map: put(blank(), [[4, 2, "1"]]), functions: ["Dust detection powers up cleaning", "clean_up ends when dust is gone", "Forward cleaning"],
      requirements: ["PR-5", "PR-11", "PR-2", "PR-12"], artifacts: ["FSM rows Forward->CleanUpForward, CleanUpForward->Forward"],
      expected: "On the dusty cell it switches to clean_up while still moving forward. On the next cell the dust is no longer detected, so the activation ends early (less than 3 ticks) and clean_on resumes." }),
    S({ id: "ST-07", name: "Long dust strip: activations of at most 3 ticks",
      start: { x: 4, y: 8, heading: "N" }, map: put(blank(), [[4, 7, "1"], [4, 6, "1"], [4, 5, "1"], [4, 4, "1"], [4, 3, "1"]]),
      functions: ["Dust powers up cleaning", "One activation lasts at most 3 ticks", "Next activation starts while dust remains", "Forward cleaning"],
      requirements: ["PR-5", "PR-11", "PR-2"], artifacts: ["FSM rows CleanUpForward (activation ends, dust remains)"],
      expected: "Over the 5 dusty cells it moves forward with clean_up. cleanup_count goes 1,2,3 and then restarts at 1 because dust is still detected (no activation longer than 3 ticks). After the strip it returns to clean_on." }),
    S({ id: "ST-08", name: "Front obstacle during clean_up: stop and keep cleaning, then turn",
      map: put(blank(), [[4, 2, "1"], [4, 1, "4"], [4, 0, "#"]]),
      functions: ["Dust powers up cleaning", "Stop in place when front is blocked during clean_up", "Repeated activations while stopped", "Turn after cleaning ends"],
      requirements: ["PR-15", "PR-11", "PR-5", "PR-3", "PR-13"], artifacts: ["FSM rows CleanUpForward->CleanUpStopped, CleanUpStopped rows"],
      expected: "Enters clean_up on (4,2), moves to (4,1) where the front is blocked and dust is present: direction becomes stop with clean_up and it stays at (4,1). It keeps cleaning (new activation when 3 ticks end) and does not turn or move backward until the dust is gone; then it turns LEFT." }),
    S({ id: "ST-09", name: "clean_up stopped in a dead end: backward after cleaning",
      map: put(blank(), [[4, 2, "1"], [4, 1, "2"], [4, 0, "#"], [3, 1, "#"], [5, 1, "#"]]),
      functions: ["Stop and keep cleaning during clean_up", "Backward when all sides are blocked after cleaning", "clean_off only while backward"],
      requirements: ["PR-15", "PR-4", "PR-13", "PR-11"], artifacts: ["FSM row CleanUpStopped->Backward"],
      expected: "Stops at (4,1) with clean_up until the dust is gone. Then, with front, left and right blocked, it moves backward with clean_off, and turns when a side becomes clear." }),
    S({ id: "ST-10", name: "Dust while moving backward: one stop-and-clean activation",
      map: corridor([[4, 2, "3"]]), functions: ["Backward when all blocked", "Dust while backward: stop and clean_up", "Return to backward after one activation"],
      requirements: ["PR-16", "PR-4", "PR-14", "PR-11"], artifacts: ["FSM rows Backward->CleanUpBackwardPause->Backward"],
      expected: "After reaching the dead end it moves backward. On the dusty cell it stops with clean_up for one activation (at most 3 ticks, shorter if dust disappears), then continues backward with clean_off, then turns when a side is clear." }),
    S({ id: "ST-11", name: "Heavy dust while backing up: activation limit",
      map: corridor([[4, 2, "9"]]), functions: ["Dust while backward", "Activation limited to 3 ticks", "Backward resumes even if dust remains"],
      requirements: ["PR-16", "PR-11", "PR-14"], artifacts: ["FSM row CleanUpBackwardPause (cleanup_count = 3) -> Backward"],
      expected: "While backing up over the heavy dust cell it stops for exactly 3 clean_up ticks, then moves backward again although dust is still detected (one activation only)." }),
    S({ id: "ST-12", name: "Dust detected during a turn",
      map: put(blank(), [[4, 0, "1"]]), functions: ["3-tick turn is not interrupted", "Dust during turn: forward in clean_up afterwards", "Left-turn priority"],
      requirements: ["PR-17", "PR-10", "PR-5", "PR-9"], artifacts: ["FSM rows TurnLeft->CleanUpForward", "dust_during_turn"],
      expected: "At the wall cell (4,0) there is dust. It still turns LEFT for 3 ticks with clean_off (dust_during_turn = 1), and after the turn it goes forward with clean_up." }),
    S({ id: "ST-13", name: "Corner start: immediate right turn",
      start: { x: 0, y: 0, heading: "N" }, map: blank(), functions: ["Obstacle avoidance from the first tick", "Right turn when left is blocked", "3-tick turn"],
      requirements: ["PR-3", "PR-9", "PR-10", "PR-8"], artifacts: ["FSM row Forward->TurnRight", "Interpretation 2 (initial sensor values)"],
      expected: "Front (outside the grid) and left (outside) are blocked on tick 1, so it turns RIGHT for 3 ticks, then goes east with clean_on." }),
    S({ id: "ST-14", name: "Fully enclosed: keeps moving backward",
      map: put(blank(), [[4, 3, "#"], [4, 5, "#"], [3, 4, "#"], [5, 4, "#"]]),
      functions: ["Backward when all blocked", "Backward continues until a side is clear", "clean_off only while backward"],
      requirements: ["PR-4", "PR-14", "PR-13"], artifacts: ["FSM row Backward->Backward"],
      expected: "All four neighbors are obstacles. It commands backward with clean_off on every tick (the cell behind is blocked, so it does not move) and never turns because no side becomes clear." }),
    S({ id: "ST-15", name: "Slow periodic sensors in a cluttered room",
      map: cluttered, sidePeriod: 3, functions: ["Latest sensor values are used", "Obstacle avoidance", "Dust power-up", "Forward cleaning"],
      requirements: ["PR-8", "PR-3", "PR-5", "PR-2"], artifacts: ["Process 1 Capture Sensor Values", "D1 Latest Sensor Values"],
      expected: "Left, right and dust samples arrive only every 3 ticks; the front arrives by interrupt when it changes. The controller decides from the latest stored values, so a turn direction may use a side value up to 2 ticks old. The RVC never enters an obstacle cell." }),
    S({ id: "ST-16", name: "Corridor with periodic side sensors (period 2)",
      map: corridor(), sidePeriod: 2, functions: ["Interrupt front / periodic sides", "Backward until a side is clear", "Left-turn priority"],
      requirements: ["PR-8", "PR-14", "PR-9", "PR-4"], artifacts: ["Process 1.1-1.4", "FSM Backward rows"],
      expected: "Same path as ST-04, but backward may last one tick longer because the side values are refreshed every 2 ticks. It still turns LEFT when both sides read clear." }),
    S({ id: "ST-17", name: "Start facing east",
      start: { x: 4, y: 4, heading: "E" }, map: blank(), functions: ["Forward cleaning", "Obstacle avoidance by turning", "Left-turn priority", "3-tick turn"],
      requirements: ["PR-2", "PR-3", "PR-9", "PR-10"], artifacts: ["FSM Forward/TurnLeft rows"],
      expected: "Goes east to x=9, turns LEFT (to the north) for 3 ticks, then continues north with clean_on." }),
    S({ id: "ST-18", name: "Start facing south, left blocked at the wall",
      start: { x: 4, y: 4, heading: "S" }, map: put(blank(), [[5, 9, "#"]]),
      functions: ["Obstacle avoidance", "Right turn when left is blocked", "3-tick turn", "clean_off only while turning"],
      requirements: ["PR-3", "PR-9", "PR-10", "PR-13"], artifacts: ["FSM row Forward->TurnRight"],
      expected: "Goes south to (4,9). The front (wall) and its left (east, (5,9)) are blocked, so it turns RIGHT (to the west) for 3 ticks, then goes west." }),
    S({ id: "ST-19", name: "Cluttered room, 100 ticks",
      map: cluttered, functions: ["Forward cleaning", "Obstacle avoidance", "Backward escape", "Dust power-up"],
      requirements: ["PR-2", "PR-3", "PR-4", "PR-5", "PR-10", "PR-11", "PR-13"], artifacts: ["Level 4 FSM", "Process 2.2.2"],
      expected: "Runs the whole 100 ticks without entering an obstacle. Every turn lasts 3 ticks, every clean_up activation at most 3 ticks, and clean_off appears only while turning or backing up." }),
    S({ id: "ST-20", name: "Scattered dust field",
      map: ["..1.......", "....2.....", ".1......1.", "....1.....", ".......2..", "..3.......", "......1...", ".1........", "....2...1.", ".........."],
      functions: ["Dust powers up cleaning", "Activation limit / early end", "Return to clean_on", "Obstacle (wall) avoidance"],
      requirements: ["PR-5", "PR-11", "PR-2", "PR-3"], artifacts: ["FSM CleanUpForward rows"],
      expected: "Each time a dusty cell is under the RVC it switches to clean_up while moving; the activation ends when dust is no longer detected or after 3 ticks, and clean_on resumes. Walls are avoided by turns." }),
    S({ id: "ST-21", name: "Stopped clean_up, then right turn",
      map: put(blank(), [[4, 2, "1"], [4, 1, "2"], [4, 0, "#"], [3, 1, "#"]]),
      functions: ["Stop and keep cleaning during clean_up", "Turn selection after cleaning ends", "Right turn when left is blocked"],
      requirements: ["PR-15", "PR-9", "PR-3", "PR-10"], artifacts: ["FSM row CleanUpStopped->TurnRight"],
      expected: "Stops at (4,1) with clean_up until the dust is gone; then, with the front and left blocked and the right clear, it turns RIGHT for 3 ticks." }),
  ];

  const api = { scenarios };
  if (typeof module !== "undefined" && module.exports) module.exports = api;
  root.RvcScenarios = scenarios;
})(typeof globalThis !== "undefined" ? globalThis : this);
