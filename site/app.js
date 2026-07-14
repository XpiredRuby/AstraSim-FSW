const MODEL_URL = "model.json";
const UINT32_MOD = 0x1_0000_0000;
const UINT32_HALF = 0x8000_0000;

const $ = (id) => document.getElementById(id);
const sleep = (ms) => new Promise((resolve) => setTimeout(resolve, ms));

let model;
let state;
let scenarioRunning = false;

function makeInitialState() {
  return {
    mode: "BOOT",
    fault: "NONE",
    heartbeat: 0,
    telemetrySequence: 0,
    highestCommandSequence: null,
    lastCommand: "NONE",
    lastStatus: "NO_ACK",
    lastAckSequence: 0,
    recoveryAttempts: 0,
    recoveryActive: false,
    health: {
      cpuLoad: 20,
      memoryLoad: 35,
      sensorAgeMs: 10,
      payloadAgeMs: 20,
      loopDurationMs: 25
    },
    history: [],
    events: []
  };
}

function sequenceIsNewer(candidate, reference) {
  const distance = (candidate - reference + UINT32_MOD) % UINT32_MOD;
  return distance !== 0 && distance < UINT32_HALF;
}

function nowLabel() {
  return new Date().toLocaleTimeString([], { hour12: false });
}

function addEvent(kind, message, severity = "info") {
  state.events.unshift({ kind, message, severity, time: nowLabel() });
  state.events = state.events.slice(0, 120);
  renderEventLog();
}

function acknowledge(command, status, sequence, detail = "") {
  state.lastCommand = command;
  state.lastStatus = status;
  state.lastAckSequence = sequence;
  const severity = status === "ACCEPTED" ? "info" : (status.includes("INVALID") || status.includes("STALE") || status.includes("FUTURE") ? "warning" : "error");
  addEvent("COMMAND", `${command} seq=${sequence} → ${status}${detail ? ` · ${detail}` : ""}`, severity);
}

function recordTelemetry(source = "STEP") {
  state.heartbeat += 1;
  state.telemetrySequence += 1;
  state.history.push({
    tick: state.telemetrySequence,
    mode: state.mode,
    fault: state.fault,
    source
  });
  state.history = state.history.slice(-60);
  renderAll();
}

function transitionAllowed(from, to) {
  return model.transitions[from]?.includes(to) ?? false;
}

function forceMode(nextMode, reason) {
  const before = state.mode;
  state.mode = nextMode;
  if (before !== nextMode) {
    addEvent("MODE", `${before} → ${nextMode}${reason ? ` · ${reason}` : ""}`);
  }
}

function applyFault(faultName, origin = "GROUND") {
  const fault = model.faults[faultName];
  if (!fault) return { status: "REJECTED_BAD_ARGUMENT", detail: "Unknown fault" };

  const previousFault = state.fault;
  state.fault = faultName;
  if (previousFault !== faultName) {
    addEvent("FDIR", `${previousFault} → ${faultName} · ${fault.response} · origin=${origin}`, fault.severity === "CRITICAL" ? "error" : "warning");
  }

  if (fault.targetMode) {
    if (transitionAllowed(state.mode, fault.targetMode)) {
      forceMode(fault.targetMode, faultName);
    } else if (state.mode !== "SAFE") {
      forceMode("SAFE", `${faultName} · invalid primary transition, SAFE fallback`);
    }
  }
  return { status: "ACCEPTED", detail: `${fault.severity} / ${fault.response}` };
}

function policyAllows(command, argument) {
  if (command === "SET_MODE" && !$('allowSetMode').checked) return "SET_MODE disabled by policy";
  if (command === "INJECT_FAULT" && !$('allowFaultInjection').checked) return "Fault injection disabled by policy";
  if (command === "CLEAR_FAULT" && !$('allowFaultClear').checked) return "Fault clear disabled by policy";
  if (command === "SET_MODE" && argument === "TEST" && !$('allowTestMode').checked) return "TEST mode disabled by policy";
  return null;
}

function processSetMode(argument) {
  if (!model.transitions[argument]) {
    return { status: "REJECTED_BAD_ARGUMENT", detail: "Unknown mode" };
  }

  if (state.mode === "RECOVERY" && argument !== "RECOVERY" && !transitionAllowed("RECOVERY", argument)) {
    state.recoveryAttempts += 1;
    if (state.recoveryAttempts >= model.recoverySupervisor.maximumFailedExits) {
      forceMode("SAFE", "Recovery attempt limit reached");
      state.recoveryActive = false;
      return { status: "REJECTED_RECOVERY_LIMIT", detail: `attempt ${state.recoveryAttempts}/${model.recoverySupervisor.maximumFailedExits}` };
    }
    return { status: "REJECTED_INVALID_TRANSITION", detail: `recovery attempt ${state.recoveryAttempts}/${model.recoverySupervisor.maximumFailedExits}` };
  }

  if (!transitionAllowed(state.mode, argument)) {
    return { status: "REJECTED_INVALID_TRANSITION", detail: `${state.mode} → ${argument}` };
  }

  const before = state.mode;
  forceMode(argument, "SET_MODE");
  if (before === "SAFE" && argument === "RECOVERY") {
    state.recoveryActive = true;
    state.recoveryAttempts = 0;
  } else if (before === "RECOVERY" && argument !== "RECOVERY") {
    state.recoveryActive = false;
    state.recoveryAttempts = 0;
  }
  return { status: "ACCEPTED", detail: `${before} → ${argument}` };
}

function processCommand(command, argument) {
  switch (command) {
    case "NOOP":
      return { status: "ACCEPTED", detail: "No operation" };
    case "REQUEST_TELEMETRY":
      return { status: "ACCEPTED", detail: "Telemetry snapshot requested" };
    case "CLEAR_FAULT": {
      const before = state.fault;
      state.fault = "NONE";
      addEvent("FDIR", `${before} → NONE · ground clear`);
      return { status: "ACCEPTED", detail: "Fault field cleared" };
    }
    case "SET_MODE":
      return processSetMode(argument);
    case "INJECT_FAULT":
      return applyFault(argument, "GROUND");
    default:
      return { status: "REJECTED_UNKNOWN_COMMAND", detail: "Unknown command" };
  }
}

function sendCommand({ command, argument = null, sequence, timestampOffsetMs = 0 }) {
  const seq = Number(sequence) >>> 0;
  const offset = Number(timestampOffsetMs) || 0;

  if (offset < -model.guard.maximumAgeMs) {
    acknowledge(command, "REJECTED_STALE_TIMESTAMP", seq, `age=${Math.abs(offset)} ms`);
    recordTelemetry("COMMAND");
    return "REJECTED_STALE_TIMESTAMP";
  }
  if (offset > model.guard.maximumFutureSkewMs) {
    acknowledge(command, "REJECTED_FUTURE_TIMESTAMP", seq, `skew=${offset} ms`);
    recordTelemetry("COMMAND");
    return "REJECTED_FUTURE_TIMESTAMP";
  }

  if (state.highestCommandSequence !== null) {
    if (seq === state.highestCommandSequence) {
      acknowledge(command, "REJECTED_DUPLICATE_SEQUENCE", seq);
      recordTelemetry("COMMAND");
      return "REJECTED_DUPLICATE_SEQUENCE";
    }
    if (!sequenceIsNewer(seq, state.highestCommandSequence)) {
      acknowledge(command, "REJECTED_REPLAYED_SEQUENCE", seq);
      recordTelemetry("COMMAND");
      return "REJECTED_REPLAYED_SEQUENCE";
    }
  }

  state.highestCommandSequence = seq;
  const policyRejection = policyAllows(command, argument);
  if (policyRejection) {
    acknowledge(command, "REJECTED_UNAUTHORIZED", seq, policyRejection);
    recordTelemetry("COMMAND");
    return "REJECTED_UNAUTHORIZED";
  }

  const result = processCommand(command, argument);
  acknowledge(command, result.status, seq, result.detail);
  recordTelemetry("COMMAND");
  return result.status;
}

function classifyHealth() {
  const candidates = [];
  const h = state.health;
  const t = model.healthThresholds;
  if (h.loopDurationMs >= t.watchdogExpiredMs) candidates.push("WATCHDOG_DEADLINE_MISS");
  if (h.sensorAgeMs >= t.sensorCriticalAgeMs) candidates.push("SENSOR_TIMEOUT");
  if (h.payloadAgeMs >= t.payloadCriticalAgeMs) candidates.push("PAYLOAD_HEARTBEAT_TIMEOUT");
  if (h.cpuLoad >= t.cpuCritical) candidates.push("CPU_OVERLOAD");
  if (h.memoryLoad >= t.memoryCritical) candidates.push("MEMORY_OVERLOAD");
  return candidates.sort((a, b) => model.faults[b].priority - model.faults[a].priority);
}

function advanceTick() {
  const candidates = classifyHealth();
  if (candidates.length > 0) {
    applyFault(candidates[0], "INTERNAL");
    if (candidates.length > 1) {
      addEvent("FDIR", `Primary=${candidates[0]} · suppressed=${candidates.slice(1).join(", ")}`, "warning");
    }
  } else {
    const h = state.health;
    const t = model.healthThresholds;
    if (h.cpuLoad >= t.cpuWarning || h.memoryLoad >= t.memoryWarning || h.loopDurationMs >= t.watchdogWarningMs) {
      addEvent("HEALTH", "Warning threshold active; no fault injected", "warning");
    } else {
      addEvent("HEALTH", "Nominal health evaluation");
    }
  }
  recordTelemetry("TICK");
}

function setHealthValues(values) {
  Object.assign(state.health, values);
  syncHealthControls();
  renderAll();
}

async function runScenario(id) {
  if (scenarioRunning) return;
  const scenario = model.scenarios.find((item) => item.id === id);
  if (!scenario) return;
  scenarioRunning = true;
  $('runScenarioButton').disabled = true;
  addEvent("SCENARIO", `START ${scenario.name}`);
  let sequence = state.highestCommandSequence === null ? 1 : ((state.highestCommandSequence + 1) >>> 0);

  for (const step of scenario.steps) {
    if (step.set) {
      setHealthValues(step.set);
      addEvent("SCENARIO", `SET ${Object.entries(step.set).map(([k, v]) => `${k}=${v}`).join(" ")}`);
    } else if (step.tick) {
      advanceTick();
    } else if (step.command) {
      sendCommand({
        command: step.command,
        argument: step.argument ?? null,
        sequence,
        timestampOffsetMs: step.timestampOffsetMs ?? 0
      });
      sequence = (sequence + 1) >>> 0;
    }
    await sleep(420);
  }
  addEvent("SCENARIO", `END ${scenario.name}`);
  scenarioRunning = false;
  $('runScenarioButton').disabled = false;
}

function updateArgumentOptions() {
  const command = $('commandSelect').value;
  const select = $('argumentSelect');
  select.innerHTML = "";
  let values = [];
  if (command === "SET_MODE") values = model.modes.map((mode) => mode.name);
  if (command === "INJECT_FAULT") values = Object.keys(model.faults).filter((fault) => fault !== "NONE");

  if (values.length === 0) {
    select.disabled = true;
    const option = document.createElement("option");
    option.textContent = "No argument";
    option.value = "";
    select.append(option);
  } else {
    select.disabled = false;
    values.forEach((value) => {
      const option = document.createElement("option");
      option.value = value;
      option.textContent = value;
      select.append(option);
    });
  }
}

function nextSuggestedSequence() {
  return state.highestCommandSequence === null ? 1 : ((state.highestCommandSequence + 1) >>> 0);
}

function renderEventLog() {
  const log = $('eventLog');
  log.innerHTML = "";
  state.events.forEach((event) => {
    const li = document.createElement("li");
    li.className = `event-${event.severity}`;
    li.innerHTML = `<span class="event-time">${event.time}</span><span class="event-kind">${event.kind}</span><span class="event-message"></span>`;
    li.querySelector('.event-message').textContent = event.message;
    log.append(li);
  });
}

function renderEvidence() {
  const labels = {
    nativeCTest: "Native CTest",
    sanitizerCTest: "Sanitizer CTest",
    pythonTests: "Python tooling",
    scenarios: "Deterministic scenarios",
    fdir: "FDIR campaign",
    monteCarlo: "Monte Carlo",
    protocol: "Protocol checks",
    permissionEval: "Permission cases",
    requirementFailures: "Requirement failures",
    traceabilityProblems: "Traceability problems"
  };
  const grid = $('evidenceGrid');
  grid.innerHTML = "";
  Object.entries(model.verification).forEach(([key, value]) => {
    const card = document.createElement("article");
    card.className = "evidence-card";
    card.innerHTML = `<span>${labels[key] ?? key}</span><strong>${value}</strong>`;
    grid.append(card);
  });
}

function syncHealthControls() {
  $('cpuLoad').value = state.health.cpuLoad;
  $('memoryLoad').value = state.health.memoryLoad;
  $('sensorAge').value = state.health.sensorAgeMs;
  $('payloadAge').value = state.health.payloadAgeMs;
  $('loopDuration').value = state.health.loopDurationMs;
  $('cpuOutput').textContent = `${state.health.cpuLoad}%`;
  $('memoryOutput').textContent = `${state.health.memoryLoad}%`;
  $('sensorOutput').textContent = `${state.health.sensorAgeMs} ms`;
  $('payloadOutput').textContent = `${state.health.payloadAgeMs} ms`;
  $('loopOutput').textContent = `${state.health.loopDurationMs} ms`;
}

function renderChart() {
  const canvas = $('telemetryChart');
  const rect = canvas.getBoundingClientRect();
  const ratio = Math.min(window.devicePixelRatio || 1, 2);
  canvas.width = Math.max(620, Math.round(rect.width * ratio));
  canvas.height = Math.round(260 * ratio);
  const ctx = canvas.getContext('2d');
  ctx.scale(ratio, ratio);
  const width = canvas.width / ratio;
  const height = canvas.height / ratio;
  ctx.clearRect(0, 0, width, height);

  const padding = { left: 52, right: 18, top: 18, bottom: 34 };
  const plotW = width - padding.left - padding.right;
  const plotH = height - padding.top - padding.bottom;
  const modes = model.modes.map((item) => item.name);

  ctx.strokeStyle = 'rgba(145,180,220,.15)';
  ctx.fillStyle = '#7890a8';
  ctx.font = '10px system-ui';
  modes.forEach((mode, index) => {
    const y = padding.top + (index / (modes.length - 1)) * plotH;
    ctx.beginPath(); ctx.moveTo(padding.left, y); ctx.lineTo(width - padding.right, y); ctx.stroke();
    ctx.fillText(mode.replace('DEGRADED_', 'D_'), 3, y + 3);
  });

  if (state.history.length === 0) {
    ctx.fillStyle = '#9eb2c9';
    ctx.font = '13px system-ui';
    ctx.fillText('Transmit a command or advance a tick to generate telemetry.', padding.left + 10, height / 2);
    return;
  }

  const points = state.history;
  const xFor = (i) => padding.left + (points.length === 1 ? plotW / 2 : (i / (points.length - 1)) * plotW);
  const yFor = (mode) => padding.top + (modes.indexOf(mode) / (modes.length - 1)) * plotH;

  ctx.lineWidth = 2;
  ctx.strokeStyle = '#5ee7ff';
  ctx.beginPath();
  points.forEach((point, index) => {
    const x = xFor(index); const y = yFor(point.mode);
    if (index === 0) ctx.moveTo(x, y); else ctx.lineTo(x, y);
  });
  ctx.stroke();

  points.forEach((point, index) => {
    const x = xFor(index); const y = yFor(point.mode);
    ctx.beginPath();
    ctx.fillStyle = point.fault === 'NONE' ? '#69f0ae' : '#ff6b7a';
    ctx.arc(x, y, point.fault === 'NONE' ? 3 : 5, 0, Math.PI * 2);
    ctx.fill();
  });

  ctx.fillStyle = '#7890a8';
  ctx.fillText(`tick ${points[0].tick}`, padding.left, height - 10);
  ctx.textAlign = 'right';
  ctx.fillText(`tick ${points.at(-1).tick}`, width - padding.right, height - 10);
  ctx.textAlign = 'left';
}

function renderAll() {
  const mode = state.mode;
  const fault = state.fault;
  const disposition = model.faults[fault];
  const allowed = model.transitions[mode].filter((item) => item !== mode);

  $('heroMode').textContent = mode;
  $('heroFault').textContent = `FAULT: ${fault}`;
  $('stripMode').textContent = mode;
  $('stripFault').textContent = fault;
  $('stripHeartbeat').textContent = state.heartbeat;
  $('stripAck').textContent = state.lastStatus === 'NO_ACK' ? '—' : state.lastStatus;
  $('stripSequence').textContent = state.telemetrySequence;

  $('modeValue').textContent = mode;
  $('faultValue').textContent = fault;
  $('faultDisposition').textContent = fault === 'NONE' ? 'No active response' : `${disposition.severity} · ${disposition.response}`;
  $('modeTransitionHint').textContent = `Allowed next: ${allowed.join(', ') || mode}`;
  $('recoveryAttempts').textContent = `${state.recoveryAttempts} / ${model.recoverySupervisor.maximumFailedExits}`;
  $('lastCommand').textContent = state.lastCommand;
  $('lastStatus').textContent = state.lastStatus === 'NO_ACK' ? 'No acknowledgement yet' : `${state.lastStatus} · seq ${state.lastAckSequence}`;

  const pill = $('modePill');
  pill.textContent = mode;
  pill.className = `mode-pill mode-${mode.toLowerCase()}`;
  $('sequenceInput').value = nextSuggestedSequence();
  syncHealthControls();
  renderEventLog();
  renderChart();
}

function resetSimulation() {
  state = makeInitialState();
  addEvent("SYSTEM", "ASTRA-OS browser model initialized");
  renderAll();
}

function wireControls() {
  $('commandSelect').addEventListener('change', updateArgumentOptions);
  $('sendButton').addEventListener('click', () => {
    sendCommand({
      command: $('commandSelect').value,
      argument: $('argumentSelect').disabled ? null : $('argumentSelect').value,
      sequence: $('sequenceInput').value,
      timestampOffsetMs: $('timestampOffset').value
    });
  });
  $('tickButton').addEventListener('click', advanceTick);
  $('resetButton').addEventListener('click', resetSimulation);
  $('clearLogButton').addEventListener('click', () => { state.events = []; renderEventLog(); });
  $('runScenarioButton').addEventListener('click', () => runScenario($('scenarioSelect').value));
  $('heroScenario').addEventListener('click', () => { document.querySelector('#console').scrollIntoView(); runScenario('nominal-fault-clear'); });

  const healthBindings = [
    ['cpuLoad', 'cpuLoad', 'cpuOutput', '%'],
    ['memoryLoad', 'memoryLoad', 'memoryOutput', '%'],
    ['sensorAge', 'sensorAgeMs', 'sensorOutput', ' ms'],
    ['payloadAge', 'payloadAgeMs', 'payloadOutput', ' ms'],
    ['loopDuration', 'loopDurationMs', 'loopOutput', ' ms']
  ];
  healthBindings.forEach(([inputId, key, outputId, suffix]) => {
    $(inputId).addEventListener('input', (event) => {
      state.health[key] = Number(event.target.value);
      $(outputId).textContent = `${state.health[key]}${suffix}`;
    });
  });
  window.addEventListener('resize', renderChart);
}

async function init() {
  const response = await fetch(MODEL_URL);
  if (!response.ok) throw new Error(`Unable to load ${MODEL_URL}: ${response.status}`);
  model = await response.json();
  state = makeInitialState();

  model.scenarios.forEach((scenario) => {
    const option = document.createElement('option');
    option.value = scenario.id;
    option.textContent = scenario.name;
    $('scenarioSelect').append(option);
  });
  renderEvidence();
  updateArgumentOptions();
  wireControls();
  addEvent("SYSTEM", `ASTRA-OS browser model v${model.version} initialized`);
  renderAll();
}

init().catch((error) => {
  console.error(error);
  document.body.insertAdjacentHTML('afterbegin', `<div class="noscript">Simulator initialization failed: ${error.message}</div>`);
});
