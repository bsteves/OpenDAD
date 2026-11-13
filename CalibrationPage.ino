/******************************************************************************
  OpenDAD calibration web interface
******************************************************************************/

const char CALIBRATION_PAGE[] PROGMEM = R"HTML(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>OpenDAD Calibration</title>
  <style>
    * { box-sizing: border-box; }
    body {
      margin: 0;
      padding: 18px;
      font-family: Arial, sans-serif;
      background: #151515;
      color: #ffffff;
    }
    main { max-width: 560px; margin: 0 auto; }
    a { color: #a9c7ff; }
    h1 { margin-bottom: 8px; }
    p { line-height: 1.4; }
    .note { color: #bdbdbd; font-size: 0.9rem; }
    .row {
      display: grid;
      grid-template-columns: 90px 1fr 1fr;
      align-items: center;
      gap: 8px;
      padding: 5px 0;
      font-variant-numeric: tabular-nums;
    }
    .head { color: #bdbdbd; font-size: 0.85rem; }
    .plot { width: 100%; height: auto; display: block; }
    .plot text { fill: #bdbdbd; font: 12px Arial, sans-serif; }
    .plot .axis { stroke: #777777; stroke-width: 1; }
    .plot .zero { stroke: #bdbdbd; stroke-width: 1; stroke-dasharray: 4 4; }
    .plot polyline { fill: none; stroke: #a9c7ff; stroke-width: 2; }
    .plot circle { fill: #a9c7ff; }
    button {
      border: 0;
      border-radius: 8px;
      padding: 10px;
      font-weight: bold;
      cursor: pointer;
      touch-action: manipulation;
    }
    button:disabled { opacity: 0.5; cursor: default; }
    .reset { margin-top: 16px; background: #8f2929; color: white; }
    #warning { color: #ffba83; }
    #connection, #message { min-height: 1.4em; }
  </style>
</head>
<body>
  <main>
    <a href="/">&#8592; Back to display</a>
    <h1>Calibration</h1>
    <p>Set the mast to each reference angle, then press its button.</p>
    <p>Current angle: <strong id="angle">--</strong> &nbsp;
       Raw sensor angle: <strong id="rawAngle">--</strong></p>
    <div class="row head">
      <span>Reference</span><span>Saved raw</span><span>Deviation</span>
    </div>
    <div id="points"></div>
    <p class="note">
      Once all points are saved, deviation compares each raw reading with
      the value estimated from its two neighbors. Endpoints use the next two
      points and are less reliable. Values are signed raw sensor degrees,
      not measured calibration errors; a real nonlinear response can also
      produce deviations.
    </p>
    <h2>Deviation by reference angle</h2>
    <p id="plotPending" class="note">Save all calibration points to show the plot.</p>
    <div id="plotContainer" hidden>
    <svg class="plot" viewBox="0 0 400 260" role="img"
         aria-label="Signed raw sensor deviation by reference angle. Values are listed above.">
      <line class="axis" x1="70" y1="25" x2="70" y2="205"></line>
      <line class="axis" x1="70" y1="205" x2="390" y2="205"></line>
      <line class="zero" x1="70" y1="115" x2="390" y2="115"></line>
      <text id="plotMax" x="64" y="29" text-anchor="end"></text>
      <text x="64" y="119" text-anchor="end">0</text>
      <text id="plotMin" x="64" y="209" text-anchor="end"></text>
      <text x="230" y="253" text-anchor="middle">Reference angle (°)</text>
      <text transform="translate(15 115) rotate(-90)" text-anchor="middle">Raw deviation (°)</text>
      <g id="plotTicks"></g>
      <polyline id="plotLine"></polyline>
      <g id="plotPoints"></g>
    </svg>
    </div>
    <p id="warning" role="status"></p>
    <button class="reset" data-command="/api/reset">Reset calibration</button>
    <p id="message" role="status"></p>
    <p id="connection">Connecting...</p>
  </main>
  <script>
    const points = document.getElementById("points");
    const message = document.getElementById("message");
    const connection = document.getElementById("connection");
    const warning = document.getElementById("warning");
    const plotContainer = document.getElementById("plotContainer");
    const plotPending = document.getElementById("plotPending");
    const plotTicks = document.getElementById("plotTicks");
    const plotPoints = document.getElementById("plotPoints");
    const svgNamespace = "http://www.w3.org/2000/svg";
    const rows = [];

    for (let i = 0; i < 10; i++) {
      const row = document.createElement("div");
      row.className = "row";
      const button = document.createElement("button");
      button.textContent = "Set " + (i * 10) + "°";
      button.dataset.command = "/api/calibrate/" + (i * 10);
      const raw = document.createElement("span");
      const deviation = document.createElement("span");
      row.append(button, raw, deviation);
      points.appendChild(row);
      rows.push({ raw, deviation });
      const tick = document.createElementNS(svgNamespace, "text");
      tick.setAttribute("x", 70 + i * 320 / 9);
      tick.setAttribute("y", 224);
      tick.setAttribute("text-anchor", "middle");
      tick.textContent = i * 10;
      plotTicks.appendChild(tick);
    }

    const signedDelta = (from, to) => {
      let delta = (to - from) % 360;
      if (delta > 180) delta -= 360;
      if (delta <= -180) delta += 360;
      return delta;
    };

    const pointDeviation = (raw, i) => {
      let expected;
      if (i === 0) {
        expected = 2 * raw[1] - raw[2];
      } else if (i === raw.length - 1) {
        expected = 2 * raw[i - 1] - raw[i - 2];
      } else {
        expected = (raw[i - 1] + raw[i + 1]) / 2;
      }
      return raw[i] - expected;
    };

    const updatePlot = deviations => {
      plotContainer.hidden = !deviations;
      plotPending.hidden = !!deviations;
      if (!deviations) return;

      const limit = Math.max(0.001, Math.ceil(Math.max(...deviations.map(Math.abs)) * 1100) / 1000);
      document.getElementById("plotMax").textContent = "+" + limit.toFixed(3);
      document.getElementById("plotMin").textContent = "-" + limit.toFixed(3);
      const coordinates = deviations.map((value, i) => ({
        x: 70 + i * 320 / 9,
        y: 115 - value / limit * 90
      }));
      document.getElementById("plotLine").setAttribute(
        "points", coordinates.map(point => point.x + "," + point.y).join(" ")
      );
      plotPoints.replaceChildren();
      coordinates.forEach((point, i) => {
        const circle = document.createElementNS(svgNamespace, "circle");
        circle.setAttribute("cx", point.x);
        circle.setAttribute("cy", point.y);
        circle.setAttribute("r", "4");
        const title = document.createElementNS(svgNamespace, "title");
        title.textContent = (i * 10) + "°: " +
          (deviations[i] >= 0 ? "+" : "") + deviations[i].toFixed(3) + "°";
        circle.appendChild(title);
        plotPoints.appendChild(circle);
      });
    };

    const updateStatus = async () => {
      try {
        const response = await fetch("/api/status", { cache: "no-store" });
        if (!response.ok) throw new Error("Status request failed");
        const status = await response.json();
        const raw = status.calibrationRawAngles;
        if (!Array.isArray(raw) || raw.length !== 10 ||
            raw.some(value => value !== null && !Number.isFinite(value))) {
          throw new Error("Invalid calibration data");
        };

        document.getElementById("angle").textContent =
          status.sensorReady ? Number(status.angle).toFixed(2) + "°" : "--";
        document.getElementById("rawAngle").textContent =
          status.sensorReady ? Number(status.rawAngle).toFixed(2) + "°" : "--";
        const complete = status.calibrationComplete && raw.every(value => value !== null);
        const unwrapped = complete ? [raw[0]] : [];
        if (complete) {
          for (let i = 1; i < raw.length; i++) {
            unwrapped.push(unwrapped[i - 1] + signedDelta(raw[i - 1], raw[i]));
          }
        }
        const deviations = complete ? unwrapped.map((_, i) => pointDeviation(unwrapped, i)) : null;
        rows.forEach((row, i) => {
          row.raw.textContent = raw[i] === null ? "Not set" : raw[i].toFixed(3) + "°";
          if (complete) {
            const deviation = deviations[i];
            row.deviation.textContent =
              (deviation >= 0 ? "+" : "") + deviation.toFixed(3) + "°";
          } else {
            row.deviation.textContent = "—";
          }
        });
        updatePlot(deviations);

        const saved = raw.filter(value => value !== null);
        const spans = saved.slice(1).map((value, i) => signedDelta(saved[i], value));
        const direction = spans.length && spans[0] >= 0 ? 1 : -1;
        warning.textContent = spans.some(span => span * direction <= 0.01)
          ? "Warning: saved readings do not progress consistently; calibration cannot be applied."
          : "";
        connection.textContent = status.sensorReady
          ? "OpenDAD connected" : "Waiting for sensor data";
      } catch (error) {
        console.error("Status update failed:", error);
        connection.textContent = "Connection or calibration data error";
      }
    }

    document.querySelectorAll("[data-command]").forEach(button => {
      button.addEventListener("click", async () => {
        const buttons = document.querySelectorAll("[data-command]");
        buttons.forEach(item => { item.disabled = true; });
        message.textContent = "Saving...";
        try {
          const response = await fetch(button.dataset.command, { method: "POST" });
          const result = await response.json();
          if (!response.ok || !result.success) {
            throw new Error(result.message || "Command failed");
          }
          message.textContent = result.message;
          await updateStatus();
        } catch (error) {
          console.error("Command failed:", error);
          message.textContent = "Command failed: " + error.message;
        } finally {
          buttons.forEach(item => { item.disabled = false; });
        }
      });
    });

    updateStatus();
    setInterval(updateStatus, 1000);
  </script>
</body>
</html>
)HTML";
