#ifndef INDEX_H
#define INDEX_H

const char* index_html = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
  <title>Miku Desktop Control & Telemetry</title>
  <style>
    :root {
      --bg-color: #0f172a;
      --card-bg: #1e293b;
      --text-color: #f8fafc;
      --accent: #38bdf8;
      --accent-hover: #0ea5e9;
      --danger: #f43f5e;
      --success: #10b981;
    }
    body {
      margin: 0;
      padding: 0;
      background-color: var(--bg-color);
      color: var(--text-color);
      font-family: 'Inter', -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
      touch-action: pan-y;
      display: flex;
      flex-direction: column;
      align-items: center;
      min-height: 100vh;
      overflow-x: hidden;
    }
    header {
      width: 100%;
      padding: 15px;
      text-align: center;
      background: rgba(30, 41, 59, 0.9);
      backdrop-filter: blur(10px);
      box-shadow: 0 4px 6px -1px rgba(0, 0, 0, 0.1);
      position: sticky;
      top: 0;
      z-index: 100;
    }
    h1 { margin: 0; font-size: 20px; font-weight: 700; color: var(--accent); letter-spacing: 1px; }
    .status-bar { font-size: 13px; color: #94a3b8; margin-top: 5px; }
    #connStatus { color: var(--danger); font-weight: bold; }
    
    .view-container {
      width: 100%;
      max-width: 800px;
      padding: 20px;
      box-sizing: border-box;
      display: grid;
      grid-template-columns: 1fr;
      gap: 20px;
    }
    
    @media (min-width: 768px) {
      .view-container {
        grid-template-columns: 1fr 1fr;
      }
    }

    /* CONTROL PANEL */
    .panel {
      background: var(--card-bg);
      border-radius: 12px;
      padding: 20px;
      box-shadow: 0 10px 15px -3px rgba(0, 0, 0, 0.1);
      display: flex;
      flex-direction: column;
      align-items: center;
    }
    .panel h2 {
      margin-top: 0; margin-bottom: 20px;
      font-size: 16px; color: #94a3b8; text-transform: uppercase;
    }

    .mode-controls { display: flex; gap: 15px; width: 100%; margin-bottom: 20px; }
    .btn {
      flex: 1; background: var(--bg-color); border: 2px solid transparent; color: var(--text-color);
      padding: 12px; border-radius: 8px; font-size: 14px; font-weight: 600; cursor: pointer; transition: all 0.2s;
    }
    .btn.active { border-color: var(--accent); background: rgba(56, 189, 248, 0.1); box-shadow: 0 0 15px rgba(56, 189, 248, 0.2); }
    .btn:active { transform: scale(0.98); }

    .joystick-zone {
      width: 220px; height: 220px; background: var(--bg-color);
      border-radius: 50%; position: relative;
      box-shadow: inset 0 4px 10px rgba(0,0,0,0.3);
      display: flex; justify-content: center; align-items: center;
      transition: opacity 0.3s ease;
    }
    .joystick-zone.disabled { opacity: 0.3; pointer-events: none; }
    .joystick-stick {
      width: 70px; height: 70px; background: linear-gradient(135deg, var(--accent), var(--accent-hover));
      border-radius: 50%; position: absolute; box-shadow: 0 8px 16px rgba(0,0,0,0.4);
      cursor: grab; touch-action: none;
    }
    .joystick-stick:active { cursor: grabbing; }

    /* TELEMETRY PANEL */
    .telemetry-row {
      display: flex; justify-content: space-between; width: 100%;
      border-bottom: 1px solid rgba(255,255,255,0.05); padding: 8px 0;
      font-family: monospace; font-size: 13px; color: #cbd5e1;
    }
    .val-cell { font-weight: bold; color: var(--accent); text-align: right; width: 50px; display: inline-block; }

    /* LIDAR GRID */
    .lidar-grid {
      display: grid;
      grid-template-columns: repeat(8, 1fr);
      gap: 2px;
      margin-top: 15px;
      width: 100%;
      max-width: 250px;
      aspect-ratio: 1;
      background: #000;
      padding: 2px;
      border-radius: 6px;
    }
    .lidar-cell {
      background: #1e293b; border-radius: 2px;
      display: flex; align-items: center; justify-content: center;
      font-size: 8px; font-family: monospace; color: rgba(255,255,255,0.7);
      transition: background 0.1s;
    }
  </style>
</head>
<body>

  <header>
    <h1>MIKU DESKTOP</h1>
    <div class="status-bar">Status: <span id="connStatus">Disconnected</span></div>
  </header>

  <div class="view-container">
    
    <!-- Left: Control -->
    <div class="panel">
      <h2>Control</h2>
      <div class="mode-controls">
        <button class="btn active" id="btnRC" onclick="setMode('RC')">RC Mode</button>
        <button class="btn" id="btnFuzzy" onclick="setMode('FUZZY')">Fuzzy Mode</button>
      </div>
      <div class="joystick-zone" id="jZone">
        <div class="joystick-stick" id="jStick"></div>
      </div>
    </div>

    <!-- Right: Telemetry -->
    <div class="panel">
      <h2>Sensors</h2>
      <div class="telemetry-row"><span>Heading</span><span class="val-cell" id="t-head">0.0</span></div>
      <div class="telemetry-row"><span>Accel (X,Y,Z)</span><span><span class="val-cell" id="t-ax">0</span> <span class="val-cell" id="t-ay">0</span> <span class="val-cell" id="t-az">0</span></span></div>
      <div class="telemetry-row"><span>Gyro (X,Y,Z)</span><span><span class="val-cell" id="t-gx">0</span> <span class="val-cell" id="t-gy">0</span> <span class="val-cell" id="t-gz">0</span></span></div>
      
      <h2 style="margin-top: 25px;">LiDAR Depth Map</h2>
      <div class="lidar-grid" id="lidarGrid">
        <!-- populated by JS -->
      </div>
    </div>

  </div>

  <script>
    // Build LiDAR grid
    const lidarGrid = document.getElementById('lidarGrid');
    const cells = [];
    for(let i=0; i<64; i++){
      let cell = document.createElement('div');
      cell.className = 'lidar-cell';
      cell.innerText = "-";
      lidarGrid.appendChild(cell);
      cells.push(cell);
    }

    let socket;
    let currentMode = "RC";
    const jZone = document.getElementById('jZone');
    const jStick = document.getElementById('jStick');
    const connStatus = document.getElementById('connStatus');
    
    function initWebSocket() {
      const gateway = `ws://${window.location.hostname}:81/`;
      socket = new WebSocket(gateway);
      socket.onopen = () => { connStatus.innerText = "Connected"; connStatus.style.color = "var(--success)"; };
      socket.onclose = () => { connStatus.innerText = "Disconnected"; connStatus.style.color = "var(--danger)"; setTimeout(initWebSocket, 2000); };
      socket.onmessage = (event) => processData(event.data);
    }

    function processData(msg) {
      if(msg.startsWith("IMU,")) {
        // IMU,ax,ay,az,gx,gy,gz,mx,my,mz,heading
        let parts = msg.split(",");
        if(parts.length >= 11) {
          document.getElementById('t-ax').innerText = parseFloat(parts[1]).toFixed(1);
          document.getElementById('t-ay').innerText = parseFloat(parts[2]).toFixed(1);
          document.getElementById('t-az').innerText = parseFloat(parts[3]).toFixed(1);
          document.getElementById('t-gx').innerText = parseFloat(parts[4]).toFixed(1);
          document.getElementById('t-gy').innerText = parseFloat(parts[5]).toFixed(1);
          document.getElementById('t-gz').innerText = parseFloat(parts[6]).toFixed(1);
          document.getElementById('t-head').innerText = parseFloat(parts[10]).toFixed(1);
        }
      } else if(msg.startsWith("LIDAR,")) {
        // LIDAR,val1,val2...val64
        let parts = msg.split(",");
        for(let i=0; i<64 && (i+1)<parts.length; i++) {
          let dist = parseInt(parts[i+1]);
          cells[i].innerText = dist;
          // Color map: 0 = Red (close), 1000 = Blue/Black (far)
          // hue from 0 (red) to 200 (light blue)
          let hue = Math.min(200, (dist / 1500) * 200);
          let lum = dist < 200 ? 50 : (dist > 1500 ? 15 : 30);
          cells[i].style.background = `hsl(${hue}, 80%, ${lum}%)`;
        }
      }
    }

    function setMode(mode) {
      currentMode = mode;
      document.getElementById('btnRC').classList.toggle('active', mode === 'RC');
      document.getElementById('btnFuzzy').classList.toggle('active', mode === 'FUZZY');
      
      if(mode === 'RC') jZone.classList.remove('disabled');
      else {
        jZone.classList.add('disabled');
        jStick.style.transform = `translate(0px, 0px)`;
        sendJoy(0, 0); 
      }
      if(socket && socket.readyState === WebSocket.OPEN) socket.send(mode);
    }

    // Joystick Logic
    let isDragging = false;
    const maxRadius = 75;

    function processJoyStart(e) { if(currentMode === 'RC') isDragging = true; }
    function processJoyMove(e) {
      if (!isDragging || currentMode !== 'RC') return;
      let clientX = e.clientX, clientY = e.clientY;
      if (e.touches && e.touches.length > 0) { clientX = e.touches[0].clientX; clientY = e.touches[0].clientY; }
      const rect = jZone.getBoundingClientRect();
      let dx = clientX - (rect.left + rect.width/2), dy = clientY - (rect.top + rect.height/2);
      const distance = Math.sqrt(dx*dx + dy*dy);
      if (distance > maxRadius) { dx = (dx/distance)*maxRadius; dy = (dy/distance)*maxRadius; }
      
      jStick.style.transform = `translate(${dx}px, ${dy}px)`;
      sendJoy(Math.round((dx/maxRadius)*100), Math.round(-(dy/maxRadius)*100));
    }
    function processJoyEnd() {
      if (!isDragging) return;
      isDragging = false;
      jStick.style.transform = `translate(0px, 0px)`;
      jStick.style.transition = 'transform 0.2s'; setTimeout(() => jStick.style.transition = 'none', 200);
      sendJoy(0, 0);
    }
    
    let lastSend = 0;
    function sendJoy(x, y) {
      const now = Date.now();
      if (now - lastSend < 50 && (x!==0 || y!==0)) return; 
      lastSend = now;
      if(socket && socket.readyState === WebSocket.OPEN) socket.send(`J_${x}_${y}`);
    }

    jStick.addEventListener('mousedown', processJoyStart);
    jStick.addEventListener('touchstart', processJoyStart, {passive: false});
    document.addEventListener('mousemove', processJoyMove);
    document.addEventListener('touchmove', (e)=>{ if(isDragging) e.preventDefault(); processJoyMove(e); }, {passive: false});
    document.addEventListener('mouseup', processJoyEnd);
    document.addEventListener('touchend', processJoyEnd);
    window.onload = initWebSocket;
  </script>
</body>
</html>
)=====";

#endif
