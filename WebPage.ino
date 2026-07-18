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
      margin: 24px 0 8px;
      font-size: clamp(72px, 20vw, 96px);
      line-height: 1;
      font-weight: bold;
      letter-spacing: -0.04em;
    }

    .raw {
      margin-bottom: 18px;
      color: #999999;
      font-size: 0.9rem;
    }

    .target-panel {
      margin-top: 18px;
      padding: 16px;
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
      margin-top: 3px;
      font-size: 1.8rem;
      font-weight: bold;
    }

    .difference-label {
      margin-top: 15px;
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
      margin: 6px auto 12px;
      padding: 12px 8px;
      border-radius: 12px;
      font-size: clamp(52px, 16vw, 76px);
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
      margin-top: 4px;
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

    @media (max-width: 390px) {
      body {
        padding: 12px;
      }

      .target-panel {
        padding: 13px;
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
    <h1>OpenDAD</h1>
    <p class="subtitle">Digital Angle Display</p>

    <div id="angle" class="angle">--.--°</div>

    <div class="raw">
      Raw sensor angle:
      <span id="rawAngle">--.--</span>°
    </div>

    <div class="target-panel">
      <div class="target-label">Target angle</div>
      <div id="targetAngle" class="target-angle">Not set</div>

      <div class="difference-label">Difference</div>
      <strong
        id="angleDifference"
        class="difference unset"
      >—</strong>

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

    <div class="calibration-section">
      <div class="section-label">Calibration</div>

      <div class="calibration-buttons">
        <button class="cal-button" data-command="/api/calibrate/0">
          Set 0°
        </button>

        <button class="cal-button" data-command="/api/calibrate/45">
          Set 45°
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

    const connectionElement =
      document.getElementById("connection");


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





    updateStatus();
    setInterval(updateStatus, 150);
  </script>
</body>
</html>
)HTML";
