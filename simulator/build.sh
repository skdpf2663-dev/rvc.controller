#!/usr/bin/env bash
# Builds the web simulator: compiles the unchanged C11 controller (src/, without
# main.c) to WebAssembly with clang + wasm-ld, then inlines it into one HTML file
# (simulator/dist/rvc-simulator.html) that opens directly in a browser.
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p simulator/dist
clang --target=wasm32 -std=c11 -O2 -Wall -Wextra -nostdlib -ffreestanding \
  -Iinclude -Wl,--no-entry -Wl,--strip-all -o simulator/dist/rvc.wasm \
  src/sensor_store.c src/capture_sensor_values.c src/generate_tick.c \
  src/actuator.c src/controller.c src/rvc_main.c simulator/wasm/rvc_wasm.c
python3 - <<'PY'
import base64
wasm = base64.b64encode(open("simulator/dist/rvc.wasm", "rb").read()).decode()
html = open("simulator/web/index.html", encoding="utf-8").read()
js = open("simulator/web/simulator.js", encoding="utf-8").read()
scen = open("simulator/web/scenarios.js", encoding="utf-8").read()
core = open("simulator/web/sim-core.js", encoding="utf-8").read()
css = open("simulator/web/style.css", encoding="utf-8").read()
out = (html.replace("/*@@STYLE@@*/", css)
           .replace("/*@@WASM@@*/", 'const RVC_WASM_BASE64 = "' + wasm + '";')
           .replace("/*@@SIMCORE@@*/", core)
           .replace("/*@@SCENARIOS@@*/", scen)
           .replace("/*@@APP@@*/", js))
open("simulator/dist/rvc-simulator.html", "w", encoding="utf-8").write(out)
print("built simulator/dist/rvc-simulator.html (%d bytes, wasm %d bytes)" % (len(out), len(wasm) * 3 // 4))
PY
