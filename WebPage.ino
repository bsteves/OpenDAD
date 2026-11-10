/******************************************************************************
  OpenDAD web interface

  This file contains only the HTML, CSS, and JavaScript for the browser-based
  display. The HTTP routes and instrument logic remain in WebServer.ino.
******************************************************************************/

const char WEB_PAGE[] PROGMEM = R"HTML(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta
    name="viewport"
    content="width=device-width, initial-scale=1.0"
  >

  <title>OpenDAD</title>

  <style>
    * {
      box-sizing: border-box;
    }

    body {
      margin: 0;
      padding: 18px;
      font-family: Arial, sans-serif;
      text-align: center;
      background: #151515;
      color: #ffffff;
    }

    .container {
      width: 100%;
      max-width: 560px;
      margin: 0 auto;
    }

    h1 {
      margin: 4px 0 2px;
      font-size: 2rem;
    }

    .subtitle {
      margin: 0;
      color: #aaaaaa;
    }

    /*
      The original angle display was 64 px. This is approximately 50% larger,
      while still shrinking appropriately on narrow phone screens.
    */
    .angle {
      margin: 8px 0 4px;
      font-size: clamp(54px, 15vw, 72px);
      line-height: 1;
      font-weight: bold;
      letter-spacing: -0.04em;
    }

    .raw {
      margin-bottom: 8px;
      color: #999999;
      font-size: 0.9rem;
    }

    .target-panel {
      margin-top: 8px;
      padding: 12px;
      border: 1px solid #3a3a3a;
      border-radius: 14px;
      background: #222222;
    }

    .target-label {
      color: #bdbdbd;
      font-size: 0.9rem;
      text-transform: uppercase;
      letter-spacing: 0.08em;
    }

    .target-angle {
      margin-top: 2px;
      font-size: 2.7rem;
      font-weight: bold;
    }

    .target-indices {
      min-height: 1.25em;
      margin-top: 4px;
      color: #cbd5e1;
      font-size: 1.35rem;
      line-height: 1.25;
      font-variant-numeric: tabular-nums;
    }

    .tier-navigation {
      display: flex;
      justify-content: center;
      align-items: center;
      gap: 10px;
      margin-top: 6px;
    }

    .tier-nav-button {
      width: 38px;
      height: 30px;
      padding: 0;
      background: #4b5563;
      color: #fff;
      font-size: 1.25rem;
      line-height: 1;
    }

    .tier-nav-button:disabled {
      opacity: .3;
      cursor: default;
    }

    .difference-label {
      margin-top: 7px;
      color: #bdbdbd;
      font-size: 0.9rem;
      text-transform: uppercase;
      letter-spacing: 0.08em;
    }

    /*
      The target difference is intentionally very prominent so it can be read
      while operating the faceting machine.
    */
    .difference {
      display: block;
      width: 100%;
      margin: 4px auto 8px;
      padding: 8px 6px;
      border-radius: 12px;
      font-size: clamp(39px, 12vw, 57px);
      line-height: 1;
      font-weight: bold;
      font-variant-numeric: tabular-nums;
      color: #ffffff;
    }

    .difference.unset {
      background: #555b65;
    }

    .difference.far {
      background: #b91c1c;
    }

    .difference.approaching {
      background: #c2410c;
    }

    .difference.close {
      background: #d4a800;
      color: #111111;
    }

    .difference.target {
      background: #15803d;
    }

    .target-buttons {
      display: flex;
      justify-content: center;
      gap: 10px;
      margin-top: 2px;
    }

    button {
      border: 0;
      border-radius: 8px;
      cursor: pointer;
      font-weight: bold;
      touch-action: manipulation;
    }

    button:active {
      transform: scale(0.98);
    }

    /*
      Target controls are deliberately smaller than the large display area.
    */
    .target-button {
      width: auto;
      min-width: 108px;
      margin: 0;
      padding: 9px 14px;
      font-size: 0.95rem;
      background: #2563eb;
      color: #ffffff;
    }

    .target-button.secondary {
      background: #5f6368;
    }

    .calibration-section {
      margin-top: 22px;
      padding-top: 16px;
      border-top: 1px solid #333333;
    }

    .section-label {
      margin-bottom: 9px;
      color: #aaaaaa;
      font-size: 0.85rem;
      text-transform: uppercase;
      letter-spacing: 0.08em;
    }

    /*
      These are the actual calibration controls. The former maroon 0°, 45°,
      and 90° status boxes have been removed.
    */
    .calibration-buttons {
      display: flex;
      flex-wrap: wrap;
      justify-content: center;
      gap: 8px;
    }

    .cal-button,
    .reset-button {
      width: auto;
      min-width: 82px;
      margin: 0;
      padding: 9px 11px;
      font-size: 0.9rem;
    }

    .cal-button {
      background: #e5e7eb;
      color: #111111;
    }

    .reset-button {
      background: #8f2929;
      color: #ffffff;
    }

    #connection {
      margin-top: 18px;
      color: #aaaaaa;
      font-size: 0.9rem;
    }


    .viewer-section {
      margin: 8px 0 14px;
      padding: 8px;
      border: 1px solid #3a3a3a;
      border-radius: 14px;
      background: #222222;
    }

    .viewer-section .diagram-panel {
      padding: 0;
      background: transparent;
    }

    .viewer-section #gem3d {
      height: clamp(180px, 30vh, 230px);
    }

    .design-section {
      margin-top: 22px;
      padding: 16px;
      border: 1px solid #3a3a3a;
      border-radius: 14px;
      background: #222222;
      text-align: left;
    }

    .design-upload {
      display: flex;
      flex-wrap: wrap;
      align-items: center;
      gap: 10px;
      margin-bottom: 12px;
    }

    .file-input {
      width: 100%;
      padding: 10px;
      border: 1px dashed #666;
      border-radius: 8px;
      color: #ddd;
      background: #181818;
    }

    .design-meta {
      color: #aaa;
      font-size: .85rem;
      margin: 8px 0 14px;
      overflow-wrap: anywhere;
    }

    .design-content {
      display: grid;
      grid-template-columns: minmax(0, 1fr) minmax(0, 1fr);
      gap: 12px;
      align-items: start;
    }

    .diagram-panel {
      min-width: 0;
      padding: 8px;
      background: #181818;
      border-radius: 10px;
    }

    .gem-view-wrap {
      position: relative;
      width: 100%;
      overflow: hidden;
      border: 1px solid #333;
      border-radius: 8px;
      background: radial-gradient(ellipse at 50% 42%, #263442 0%, #111820 72%);
      touch-action: none;
    }

    #gem3d {
      display: block;
      width: 100%;
      height: 360px;
      cursor: grab;
      touch-action: none;
    }
    #gem3d:active { cursor: grabbing; }

    .gem-view-controls {
      display: flex;
      align-items: center;
      justify-content: space-between;
      gap: 8px;
      flex-wrap: wrap;
      margin-top: 8px;
      color: #999;
      font-size: .72rem;
    }
    .gem-view-controls button {
      border: 1px solid #52677a;
      border-radius: 6px;
      padding: 5px 9px;
      color: #ddd;
      background: #263442;
      cursor: pointer;
    }

    .diagram-caption {
      color: #999;
      font-size: .75rem;
      text-align: center;
      margin-top: 5px;
    }

    .facet-groups {
      min-width: 0;
    }

    .facet-group-title {
      margin: 5px 0 7px;
      color: #aaa;
      font-size: .75rem;
      text-transform: uppercase;
      letter-spacing: .08em;
    }

    .facet-row {
      display: flex;
      align-items: center;
      justify-content: space-between;
      gap: 8px;
      width: 100%;
      margin: 0 0 6px;
      padding: 10px;
      border: 1px solid #444;
      background: #303030;
      color: #fff;
      text-align: left;
      font-size: .95rem;
    }

    .facet-row.selected {
      border-color: #60a5fa;
      background: #164e80;
    }

    .facet-row:disabled {
      opacity: .45;
      cursor: default;
    }

    .facet-values {
      display: flex;
      flex-direction: column;
      align-items: flex-end;
      gap: 3px;
      min-width: 0;
    }

    .facet-angle {
      font-variant-numeric: tabular-nums;
      font-weight: bold;
    }

    .facet-indices {
      color: #cbd5e1;
      font-size: .68rem;
      line-height: 1.25;
      font-variant-numeric: tabular-nums;
      text-align: right;
      overflow-wrap: anywhere;
    }

    #designStatus {
      min-height: 1.2em;
      margin-top: 10px;
      color: #aaa;
      font-size: .82rem;
    }

    @media (max-width: 430px) {
      .design-content {
        grid-template-columns: 1fr;
      }
      .viewer-section #gem3d { height: min(30vh, 210px); min-height: 165px; }
    }

    @media (max-width: 390px) {
      body {
        padding: 12px;
      }

      .target-panel {
        padding: 10px;
      }

      .target-buttons {
        gap: 7px;
      }

      .target-button {
        min-width: 96px;
        padding: 8px 10px;
      }

      .cal-button,
      .reset-button {
        min-width: 72px;
        padding: 8px;
        font-size: 0.82rem;
      }
    }
  </style>
</head>

<body>
  <div class="container">
    <div id="angle" class="angle">--.--°</div>

    <div class="raw">
      Raw sensor angle:
      <span id="rawAngle">--.--</span>°
    </div>

    <div class="target-panel">
      <div class="difference-label">Difference</div>
      <strong
        id="angleDifference"
        class="difference unset"
      >—</strong>

      <div class="target-label">Target angle</div>
      <div id="targetAngle" class="target-angle">Not set</div>
      <div id="targetIndices" class="target-indices"></div>
      <div class="tier-navigation" aria-label="Select adjacent facet tier">
        <button id="previousTierButton" class="tier-nav-button"
                type="button" title="Previous tier" aria-label="Previous tier" disabled>&lt;</button>
        <button id="nextTierButton" class="tier-nav-button"
                type="button" title="Next tier" aria-label="Next tier" disabled>&gt;</button>
      </div>

      <div class="target-buttons">
        <button id="setTargetButton" class="target-button">
          Set Target
        </button>

        <button
          id="clearTargetButton"
          class="target-button secondary"
        >
          Clear
        </button>
      </div>
    </div>

    <section class="viewer-section" aria-label="3D gemstone visualization">
      <div class="diagram-panel">
        <div class="gem-view-wrap">
          <canvas id="gem3d" aria-label="Interactive rotatable 3D gemstone model">
            Your browser does not support canvas rendering.
          </canvas>
        </div>
        <div class="gem-view-controls">
          <span>Drag to rotate · scroll/pinch to zoom</span>
          <button type="button" id="resetGemView">Reset view</button>
        </div>
        <div id="diagramCaption" class="diagram-caption">
          Interactive 3D GCS facet model
        </div>
      </div>
    </section>

    <section class="design-section">
      <h1>OpenDAD</h1>
      <p class="subtitle">Digital Angle Display</p>
      <div class="section-label">Faceting design</div>
      <div class="design-upload">
        <input
          id="designFile"
          class="file-input"
          type="file"
        >
      </div>
      <div id="designMeta" class="design-meta">
        Load a GemCad ASC or Gem Cut Studio GCS design.
      </div>
      <div class="design-content">
        <div id="facetGroups" class="facet-groups">
          <div class="diagram-caption">Facet tiers will appear here.</div>
        </div>
      </div>
      <div id="designStatus" aria-live="polite"></div>
    </section>

    <div class="calibration-section">
      <div class="section-label">Calibration · Set mast to each reference angle</div>

      <div class="calibration-buttons">
        <button class="cal-button" data-command="/api/calibrate/0">
          Set 0°
        </button>

        <button class="cal-button" data-command="/api/calibrate/10">
          Set 10°
        </button>

        <button class="cal-button" data-command="/api/calibrate/20">
          Set 20°
        </button>

        <button class="cal-button" data-command="/api/calibrate/30">
          Set 30°
        </button>

        <button class="cal-button" data-command="/api/calibrate/40">
          Set 40°
        </button>

        <button class="cal-button" data-command="/api/calibrate/50">
          Set 50°
        </button>

        <button class="cal-button" data-command="/api/calibrate/60">
          Set 60°
        </button>

        <button class="cal-button" data-command="/api/calibrate/70">
          Set 70°
        </button>

        <button class="cal-button" data-command="/api/calibrate/80">
          Set 80°
        </button>

        <button class="cal-button" data-command="/api/calibrate/90">
          Set 90°
        </button>

        <button class="reset-button" data-command="/api/reset">
          Reset
        </button>
      </div>
    </div>

    <div id="connection">Connecting...</div>
  </div>

  <script>
    const angleElement =
      document.getElementById("angle");

    const rawAngleElement =
      document.getElementById("rawAngle");

    const targetElement =
      document.getElementById("targetAngle");

    const differenceElement =
      document.getElementById("angleDifference");

    const targetIndicesElement =
      document.getElementById("targetIndices");

    const connectionElement =
      document.getElementById("connection");

    const previousTierButton = document.getElementById("previousTierButton");
    const nextTierButton = document.getElementById("nextTierButton");


    async function updateStatus()
    {
      try
      {
        const response = await fetch("/api/status", {
          cache: "no-store"
        });

        if (!response.ok)
        {
          throw new Error("Status request failed");
        }

        const status = await response.json();

        angleElement.textContent =
          Number(status.angle).toFixed(2) + "°";

        rawAngleElement.textContent =
          Number(status.rawAngle).toFixed(2);

        if (!status.targetSet)
        {
          targetElement.textContent = "Not set";
          targetIndicesElement.textContent = "";
          differenceElement.textContent = "—";
          differenceElement.className = "difference unset";
        }
        else
        {
          targetElement.textContent =
            Number(status.target).toFixed(2) + "°";

          const difference = Number(status.difference);
          const absoluteDifference = Math.abs(difference);

          differenceElement.textContent =
            (difference >= 0 ? "+" : "") +
            difference.toFixed(2) +
            "°";

          if (absoluteDifference <= 0.05)
          {
            differenceElement.className = "difference target";
          }
          else if (absoluteDifference <= 0.20)
          {
            differenceElement.className = "difference close";
          }
          else if (absoluteDifference <= 0.50)
          {
            differenceElement.className = "difference approaching";
          }
          else
          {
            differenceElement.className = "difference far";
          }
        }

        connectionElement.textContent = status.sensorReady
          ? "OpenDAD connected"
          : "Waiting for sensor data";
      }
      catch (error)
      {
        console.error("Status update failed:", error);
        connectionElement.textContent = "Connection lost";
      }
    }


    async function setTarget()
    {
      try
      {
        const response = await fetch("/api/target", {
          method: "POST"
        });

        if (!response.ok)
        {
          throw new Error("Unable to set target");
        }

        targetIndicesElement.textContent = "";
        selectedFacetName = null;
        renderFacetList();
        await updateStatus();
      }
      catch (error)
      {
        console.error(error);
      }
    }


    async function clearTarget()
    {
      try
      {
        const response = await fetch("/api/target/clear", {
          method: "POST"
        });

        if (!response.ok)
        {
          throw new Error("Unable to clear target");
        }

        targetIndicesElement.textContent = "";
        selectedFacetName = null;
        renderFacetList();
        await updateStatus();
      }
      catch (error)
      {
        console.error(error);
      }
    }


    async function sendCommand(path)
    {
      try
      {
        const response = await fetch(path, {
          method: "POST"
        });

        if (!response.ok)
        {
          throw new Error("Command failed");
        }

        await updateStatus();
      }
      catch (error)
      {
        console.error("Command failed:", error);
        connectionElement.textContent = "Command failed";
      }
    }


    document
      .querySelectorAll("[data-command]")
      .forEach(button =>
      {
        button.addEventListener("click", () =>
        {
          sendCommand(button.dataset.command);
        });
      });


    document
      .getElementById("setTargetButton")
      .addEventListener("click", setTarget);


    document
      .getElementById("clearTargetButton")
      .addEventListener("click", clearTarget);

    let wakeLock = null;

    async function requestWakeLock()
    {
      if (!("wakeLock" in navigator))
        return;

      try
      {
        wakeLock = await navigator.wakeLock.request("screen");
        console.log("Screen wake lock active");
      }
      catch(e)
      {
        console.log(e);
      }
    }

    window.addEventListener("load", requestWakeLock);

    document.addEventListener("visibilitychange", () =>
    {
      if (document.visibilityState === "visible")
      {
        requestWakeLock();
      }
    });






    // ---------------------------------------------------------------
    // Faceting design import: GemCad ASC and Gem Cut Studio GCS
    // ---------------------------------------------------------------

    const designFileInput = document.getElementById("designFile");
    const designMeta = document.getElementById("designMeta");
    const facetGroupsElement = document.getElementById("facetGroups");
    const designStatus = document.getElementById("designStatus");
    const gemCanvas = document.getElementById("gem3d");
    const gemContext = gemCanvas.getContext("2d");
    const diagramCaption = document.getElementById("diagramCaption");
    const resetGemViewButton = document.getElementById("resetGemView");

    let designFacets = [];
    let currentDesignGeometry = null;
    let selectedFacetName = null;

    function selectableFacets() {
      return designFacets.filter(facet => facet.name.toUpperCase() !== "T");
    }

    function updateTierNavigation() {
      const tiers = selectableFacets();
      const selectedIndex = tiers.findIndex(facet => facet.name === selectedFacetName);
      previousTierButton.disabled = selectedIndex <= 0;
      nextTierButton.disabled = selectedIndex < 0 || selectedIndex >= tiers.length - 1;
    }

    async function moveTier(offset) {
      const tiers = selectableFacets();
      const selectedIndex = tiers.findIndex(facet => facet.name === selectedFacetName);
      if (selectedIndex < 0) return;
      const nextIndex = selectedIndex + offset;
      if (nextIndex < 0 || nextIndex >= tiers.length) return;
      await selectDesignFacet(tiers[nextIndex]);
    }

    previousTierButton.addEventListener("click", () => moveTier(-1));
    nextTierButton.addEventListener("click", () => moveTier(1));

    function facetGroup(name) {
      if (/^G/i.test(name)) return "Girdle";
      if (/^[0-9]/.test(name)) return "Pavilion";
      return "Crown";
    }

    function normalizeIndices(values, gear) {
      const cleaned = values
        .filter(value => Number.isFinite(value))
        .map(value => ((Math.round(value) % gear) + gear) % gear);
      return Array.from(new Set(cleaned)).sort((a, b) => a - b);
    }

    function parseAsc(text) {
      const facets = [];
      let title = "";
      let indexGear = 96;

      for (const line of text.split(/\r?\n/)) {
        const header = line.match(/^H\s+(.+)$/);
        if (header && !title) title = header[1].trim();
        const gearMatch = line.match(/^g\s+(\d+)/i);
        if (gearMatch) indexGear = Number(gearMatch[1]) || indexGear;

        // GemCad tier record: a <angle> <depth> [indexes] n <name> [indexes] G ...
        if (!/^a\s/i.test(line)) continue;
        const tokens = line.trim().split(/\s+/);
        const nIndex = tokens.indexOf("n");
        if (nIndex < 0 || !tokens[nIndex + 1]) continue;

        const storedAngle = Number(tokens[1]);
        if (!Number.isFinite(storedAngle)) continue;

        const name = tokens[nIndex + 1];
        const indexValues = [];
        // Index positions can occur on either side of the "n <tier>" marker.
        for (const token of tokens.slice(3, nIndex)) {
          if (/^\d+$/.test(token)) indexValues.push(Number(token));
        }
        const gIndex = tokens.indexOf("G", nIndex + 2);
        const afterName = tokens.slice(nIndex + 2, gIndex < 0 ? tokens.length : gIndex);
        for (const token of afterName) {
          if (/^\d+$/.test(token)) indexValues.push(Number(token));
        }

        facets.push({
          name,
          angle: Math.abs(storedAngle),
          indices: normalizeIndices(indexValues, indexGear),
          group: facetGroup(name),
          instructions: ""
        });
      }

      return { title: title || "Imported ASC design", facets, geometry: null, format: "ASC", indexGear };
    }

    function parseGcs(text) {
      const xml = new DOMParser().parseFromString(text, "application/xml");
      if (xml.querySelector("parsererror")) {
        throw new Error("This GCS file could not be read as XML.");
      }

      const root = xml.querySelector("GemCutStudio");
      if (!root) throw new Error("This does not look like a Gem Cut Studio file.");

      const facets = [];
      const geometry = [];
      const indexElement = root.querySelector("index");
      const indexGear = Number(indexElement && indexElement.getAttribute("gear")) || 96;
      const degreesPerIndex = 360 / indexGear;
      const tiers = Array.from(xml.querySelectorAll("tier"));

      for (const tier of tiers) {
        const name = tier.getAttribute("name") || "?";
        const storedAngle = Number(tier.getAttribute("angle"));
        if (!Number.isFinite(storedAngle)) continue;

        // GCS stores pavilion angles as supplements (e.g. 134.47 -> 45.53).
        const angle = storedAngle > 90 ? 180 - storedAngle : storedAngle;
        const facetData = Array.from(tier.querySelectorAll(":scope > facet"));
        const indexValues = [];

        for (const facet of facetData) {
          const indexAngle = Number(facet.getAttribute("index_angle"));
          if (Number.isFinite(indexAngle)) {
            indexValues.push(Math.round(indexAngle / degreesPerIndex));
          }

          const vertices = Array.from(facet.querySelectorAll("vertex"))
            .map(v => ({
              x: Number(v.getAttribute("x")),
              y: Number(v.getAttribute("y")),
              z: Number(v.getAttribute("z"))
            }))
            .filter(v => Number.isFinite(v.x) && Number.isFinite(v.y) && Number.isFinite(v.z));
          // Keep the tier name with each polygon so the selected tier can
          // be highlighted consistently in every projection.
          const normal = {
            x: Number(facet.getAttribute("nx")) || 0,
            y: Number(facet.getAttribute("ny")) || 0,
            z: Number(facet.getAttribute("nz")) || 0
          };
          if (vertices.length >= 3) geometry.push({ name, group: facetGroup(name), vertices, normal });
        }

        facets.push({
          name,
          angle,
          indices: normalizeIndices(indexValues, indexGear),
          group: facetGroup(name),
          instructions: tier.getAttribute("instructions") || ""
        });
      }

      return {
        title: "Gem Cut Studio design",
        facets,
        geometry,
        format: "GCS",
        indexGear
      };
    }

    // Interactive 3D renderer. It projects the original GCS XYZ vertices into
    // a perspective canvas, depth-sorts the facets, and shades each face.
    let gemRotationX = -0.62;
    let gemRotationY = 0.55;
    let gemZoom = 1;
    let gemDragging = false;
    let gemLastPointer = null;
    let gemGeometry = null;

    function resizeGemCanvas() {
      const rect = gemCanvas.getBoundingClientRect();
      if (!rect.width || !rect.height) return;
      const dpr = Math.min(window.devicePixelRatio || 1, 2);
      const width = Math.round(rect.width * dpr);
      const height = Math.round(rect.height * dpr);
      if (gemCanvas.width !== width || gemCanvas.height !== height) {
        gemCanvas.width = width;
        gemCanvas.height = height;
      }
      drawGem3d();
    }

    function rotatePoint(point) {
      const cy = Math.cos(gemRotationY), sy = Math.sin(gemRotationY);
      const cx = Math.cos(gemRotationX), sx = Math.sin(gemRotationX);
      const x1 = point.x * cy + point.z * sy;
      const z1 = -point.x * sy + point.z * cy;
      const y2 = point.y * cx - z1 * sx;
      const z2 = point.y * sx + z1 * cx;
      return { x: x1, y: y2, z: z2 };
    }

    function drawGem3d() {
      if (!gemContext || !gemCanvas.width || !gemCanvas.height) return;
      const ctx = gemContext;
      const width = gemCanvas.width, height = gemCanvas.height;
      ctx.clearRect(0, 0, width, height);
      if (!gemGeometry || !gemGeometry.length) {
        ctx.fillStyle = "#9aa8b5";
        ctx.font = `${14 * (window.devicePixelRatio || 1)}px sans-serif`;
        ctx.textAlign = "center";
        ctx.fillText("Load a GCS design to view the 3D gemstone", width / 2, height / 2);
        return;
      }

      const allPoints = gemGeometry.flatMap(poly => poly.vertices);
      const center = {
        x: (Math.min(...allPoints.map(p => p.x)) + Math.max(...allPoints.map(p => p.x))) / 2,
        y: (Math.min(...allPoints.map(p => p.y)) + Math.max(...allPoints.map(p => p.y))) / 2,
        z: (Math.min(...allPoints.map(p => p.z)) + Math.max(...allPoints.map(p => p.z))) / 2
      };
      // Fit the complete model inside a bounding sphere. Keep the camera and
      // vertices in the same world units; the previous version mixed pixel
      // scale into the camera depth calculation and magnified the model twice.
      const extent = Math.max(...allPoints.map(p => Math.hypot(p.x-center.x, p.y-center.y, p.z-center.z))) || 1;
      const cameraDistance = extent * 4.5;
      const focal = Math.min(width, height) * 1.35 * gemZoom;

      const projected = gemGeometry.map(poly => {
        const points = poly.vertices.map(vertex => {
          const p = rotatePoint({ x: vertex.x-center.x, y: vertex.y-center.y, z: vertex.z-center.z });
          const depth = Math.max(extent * 0.5, cameraDistance - p.z);
          return { x: width/2 + p.x*focal/depth, y: height/2 - p.y*focal/depth, z: p.z };
        });
        const avgDepth = points.reduce((sum, p) => sum + p.z, 0) / points.length;
        const isSelected = selectedFacetName !== null && poly.name === selectedFacetName;
        // Unselected tiers share a neutral gray treatment; only the active
        // tier uses amber so it remains obvious while the model is rotated.
        const groupColor = [145, 151, 158];
        const n = poly.normal || {x:0,y:0,z:1};
        const rn = rotatePoint(n);
        const light = Math.max(0.22, Math.abs(rn.x * -0.35 + rn.y * 0.55 + rn.z * 0.76));
        return { points, avgDepth, isSelected, groupColor, light };
      }).sort((a, b) => a.avgDepth - b.avgDepth);

      for (const face of projected) {
        if (face.points.length < 3) continue;
        const [r,g,b] = face.groupColor;
        const shade = face.isSelected ? 1 : Math.max(0.42, face.light);
        ctx.beginPath();
        ctx.moveTo(face.points[0].x, face.points[0].y);
        for (let i=1; i<face.points.length; i++) ctx.lineTo(face.points[i].x, face.points[i].y);
        ctx.closePath();
        ctx.fillStyle = face.isSelected ? "rgba(245,158,11,0.96)" :
          `rgba(${Math.round(r*shade)},${Math.round(g*shade)},${Math.round(b*shade)},0.96)`;
        ctx.fill();
        ctx.strokeStyle = face.isSelected ? "#fff0b5" : "rgba(210,228,240,0.72)";
        ctx.lineWidth = (face.isSelected ? 2 : 0.8) * (window.devicePixelRatio || 1);
        ctx.stroke();
      }
    }

    function drawGcsDiagram(polygons, selectedName = selectedFacetName) {
      gemGeometry = polygons || null;
      selectedFacetName = selectedName;
      resizeGemCanvas();
      diagramCaption.textContent = polygons && polygons.length
        ? "GCS 3D geometry · selected tier highlighted · drag to rotate"
        : "No 3D geometry available in this file";
    }

    gemCanvas.addEventListener("pointerdown", event => {
      gemDragging = true;
      gemLastPointer = { x: event.clientX, y: event.clientY };
      gemCanvas.setPointerCapture(event.pointerId);
    });
    gemCanvas.addEventListener("pointermove", event => {
      if (!gemDragging || !gemLastPointer) return;
      const dx = event.clientX - gemLastPointer.x;
      const dy = event.clientY - gemLastPointer.y;
      gemRotationY += dx * 0.009;
      gemRotationX = Math.max(-Math.PI, Math.min(Math.PI, gemRotationX + dy * 0.009));
      gemLastPointer = { x: event.clientX, y: event.clientY };
      drawGem3d();
    });
    function stopGemDrag() { gemDragging = false; gemLastPointer = null; }
    gemCanvas.addEventListener("pointerup", stopGemDrag);
    gemCanvas.addEventListener("pointercancel", stopGemDrag);
    gemCanvas.addEventListener("wheel", event => {
      event.preventDefault();
      gemZoom = Math.max(0.55, Math.min(3.2, gemZoom * (event.deltaY < 0 ? 1.1 : 0.91)));
      drawGem3d();
    }, { passive: false });
    resetGemViewButton.addEventListener("click", () => {
      gemRotationX = -0.62;
      gemRotationY = 0.55;
      gemZoom = 1;
      drawGem3d();
    });
    window.addEventListener("resize", resizeGemCanvas);
    if (window.ResizeObserver) new ResizeObserver(resizeGemCanvas).observe(gemCanvas);
    resizeGemCanvas();

    function renderFacetList() {
      facetGroupsElement.replaceChildren();
      updateTierNavigation();

      for (const groupName of ["Pavilion", "Girdle", "Crown"]) {
        const groupFacets = designFacets.filter(f => f.group === groupName);
        if (!groupFacets.length) continue;

        const heading = document.createElement("div");
        heading.className = "facet-group-title";
        heading.textContent = groupName;
        facetGroupsElement.appendChild(heading);

        for (const facet of groupFacets) {
          const button = document.createElement("button");
          button.type = "button";
          button.className = "facet-row" +
            (selectedFacetName === facet.name ? " selected" : "");
          button.disabled = facet.name.toUpperCase() === "T";

          const label = document.createElement("span");
          label.textContent = facet.name;
          const values = document.createElement("span");
          values.className = "facet-values";
          const angle = document.createElement("span");
          angle.className = "facet-angle";
          angle.textContent = facet.angle.toFixed(2) + "°";
          values.appendChild(angle);

          // Show index positions beneath the angle for the selected tier.
          if (selectedFacetName === facet.name && facet.indices && facet.indices.length) {
            const indices = document.createElement("span");
            indices.className = "facet-indices";
            indices.textContent = "Index: " + facet.indices.map(v => String(v).padStart(2, "0")).join(" · ");
            values.appendChild(indices);
          }

          button.append(label, values);
          if (facet.name.toUpperCase() !== "T") {
            button.addEventListener("click", () => selectDesignFacet(facet));
          } else {
            button.title = "Table tier is shown for reference, not selectable as a cutting target.";
          }
          facetGroupsElement.appendChild(button);
        }
      }
    }

    async function selectDesignFacet(facet) {
      try {
        const response = await fetch(
          "/api/target?angle=" + encodeURIComponent(facet.angle.toFixed(2)),
          { method: "POST" }
        );
        if (!response.ok) {
          const result = await response.json().catch(() => ({}));
          throw new Error(result.message || "Unable to set target angle.");
        }

        selectedFacetName = facet.name;
        // Redraw all three projections so only this tier is emphasized.
        if (currentDesignGeometry) drawGcsDiagram(currentDesignGeometry, selectedFacetName);
        targetIndicesElement.textContent = facet.indices && facet.indices.length
          ? facet.name + ": " + facet.indices.map(v => String(v).padStart(2, "0")).join(" - ")
          : facet.name + ": index positions unavailable";
        renderFacetList();
        designStatus.textContent =
          "Target set to " + facet.name + " · " + facet.angle.toFixed(2) + "°";
        await updateStatus();
      } catch (error) {
        console.error(error);
        designStatus.textContent = error.message +
          " Check that the matching WebServer.ino change is installed.";
      }
    }

    designFileInput.addEventListener("change", async event => {
      const file = event.target.files && event.target.files[0];
      if (!file) return;

      try {
        const lowerName = file.name.toLowerCase();
        const isAsc = lowerName.endsWith(".asc");
        // iOS Safari may append .xml to downloaded Gem Cut Studio files.
        const isGcs = lowerName.endsWith(".gcs") || lowerName.endsWith(".gcs.xml");

        if (!isAsc && !isGcs) {
          throw new Error("Please choose an .ASC, .GCS, or .GCS.XML file.");
        }

        const text = await file.text();
        const parsed = isAsc ? parseAsc(text) : parseGcs(text);
        if (!parsed.facets.length) {
          throw new Error("No facet tiers were found in this file.");
        }

        designFacets = parsed.facets;
        currentDesignGeometry = parsed.geometry || null;
        selectedFacetName = null;
        const designTitle = parsed.title || file.name;
        designMeta.textContent =
          designTitle + " · " + parsed.format + " · " +
          parsed.facets.length + " tiers · " + parsed.indexGear + "-tooth index gear";
        renderFacetList();

        if (parsed.geometry) drawGcsDiagram(parsed.geometry, selectedFacetName);
        else {
          drawGcsDiagram(null, null);
          diagramCaption.textContent = "ASC contains facet instructions, not 3D geometry";
        }

        designStatus.textContent =
          "Loaded " + parsed.facets.length + " facet tiers. Select a tier to set the target.";
      } catch (error) {
        console.error(error);
        designStatus.textContent = error.message;
      }
    });


    updateStatus();
    setInterval(updateStatus, 150);
  </script>
</body>
</html>
)HTML";
