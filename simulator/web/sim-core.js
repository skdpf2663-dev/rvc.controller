/* RVC simulator core (no DOM). Runs the real C controller compiled to
 * WebAssembly and models only the environment: the 10x10 map, sensor
 * sampling, movement, and dust removal. It never decides Direction or Clean.
 * Works in the browser (globalThis.RvcSim) and in Node (module.exports). */
(function (root) {
  "use strict";

  const N = 10;
  const MAX_TICKS = 100;
  const STATE_NAMES = ["Forward", "CleanUpForward", "CleanUpStopped", "TurnLeft",
                       "TurnRight", "Backward", "CleanUpBackwardPause"];
  const DIR_NAMES = ["forward", "backward", "turn_left", "turn_right", "stop"];
  const CLEAN_NAMES = ["clean_off", "clean_on", "clean_up"];
  const HEADINGS = ["N", "E", "S", "W"];
  const VEC = [[0, -1], [1, 0], [0, 1], [-1, 0]];

  function instantiate(wasmBytes) {
    const module = new WebAssembly.Module(wasmBytes);
    return new WebAssembly.Instance(module, {}).exports;
  }

  /* Map format: rows of 10 chars. '.' empty, '#' obstacle, '1'-'9' dust amount. */
  function parseMap(rows) {
    if (!Array.isArray(rows) || rows.length !== N) throw new Error("map must have 10 rows");
    const obstacle = [], dust = [];
    for (let y = 0; y < N; y++) {
      const row = rows[y];
      if (typeof row !== "string" || row.length !== N) throw new Error("row " + y + " must have 10 cells");
      obstacle.push([]); dust.push([]);
      for (let x = 0; x < N; x++) {
        const c = row[x];
        if (c !== "." && c !== "#" && !(c >= "1" && c <= "9")) throw new Error("invalid cell '" + c + "' at (" + x + "," + y + ")");
        obstacle[y].push(c === "#");
        dust[y].push(c >= "1" && c <= "9" ? Number(c) : 0);
      }
    }
    return { obstacle, dust };
  }

  function mapToRows(obstacle, dust) {
    const rows = [];
    for (let y = 0; y < N; y++) {
      let s = "";
      for (let x = 0; x < N; x++) s += obstacle[y][x] ? "#" : dust[y][x] > 0 ? String(Math.min(9, dust[y][x])) : ".";
      rows.push(s);
    }
    return rows;
  }

  /* Returns a list of problems; an empty list means the configuration is valid. */
  function validate(config) {
    const errors = [];
    let parsed = null;
    try { parsed = parseMap(config.map); } catch (e) { errors.push(e.message); }
    const s = config.start || {};
    if (!Number.isInteger(s.x) || !Number.isInteger(s.y) || s.x < 0 || s.y < 0 || s.x >= N || s.y >= N)
      errors.push("start position must be inside the 10x10 grid");
    else if (parsed && parsed.obstacle[s.y][s.x]) errors.push("start position is on an obstacle");
    if (!HEADINGS.includes(s.heading)) errors.push("start heading must be N, E, S, or W");
    const p = config.sidePeriod === undefined ? 1 : config.sidePeriod;
    if (!Number.isInteger(p) || p < 1 || p > 5) errors.push("side/dust sensor period must be 1-5 ticks");
    const m = config.maxTicks === undefined ? MAX_TICKS : config.maxTicks;
    if (!Number.isInteger(m) || m < 1 || m > MAX_TICKS) errors.push("max ticks must be 1-100");
    return errors;
  }

  class Simulation {
    constructor(wasmExports) { this.c = wasmExports; }

    /* initialize(configuration) / reset(): restores controller and map. */
    reset(config) {
      const errors = validate(config);
      if (errors.length) throw new Error(errors.join("; "));
      this.config = JSON.parse(JSON.stringify(config));
      this.sidePeriod = config.sidePeriod || 1;
      this.maxTicks = config.maxTicks || MAX_TICKS;
      const m = parseMap(config.map);
      this.obstacle = m.obstacle;
      this.dust = m.dust;
      this.x = config.start.x;
      this.y = config.start.y;
      this.h = HEADINGS.indexOf(config.start.heading);
      this.lastFront = null;
      this.c.rvc_reset();
      this.history = [this.snapshot(null, null, [])];
      return this.history[0];
    }

    blocked(x, y) { return x < 0 || y < 0 || x >= N || y >= N || this.obstacle[y][x]; }
    blockedToward(h) { const v = VEC[(h + 4) % 4]; return this.blocked(this.x + v[0], this.y + v[1]); }
    get tick() { return this.history.length - 1; }
    get finished() { return this.tick >= this.maxTicks; }

    /* Sensor inputs from the map. Front arrives by interrupt (only when it changes);
     * left, right and dust arrive periodically every sidePeriod ticks (PR-8). */
    senseInputs(t) {
      const front = this.blockedToward(this.h);
      const inp = { frontArrived: false, front: false, leftArrived: false, left: false,
                    rightArrived: false, right: false, dustArrived: false, dust: false };
      if (this.lastFront === null || front !== this.lastFront) {
        inp.frontArrived = true; inp.front = front; this.lastFront = front;
      }
      if ((t - 1) % this.sidePeriod === 0) {
        inp.leftArrived = inp.rightArrived = inp.dustArrived = true;
        inp.left = this.blockedToward(this.h + 3);
        inp.right = this.blockedToward(this.h + 1);
        inp.dust = this.dust[this.y][this.x] > 0;
      }
      return inp;
    }

    /* step(): one complete system update. */
    step() {
      if (this.finished) return null;
      const t = this.tick + 1;
      const prev = this.history[this.history.length - 1];
      const inp = this.senseInputs(t);
      const b = (v) => (v ? 1 : 0);
      this.c.rvc_step(b(inp.frontArrived), b(inp.front), b(inp.leftArrived), b(inp.left),
                      b(inp.rightArrived), b(inp.right), b(inp.dustArrived), b(inp.dust));
      const st = this.readController();
      const changes = this.applyOutputs(st);
      const snap = this.snapshot(inp, prev, changes);
      snap.checks = checkRequirements(this.history, snap);
      this.history.push(snap);
      return snap;
    }

    readController() {
      const c = this.c;
      return {
        state: c.rvc_get_state(), turnCount: c.rvc_get_turn_count(), cleanupCount: c.rvc_get_cleanup_count(),
        dustDuringTurn: c.rvc_get_dust_during_turn() === 1, directionCommand: c.rvc_get_direction_command(),
        cleanCommand: c.rvc_get_clean_command(), motor: c.rvc_get_motor_output(), cleaner: c.rvc_get_cleaner_output(),
        sensors: c.rvc_get_sensors(), tick: Number(c.rvc_get_tick()),
      };
    }

    /* Environment response to the outputs actually sent to the motor and cleaner. */
    applyOutputs(st) {
      const changes = [];
      if (st.cleaner === 2 && this.dust[this.y][this.x] > 0) {
        this.dust[this.y][this.x]--;
        changes.push("dust at (" + this.x + "," + this.y + ") " + (this.dust[this.y][this.x] + 1) + "->" + this.dust[this.y][this.x]);
      }
      const v = VEC[this.h];
      const from = "(" + this.x + "," + this.y + ")";
      if (st.motor === 0 || st.motor === 1) {
        const sign = st.motor === 0 ? 1 : -1;
        const nx = this.x + sign * v[0], ny = this.y + sign * v[1];
        if (!this.blocked(nx, ny)) { this.x = nx; this.y = ny; changes.push("moved " + from + "->(" + nx + "," + ny + ")"); }
        else changes.push("movement blocked at " + from);
      } else if ((st.motor === 2 || st.motor === 3) && st.turnCount === 3) {
        const oldH = HEADINGS[this.h];
        this.h = (this.h + (st.motor === 2 ? 3 : 1)) % 4;
        changes.push("heading " + oldH + "->" + HEADINGS[this.h] + " (turn completed)");
      }
      return changes;
    }

    snapshot(inp, prev, changes) {
      const st = this.readController();
      const s = st.sensors;
      return {
        tick: st.tick, x: this.x, y: this.y, heading: HEADINGS[this.h],
        input: inp,
        sensors: { front: !!(s & 8), left: !!(s & 4), right: !!(s & 2), dust: !!(s & 1) },
        prevState: prev ? prev.state : null,
        state: STATE_NAMES[st.state],
        transition: prev ? prev.state + " -> " + STATE_NAMES[st.state] : "begin automatic cleaning -> Forward",
        directionCommand: DIR_NAMES[st.directionCommand], cleanCommand: CLEAN_NAMES[st.cleanCommand],
        motor: st.motor >= 0 ? DIR_NAMES[st.motor] : "-", cleaner: st.cleaner >= 0 ? CLEAN_NAMES[st.cleaner] : "-",
        turnCount: st.turnCount, cleanupCount: st.cleanupCount, dustDuringTurn: st.dustDuringTurn,
        map: mapToRows(this.obstacle, this.dust), changes, checks: [],
      };
    }

    runToEnd() { while (!this.finished) this.step(); return this.history; }
  }

  /* Requirement checks recorded with each tick. They flag a possible violation
   * for the evaluator; they never assign PASS or FAIL (the human does). */
  function checkRequirements(history, s) {
    const out = [];
    const turning = s.state === "TurnLeft" || s.state === "TurnRight";
    if ((s.cleaner === "clean_off") !== (turning || s.state === "Backward"))
      out.push("PR-13: clean_off must be used only while turning or moving backward");
    if (s.motor === "stop" && s.cleaner !== "clean_up") out.push("PR-13: stop without clean_up");
    if (s.state === "Forward" && (s.motor !== "forward" || s.cleaner !== "clean_on"))
      out.push("PR-2: Forward must go forward with clean_on");
    if (s.cleanupCount > 3) out.push("PR-11: clean_up activation longer than 3 ticks");
    if (turning && s.turnCount > 3) out.push("PR-10: turn longer than 3 ticks");
    const prev = history[history.length - 1];
    const prevTurning = prev && (prev.state === "TurnLeft" || prev.state === "TurnRight");
    if (prevTurning && prev.turnCount < 3 && !(s.state === prev.state && s.turnCount === prev.turnCount + 1))
      out.push("PR-10: turn ended before 3 ticks");
    const newTurn = turning && s.turnCount === 1;
    if (newTurn && !s.sensors.left && s.state !== "TurnLeft") out.push("PR-9: left turn has priority");
    if (s.motor !== s.directionCommand || s.cleaner !== s.cleanCommand) out.push("Output differs from selected command");
    return out;
  }

  const api = { N, MAX_TICKS, STATE_NAMES, DIR_NAMES, CLEAN_NAMES, HEADINGS, instantiate, parseMap, mapToRows, validate, Simulation };
  if (typeof module !== "undefined" && module.exports) module.exports = api;
  root.RvcSim = api;
})(typeof globalThis !== "undefined" ? globalThis : this);
