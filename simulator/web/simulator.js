/* Web UI of the RVC simulator. Uses RvcSim (sim-core.js) and RvcScenarios. */
(function () {
  "use strict";
  const Sim = window.RvcSim;
  const $ = (id) => document.getElementById(id);
  const ARROW = { N: "▲", E: "▶", S: "▼", W: "◀" };
  const LS_CUSTOM = "rvc-sim-custom-scenarios";
  const LS_EVAL = "rvc-sim-evaluations";

  const store = {
    get(key, def) { try { const v = localStorage.getItem(key); return v ? JSON.parse(v) : def; } catch (e) { return def; } },
    set(key, val) { try { localStorage.setItem(key, JSON.stringify(val)); return true; } catch (e) { return false; } },
  };

  function b64ToBytes(b64) {
    const bin = atob(b64), bytes = new Uint8Array(bin.length);
    for (let i = 0; i < bin.length; i++) bytes[i] = bin.charCodeAt(i);
    return bytes;
  }

  const sim = new Sim.Simulation(Sim.instantiate(b64ToBytes(RVC_WASM_BASE64)));
  let customScenarios = store.get(LS_CUSTOM, []);
  let scenario = null;   // selected scenario (definition)
  let config = null;     // working configuration (may be edited)
  let view = 0;          // history index being displayed
  let timer = null;

  const allScenarios = () => window.RvcScenarios.concat(customScenarios);
  const clone = (o) => JSON.parse(JSON.stringify(o));

  // ---------------- scenario selection ----------------
  function fillScenarioSelect(selectId) {
    const sel = $("scenario-select");
    sel.innerHTML = "";
    for (const s of allScenarios()) {
      const o = document.createElement("option");
      o.value = s.id; o.textContent = s.id + "  " + s.name + (s.custom ? " (사용자)" : "");
      sel.appendChild(o);
    }
    if (selectId) sel.value = selectId;
  }

  function loadScenario(id) {
    scenario = allScenarios().find((s) => s.id === id) || allScenarios()[0];
    config = { map: clone(scenario.map), start: clone(scenario.start), sidePeriod: scenario.sidePeriod || 1, maxTicks: 100 };
    $("start-heading").value = config.start.heading;
    $("side-period").value = String(config.sidePeriod);
    renderScenarioInfo();
    loadEvaluation();
    resetSim();
  }

  // ---------------- simulation control ----------------
  function resetSim() {
    pause();
    const errors = Sim.validate(config);
    $("validation").hidden = errors.length === 0;
    $("validation").textContent = errors.length ? "지도 설정 오류: " + errors.join(" / ") : "";
    if (errors.length) { sim.history = []; view = 0; render(); return false; }
    sim.reset(config);
    view = 0;
    render();
    return true;
  }

  function stepOnce() {
    if (!sim.history.length) return false;
    if (view < sim.history.length - 1) { view++; render(); return true; }   // move forward through saved snapshots
    if (sim.finished) { pause(); return false; }
    sim.step();
    view = sim.history.length - 1;
    render();
    return !sim.finished;
  }

  function play() {
    if (timer || !sim.history.length) return;
    if (sim.finished && view === sim.history.length - 1) return;
    timer = setInterval(() => { if (!stepOnce()) pause(); }, Number($("speed").value));
    updateButtons();
  }
  function pause() { if (timer) { clearInterval(timer); timer = null; } updateButtons(); }

  // ---------------- rendering ----------------
  function render() { renderGrid(); renderInspector(); renderHistory(); updateButtons(); }

  function renderGrid() {
    const grid = $("grid");
    grid.innerHTML = "";
    const snap = sim.history[view];
    const rows = snap ? snap.map : config.map;
    const editing = $("edit-mode").checked;
    const trail = new Set();
    if (snap) for (let i = 0; i <= view; i++) trail.add(sim.history[i].x + "," + sim.history[i].y);
    for (let y = 0; y < Sim.N; y++) for (let x = 0; x < Sim.N; x++) {
      const c = rows[y][x];
      const cell = document.createElement("div");
      cell.className = "cell";
      cell.dataset.x = x; cell.dataset.y = y;
      if (c === "#") cell.classList.add("obstacle");
      if (trail.has(x + "," + y)) cell.classList.add("trail");
      if ((editing || !snap) && config.start.x === x && config.start.y === y) cell.classList.add("start");
      if (x === 0 || y === 0) { const k = document.createElement("span"); k.className = "coord"; k.textContent = x === 0 ? y : x; cell.appendChild(k); }
      if (c >= "1" && c <= "9") { const d = document.createElement("span"); d.className = "dust"; d.textContent = c; cell.appendChild(d); }
      const rx = snap ? snap.x : config.start.x, ry = snap ? snap.y : config.start.y;
      if (rx === x && ry === y) {
        const r = document.createElement("span"); r.className = "rvc";
        const h = snap ? snap.heading : config.start.heading;
        r.textContent = ARROW[h]; r.title = "RVC (" + x + "," + y + ") heading " + h;
        cell.appendChild(r);
      }
      cell.addEventListener("click", () => editCell(x, y));
      grid.appendChild(cell);
    }
  }

  function fmtBool(v) { return v ? "1 (감지)" : "0"; }
  function fmtInput(arrived, v) { return arrived ? (v ? "1 도착" : "0 도착") : "- (도착 없음)"; }

  function renderInspector() {
    const s = sim.history[view], p = view > 0 ? sim.history[view - 1] : null;
    const t = $("inspector");
    if (!s) { t.innerHTML = ""; $("tick-counter").textContent = ""; $("changes").innerHTML = ""; $("checks").innerHTML = ""; return; }
    $("tick-counter").innerHTML = "보는 tick <b>" + s.tick + "</b> · 실행한 tick " + sim.tick + " / " + sim.maxTicks +
      (view < sim.history.length - 1 ? " · (저장된 스냅숏을 보는 중)" : "");
    const fields = [
      ["Tick", (x) => x.tick], ["위치 (x, y)", (x) => "(" + x.x + ", " + x.y + ")"], ["방향 (heading)", (x) => x.heading],
      ["이전 상태", (x) => x.prevState || "-"], ["현재 상태", (x) => x.state], ["전이", (x) => x.transition],
      ["Front Obstacle (저장값)", (x) => fmtBool(x.sensors.front)], ["Left Obstacle (저장값)", (x) => fmtBool(x.sensors.left)],
      ["Right Obstacle (저장값)", (x) => fmtBool(x.sensors.right)], ["Dust Detected (저장값)", (x) => fmtBool(x.sensors.dust)],
      ["이번 tick 입력 Front (인터럽트)", (x) => x.input ? fmtInput(x.input.frontArrived, x.input.front) : "-"],
      ["이번 tick 입력 Left/Right/Dust (주기)", (x) => x.input ? [fmtInput(x.input.leftArrived, x.input.left), fmtInput(x.input.rightArrived, x.input.right), fmtInput(x.input.dustArrived, x.input.dust)].join(" · ") : "-"],
      ["Direction Command", (x) => x.directionCommand], ["Clean Command", (x) => x.cleanCommand],
      ["Motor 출력 (Direction)", (x) => x.motor], ["Cleaner 출력 (Clean)", (x) => x.cleaner],
      ["turn_count (회전 타이머)", (x) => x.turnCount], ["cleanup_count (clean_up 타이머)", (x) => x.cleanupCount],
      ["dust_during_turn", (x) => (x.dustDuringTurn ? "true" : "false")],
    ];
    t.innerHTML = "<tr><th>항목</th><td><b>현재</b></td><td class='prev'>직전 tick</td></tr>";
    for (const [name, f] of fields) {
      const cur = String(f(s)), prev = p ? String(f(p)) : "";
      const tr = document.createElement("tr");
      tr.innerHTML = "<th></th><td></td><td class='prev'></td>";
      tr.children[0].textContent = name; tr.children[1].textContent = cur; tr.children[2].textContent = prev;
      if (p && cur !== prev && name !== "Tick") tr.children[1].classList.add("changed");
      t.appendChild(tr);
    }
    $("changes").innerHTML = s.changes.length ? "<h3>지도 변화</h3><ul>" + s.changes.map((c) => "<li>" + c + "</li>").join("") + "</ul>" : "";
    $("checks").innerHTML = "<h3>요구사항 점검 (참고)</h3>" + (s.checks.length ? "<ul>" + s.checks.map((c) => "<li class='flag'>" + c + "</li>").join("") + "</ul>" : "<p class='note'>이 tick에서 표시된 위반 없음</p>");
  }

  function renderHistory() {
    const t = $("history");
    t.innerHTML = "<thead><tr><th>tick</th><th>위치</th><th>방향</th><th>입력 F L R D</th><th>저장값 F L R D</th><th>상태</th><th>Motor</th><th>Cleaner</th><th>turn</th><th>cleanup</th><th>dust_turn</th><th>지도 변화</th><th>점검</th></tr></thead>";
    const body = document.createElement("tbody");
    const tok = (a, v) => (a ? (v ? "1" : "0") : "-");
    sim.history.forEach((s, i) => {
      const tr = document.createElement("tr");
      if (i === view) tr.classList.add("viewing");
      if (s.checks.length) tr.classList.add("flagged");
      if (i > 0 && s.state !== sim.history[i - 1].state) tr.classList.add("transition");
      const inp = s.input ? [tok(s.input.frontArrived, s.input.front), tok(s.input.leftArrived, s.input.left), tok(s.input.rightArrived, s.input.right), tok(s.input.dustArrived, s.input.dust)].join(" ") : "";
      const sv = [s.sensors.front, s.sensors.left, s.sensors.right, s.sensors.dust].map(Number).join(" ");
      [s.tick, "(" + s.x + "," + s.y + ")", s.heading, inp, sv, s.state, s.motor, s.cleaner, s.turnCount, s.cleanupCount, s.dustDuringTurn ? 1 : 0, s.changes.join("; "), s.checks.join("; ")]
        .forEach((v) => { const td = document.createElement("td"); td.textContent = v; tr.appendChild(td); });
      tr.addEventListener("click", () => { pause(); view = i; render(); });
      body.appendChild(tr);
    });
    t.appendChild(body);
    const cur = body.children[view];
    if (cur && timer) cur.scrollIntoView({ block: "nearest" });
  }

  function updateButtons() {
    const has = sim.history.length > 0;
    const atEnd = has && sim.finished && view === sim.history.length - 1;
    $("btn-start").disabled = !has || !!timer || atEnd;
    $("btn-pause").disabled = !timer;
    $("btn-step").disabled = !has || atEnd;
    $("btn-back").disabled = !has || view === 0;
    $("btn-reset").disabled = !has && Sim.validate(config).length > 0;
    $("btn-restart").disabled = Sim.validate(config).length > 0;
  }

  function renderScenarioInfo() {
    const s = scenario;
    const li = (arr) => "<ul>" + arr.map((x) => "<li>" + x + "</li>").join("") + "</ul>";
    $("scenario-info").innerHTML = "<dl>" +
      "<dt>ID</dt><dd>" + s.id + "</dd><dt>이름</dt><dd>" + s.name + "</dd>" +
      "<dt>시작</dt><dd>(" + s.start.x + "," + s.start.y + ") " + s.start.heading + " · 최대 100 tick · 좌/우/먼지 주기 " + (s.sidePeriod || 1) + "</dd>" +
      "<dt>검증 기능</dt><dd>" + li(s.functions || []) + "</dd>" +
      "<dt>요구사항</dt><dd>" + (s.requirements || []).join(", ") + "</dd>" +
      "<dt>SA 산출물</dt><dd>" + (s.artifacts || []).join(", ") + "</dd>" +
      "<dt>기대 동작</dt><dd>" + (s.expected || "") + "</dd></dl>";
  }

  // ---------------- map editing ----------------
  function editCell(x, y) {
    if (!$("edit-mode").checked) return;
    const tool = document.querySelector("input[name=tool]:checked").value;
    const g = config.map.map((r) => r.split(""));
    if (tool === "start") { config.start.x = x; config.start.y = y; }
    else if (tool === "obstacle") g[y][x] = g[y][x] === "#" ? "." : "#";
    else if (tool === "dust") { const c = g[y][x]; g[y][x] = c === "#" ? "#" : c === "." ? "1" : c === "9" ? "." : String(Number(c) + 1); }
    else g[y][x] = ".";
    config.map = g.map((r) => r.join(""));
    resetSim();   // the edited map takes effect from tick 0; invalid maps are reported, not changed
  }

  function saveCustom() {
    const name = $("custom-name").value.trim() || "사용자 지도";
    if (Sim.validate(config).length) { alert("지도 설정 오류를 먼저 고쳐 주세요."); return; }
    const n = customScenarios.length + 1;
    const s = Object.assign(clone(config), {
      id: "CU-" + String(n).padStart(2, "0") + "-" + Date.now().toString(36).slice(-4), name, custom: true,
      functions: ["(평가자가 작성)"], requirements: [], artifacts: [],
      expected: "사용자 지도. 기대 동작은 평가자가 요구사항(PR-x)에 따라 판단합니다.",
    });
    customScenarios.push(s);
    store.set(LS_CUSTOM, customScenarios);
    fillScenarioSelect(s.id);
    loadScenario(s.id);
  }

  // ---------------- human evaluation and export ----------------
  function evaluations() { return store.get(LS_EVAL, {}); }
  function loadEvaluation() {
    const e = evaluations()[scenario.id] || {};
    $("eval-result").value = e.result || ""; $("eval-evaluator").value = e.evaluator || "";
    $("eval-ticks").value = e.ticks || ""; $("eval-notes").value = e.notes || "";
    $("eval-saved").textContent = e.savedAt ? "저장됨 " + e.savedAt : "";
  }
  function saveEvaluation() {
    const all = evaluations();
    const flags = sim.history.reduce((n, s) => n + s.checks.length, 0);
    all[scenario.id] = { result: $("eval-result").value, evaluator: $("eval-evaluator").value, ticks: $("eval-ticks").value,
      notes: $("eval-notes").value, savedAt: new Date().toLocaleString(), ticksRun: sim.tick, checkFlags: flags, name: scenario.name,
      requirements: scenario.requirements || [] };
    const ok = store.set(LS_EVAL, all);
    $("eval-saved").textContent = ok ? "저장됨 " + all[scenario.id].savedAt : "브라우저 저장소를 쓸 수 없어 저장하지 못했습니다. 결과를 .md로 내보내 주세요.";
  }
  function download(name, text, type) {
    const a = document.createElement("a");
    a.href = URL.createObjectURL(new Blob([text], { type }));
    a.download = name; document.body.appendChild(a); a.click(); a.remove();
  }
  function exportResults() {
    const all = evaluations();
    let md = "# RVC System Test Results\n\n내보낸 시각: " + new Date().toLocaleString() + "\n\n| Scenario | Name | Requirements | Human Result | Evaluator | Related Ticks | Ticks Run | Check Flags | Observations |\n|---|---|---|---|---|---|---|---|---|\n";
    for (const s of allScenarios()) {
      const e = all[s.id] || {};
      md += "| " + [s.id, s.name, (s.requirements || []).join(", "), e.result || "Pending", e.evaluator || "", e.ticks || "", e.ticksRun ?? "", e.checkFlags ?? "", (e.notes || "").replace(/\n/g, " ")].map((v) => String(v).replace(/\|/g, "/")).join(" | ") + " |\n";
    }
    download("rvc-system-test-results.md", md, "text/markdown");
  }
  function exportTrace() {
    download(scenario.id + "-trace.json", JSON.stringify({ scenario: { id: scenario.id, name: scenario.name }, configuration: config, history: sim.history }, null, 1), "application/json");
  }

  // ---------------- wiring ----------------
  $("scenario-select").addEventListener("change", (e) => loadScenario(e.target.value));
  $("btn-start").addEventListener("click", play);
  $("btn-pause").addEventListener("click", pause);
  $("btn-step").addEventListener("click", () => { pause(); stepOnce(); });
  $("btn-back").addEventListener("click", () => { pause(); if (view > 0) { view--; render(); } });
  $("btn-reset").addEventListener("click", resetSim);
  $("btn-restart").addEventListener("click", () => { if (resetSim()) play(); });
  $("speed").addEventListener("input", (e) => { $("speed-label").textContent = e.target.value + " ms/tick"; if (timer) { pause(); play(); } });
  $("edit-mode").addEventListener("change", (e) => { $("edit-tools").hidden = !e.target.checked; document.body.classList.toggle("editing", e.target.checked); if (e.target.checked) resetSim(); else render(); });
  $("start-heading").addEventListener("change", (e) => { config.start.heading = e.target.value; resetSim(); });
  $("side-period").addEventListener("change", (e) => { config.sidePeriod = Number(e.target.value); resetSim(); });
  $("clear-map").addEventListener("click", () => { config.map = Array(10).fill(".........."); resetSim(); });
  $("restore-map").addEventListener("click", () => loadScenario(scenario.id));
  $("save-custom").addEventListener("click", saveCustom);
  $("eval-save").addEventListener("click", saveEvaluation);
  $("export-results").addEventListener("click", exportResults);
  $("export-trace").addEventListener("click", exportTrace);
  $("export-scenarios").addEventListener("click", () => download("rvc-custom-scenarios.json", JSON.stringify(customScenarios, null, 1), "application/json"));
  $("import-scenarios").addEventListener("change", async (e) => {
    const f = e.target.files[0]; if (!f) return;
    try {
      const list = JSON.parse(await f.text());
      const ok = (Array.isArray(list) ? list : [list]).filter((s) => s && s.id && Sim.validate(s).length === 0);
      for (const s of ok) { s.custom = true; customScenarios = customScenarios.filter((c) => c.id !== s.id).concat([s]); }
      store.set(LS_CUSTOM, customScenarios);
      fillScenarioSelect(ok.length ? ok[0].id : scenario.id);
      if (ok.length) loadScenario(ok[0].id);
      alert(ok.length + "개 시나리오를 불러왔습니다.");
    } catch (err) { alert("불러오기 실패: " + err.message); }
    e.target.value = "";
  });
  $("delete-custom").addEventListener("click", () => {
    if (!scenario.custom) { alert("미리 정의된 시나리오는 삭제할 수 없습니다."); return; }
    customScenarios = customScenarios.filter((s) => s.id !== scenario.id);
    store.set(LS_CUSTOM, customScenarios);
    fillScenarioSelect(); loadScenario(allScenarios()[0].id);
  });

  fillScenarioSelect();
  loadScenario(allScenarios()[0].id);
})();
