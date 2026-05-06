const char PAGE_DATA[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Adaptive Home</title>
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <style>
        * { box-sizing: border-box; }
        body {
            background: #0b1120;
            color: #f8fafc;
            font-family: Arial, sans-serif;
            text-align: center;
            margin: 0;
        }
        .container {
            width: min(1180px, 100%);
            margin: 0 auto;
            padding: clamp(16px, 3vw, 32px);
        }
        h1 {
            margin: 8px 0 20px;
            font-size: clamp(30px, 5vw, 56px);
            line-height: 1.05;
        }
        .status-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(190px, 1fr));
            gap: 14px;
            margin: 20px auto 24px;
        }
        .card {
            background: #182234;
            border-radius: 14px;
            padding: 18px;
            min-width: 0;
            border: 1px solid #334155;
            box-shadow: 0 14px 30px rgba(0,0,0,0.18);
        }
        .label {
            color: #cbd5e1;
            font-size: 13px;
            font-weight: bold;
            letter-spacing: 0;
            text-transform: uppercase;
        }
        .val {
            font-size: clamp(34px, 6vw, 48px);
            color: #00d2ff;
            font-weight: bold;
            line-height: 1.1;
            margin-top: 8px;
            transition: color 0.25s;
        }
        .unit { color: #e2e8f0; font-size: 18px; font-weight: bold; }
        .subtext {
            color: #94a3b8;
            font-size: 13px;
            margin-top: 8px;
            overflow-wrap: anywhere;
        }
        .slider-wrap { margin-top: 14px; }
        input[type="range"] {
            width: 100%;
            accent-color: #00d2ff;
        }
        .control-row {
            display: flex;
            gap: 8px;
            justify-content: center;
            flex-wrap: wrap;
            margin-top: 12px;
        }
        .control-row button {
            background: #0f172a;
            color: #cbd5e1;
            border: 1px solid #475569;
            border-radius: 10px;
            padding: 8px 12px;
            cursor: pointer;
            font-size: 13px;
            min-width: 74px;
        }
        .control-row button.active {
            background: #00d2ff;
            color: #07111f;
            border-color: #00d2ff;
            font-weight: bold;
        }
        .control-row button.off.active {
            background: #64748b;
            color: white;
            border-color: #64748b;
        }
        .chart-box {
            max-width: 1000px;
            height: clamp(270px, 44vw, 420px);
            margin: 20px auto;
            background: #090f1f;
            padding: 16px;
            border-radius: 16px;
            border: 1px solid #334155;
        }
        .chart-box canvas {
            width: 100% !important;
            height: 100% !important;
        }
        .log-container {
            max-width: 1000px;
            margin: 24px auto 0;
        }
        .log-container h3 {
            font-size: clamp(20px, 4vw, 30px);
            margin: 0 0 14px;
        }
        .history-card {
            padding: 16px;
        }
        .table-scroll {
            overflow-x: auto;
        }
        table {
            width: 100%;
            min-width: 520px;
            border-collapse: collapse;
        }
        th {
            color: #94a3b8;
            font-size: 12px;
            text-transform: uppercase;
            padding: 12px 10px;
            border-bottom: 1px solid #334155;
        }
        td {
            padding: 12px 10px;
            border-bottom: 1px solid #1e293b;
        }
        .empty-row { color: #94a3b8; }

        @media (max-width: 520px) {
            .status-grid { grid-template-columns: 1fr; }
            .card { padding: 16px; }
            .control-row button { flex: 1; }
            .chart-box { padding: 10px; }
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Adaptive Home Control System</h1>

        <div class="status-grid">
            <div class="card">
                <div class="label">Temperature</div>
                <div class="val" id="t">--</div>
                <div class="unit">&deg;C</div>
            </div>

            <div class="card">
                <div class="label">Humidity</div>
                <div class="val" id="h">--</div>
                <div class="unit">%</div>
            </div>

            <div class="card">
                <div class="label">Motion</div>
                <div class="val" id="m">--</div>
                <div class="subtext" id="lastMotion">Last: Never</div>
            </div>

            <div class="card">
                <div class="label">Motor</div>
                <div class="val" id="p">--</div>
                <div class="unit">%</div>
                <div class="subtext" id="motorMode">Mode: Dynamic</div>

                <div class="slider-wrap">
                    <input id="motorSlider" type="range" min="0" max="100" value="0"
                           oninput="previewManualPower(this.value)"
                           onchange="setManualPower(this.value)">
                    <div class="subtext">Manual: <span id="sliderValue">0</span>%</div>
                </div>

                <div class="control-row">
                    <button id="btnDynamic" onclick="setDynamicMode()">Dynamic</button>
                </div>
            </div>

            <div class="card">
                <div class="label">LED</div>
                <div class="val" id="led">--</div>
                <div class="subtext" id="ledText">Status: --</div>
                <div class="control-row">
                    <button id="btnLedOff" class="off" onclick="setLed(0)">Off</button>
                    <button id="btnLedOn" onclick="setLed(1)">On</button>
                </div>
            </div>
        </div>

        <div class="chart-box">
            <canvas id="myChart"></canvas>
        </div>

        <div class="log-container">
            <h3>24-HOUR HISTORY (HOURLY AVG)</h3>
            <div class="card history-card">
                <div class="table-scroll">
                    <table>
                        <thead>
                            <tr>
                                <th>Hour</th>
                                <th>Temp</th>
                                <th>Humidity</th>
                                <th>Motion</th>
                            </tr>
                        </thead>
                        <tbody id="logBody">
                            <tr>
                                <td colspan="4" class="empty-row">No history yet</td>
                            </tr>
                        </tbody>
                    </table>
                </div>
            </div>
        </div>
    </div>

    <script>
        var lbls = [], tPts = [], hPts = [];
        var ctx = document.getElementById('myChart').getContext('2d');

        var chart = new Chart(ctx, {
            type: 'line',
            data: {
                labels: lbls,
                datasets: [
                    { label: 'Temp', borderColor: '#ff4b2b', backgroundColor: 'rgba(255,75,43,0.15)', data: tPts, tension: 0.3, pointRadius: 0 },
                    { label: 'Hum', borderColor: '#00d2ff', backgroundColor: 'rgba(0,210,255,0.15)', data: hPts, tension: 0.3, pointRadius: 0 }
                ]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                animation: false,
                scales: {
                    x: { display: true, ticks: { color: '#cbd5e1', maxRotation: 0, autoSkip: true, maxTicksLimit: 6 }, grid: { color: '#334155' } },
                    y: { ticks: { color: '#cbd5e1' }, grid: { color: '#334155' } }
                },
                plugins: { legend: { labels: { color: 'white' } } }
            }
        });

        function getTempColor(temp) {
            if (temp >= 30) return "#ff4b2b";
            if (temp >= 28) return "#F28C28";
            if (temp >= 20) return "#2ecc71";
            return "#3498db";
        }

        function getPowerColor(power) {
            if (power >= 80) return "#ff4b2b";
            if (power >= 40) return "#F28C28";
            if (power > 0) return "#2ecc71";
            return "#94a3b8";
        }

        function updateMotorMode(mode) {
            mode = mode || "AUTO";

            document.getElementById('motorMode').innerHTML =
                'Mode: ' + (mode === "AUTO" ? "Dynamic" : "Manual");

            document.getElementById('btnDynamic').classList.toggle('active', mode === "AUTO");
        }

        function updateLedState(state) {
            const on = state === "1" || state === 1 || state === true;
            const ledElem = document.getElementById('led');

            ledElem.innerHTML = on ? "ON" : "OFF";
            ledElem.style.color = on ? "#facc15" : "#94a3b8";

            document.getElementById('ledText').innerHTML =
                on ? "Status: Active" : "Status: Off";

            document.getElementById('btnLedOn').classList.toggle('active', on);
            document.getElementById('btnLedOff').classList.toggle('active', !on);
        }

        function previewManualPower(power) {
            const pVal = parseInt(power);

            document.getElementById('sliderValue').innerHTML = pVal;
            document.getElementById('p').innerHTML = pVal;
            document.getElementById('p').style.color = getPowerColor(pVal);

            updateMotorMode("MANUAL");
        }

        function setManualPower(power) {
            power = parseInt(power);

            if (isNaN(power)) power = 0;
            if (power < 0) power = 0;
            if (power > 100) power = 100;

            fetch('/motor/manual?p=' + power)
                .then(r => r.json())
                .then(d => {
                    if (d.ok) {
                        updateMotorMode(d.mode);
                        document.getElementById('sliderValue').innerHTML = d.p;
                    }
                });
        }

        function setDynamicMode() {
            fetch('/motor/auto')
                .then(r => r.json())
                .then(d => {
                    if (d.ok) updateMotorMode(d.mode);
                });
        }

        function setLed(on) {
            const path = on ? '/led/on' : '/led/off';

            fetch(path)
                .then(r => r.json())
                .then(d => {
                    if (d.ok) updateLedState(d.led);
                });
        }

        function fetchData() {
            fetch('/data')
                .then(r => r.json())
                .then(d => {
                    const tVal = parseFloat(d.t);
                    const hVal = parseFloat(d.h);
                    const pVal = parseInt(d.p);

                    const tElem = document.getElementById('t');
                    const hElem = document.getElementById('h');
                    const mElem = document.getElementById('m');
                    const pElem = document.getElementById('p');
                    const slider = document.getElementById('motorSlider');

                    tElem.innerHTML = isNaN(tVal) ? "--" : d.t;
                    hElem.innerHTML = isNaN(hVal) ? "--" : d.h;

                    if (!isNaN(tVal)) {
                        tElem.style.color = getTempColor(tVal);
                    }

                    if (d.m === "1") {
                        mElem.innerHTML = "YES";
                        mElem.style.color = "#ff4b2b";
                    } else {
                        mElem.innerHTML = "NO";
                        mElem.style.color = "#2ecc71";
                    }

                    document.getElementById('lastMotion').innerHTML = 'Last: ' + d.lastMotion;

                    pElem.innerHTML = isNaN(pVal) ? "--" : pVal;
                    if (!isNaN(pVal)) {
                        pElem.style.color = getPowerColor(pVal);

                        if (document.activeElement !== slider) {
                            slider.value = pVal;
                            document.getElementById('sliderValue').innerHTML = pVal;
                        }
                    }

                    updateMotorMode(d.mode);
                    updateLedState(d.led);

                    if (!isNaN(tVal) && !isNaN(hVal)) {
                        if (lbls.length > 20) {
                            lbls.shift();
                            tPts.shift();
                            hPts.shift();
                        }

                        lbls.push(new Date().toLocaleTimeString([], {
                            hour: '2-digit',
                            minute: '2-digit'
                        }));

                        tPts.push(tVal);
                        hPts.push(hVal);
                        chart.update('none');
                    }
                });
        }

        function fetchHistory() {
            fetch('/history')
                .then(r => r.json())
                .then(data => {
                    if (data.length === 0) {
                        document.getElementById('logBody').innerHTML =
                            '<tr><td colspan="4" class="empty-row">No history yet</td></tr>';
                        return;
                    }

                    let html = "";

                    data.slice().reverse().forEach(row => {
                        const rowColor = getTempColor(row.t);

                        html += `<tr>
                                    <td>${row.hr}:00</td>
                                    <td style="color:${rowColor}; font-weight:bold;">${row.t.toFixed(1)}&deg;C</td>
                                    <td>${row.h.toFixed(1)}%</td>
                                    <td>${row.m}</td>
                                 </tr>`;
                    });

                    document.getElementById('logBody').innerHTML = html;
                });
        }

        fetchData();
        fetchHistory();

        setInterval(fetchData, 2000);
        setInterval(fetchHistory, 60000);
    </script>
</body>
</html>
)=====";
