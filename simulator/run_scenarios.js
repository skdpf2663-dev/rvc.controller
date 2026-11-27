// Headless run of every predefined scenario on the WebAssembly controller.
// Prints a per-scenario summary and the requirement checks the simulator flags.
// It does not assign PASS/FAIL: system test results are decided by a human evaluator.
// Usage: node simulator/run_scenarios.js [path/to/rvc.wasm] [--trace ST-xx] [--native build/rvc]
const fs = require("fs");
const path = require("path");
const Sim = require("./web/sim-core.js");
const { scenarios } = require("./web/scenarios.js");

const wasmPath = process.argv[2] && !process.argv[2].startsWith("--") ? process.argv[2] : path.join(__dirname, "dist", "rvc.wasm");
const traceId = process.argv.includes("--trace") ? process.argv[process.argv.indexOf("--trace") + 1] : null;
const nativeBin = process.argv.includes("--native") ? process.argv[process.argv.indexOf("--native") + 1] : null;
const { execFileSync } = require("child_process");
// Feeds the same sensor samples to the native GCC build (src/main.c) and checks
// that it produces the same controller state and commands on every tick.
function compareWithNative(h) {
  const tok = (a, v) => (a ? (v ? "1" : "0") : "-");
  const input = h.slice(1).map((s) => [tok(s.input.frontArrived, s.input.front), tok(s.input.leftArrived, s.input.left),
    tok(s.input.rightArrived, s.input.right), tok(s.input.dustArrived, s.input.dust)].join(" ")).join("\n") + "\n";
  const lines = execFileSync(nativeBin, [], { input }).toString().trim().split("\n");
  let diffs = 0;
  h.forEach((s, i) => {
    const expect = `tick=${s.tick} state=${s.state} dir=${s.directionCommand} clean=${s.cleanCommand} turn=${s.turnCount} cleanup=${s.cleanupCount} dust_turn=${+s.dustDuringTurn}`;
    if (!lines[i] || !lines[i].startsWith(expect)) diffs++;
  });
  return diffs;
}
const sim = new Sim.Simulation(Sim.instantiate(fs.readFileSync(wasmPath)));
let flagged = 0;
for (const sc of scenarios) {
  const errs = Sim.validate(sc);
  if (errs.length) { console.log(sc.id, "INVALID", errs); flagged++; continue; }
  sim.reset(sc);
  const h = sim.runToEnd();
  const states = [...new Set(h.map((s) => s.state))];
  const checks = h.flatMap((s) => s.checks.map((c) => "t" + s.tick + " " + c));
  if (checks.length) flagged++;
  if (nativeBin) { const d = compareWithNative(h); if (d) { flagged++; console.log(`   NATIVE MISMATCH on ${d} ticks`); } else console.log("   native GCC build: identical on all " + h.length + " snapshots"); }
  const transitions = h.filter((s, i) => i > 0 && s.state !== h[i - 1].state).map((s) => "t" + s.tick + ":" + s.state);
  console.log(`${sc.id} ticks=${h.length - 1} end=(${h.at(-1).x},${h.at(-1).y},${h.at(-1).heading}) states=${states.join("/")} flags=${checks.length}`);
  console.log("   transitions: " + transitions.slice(0, 14).join(" ") + (transitions.length > 14 ? " ..." : ""));
  for (const c of checks.slice(0, 5)) console.log("   FLAG " + c);
  if (traceId === sc.id) for (const s of h) console.log(`   t${s.tick} (${s.x},${s.y},${s.heading}) ${s.state} ${s.motor}/${s.cleaner} turn=${s.turnCount} cu=${s.cleanupCount} dt=${+s.dustDuringTurn} sens=${+s.sensors.front}${+s.sensors.left}${+s.sensors.right}${+s.sensors.dust} ${s.changes.join("; ")}`);
}
console.log(`${scenarios.length} scenarios, ${flagged} with requirement-check flags`);
process.exitCode = flagged ? 1 : 0;
