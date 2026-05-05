const char PAGE_DATA[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Adaptive Home</title>
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <style>
        body { background: #0f172a; color: white; font-family: sans-serif; text-align: center; margin: 0; }
        .container { padding: 20px; }
        .status-grid { display: flex; justify-content: center; align-items: stretch; gap: 20px; flex-wrap: wrap; margin: 20px 0; }
        .card {
            background: rgba(255,255,255,0.05);
            border-radius: 15px;
            padding: 20px;
            display: inline-block;
            margin: 10px;
            min-width: 160px;
            max-width: 100%;
            box-sizing: border-box;
            border: 1px solid #334155;
        }
        .status-grid .card { width: 220px; min-height: 150px; }
        .val { font-size: 40px; color: #00d2ff; font-weight: bold; transition: color 0.5s; }
        .subtext { color: #94a3b8; font-size: 13px; margin-top: 8px; overflow-wrap: anywhere; }
        .slider-wrap { margin-top: 12px; }
        input[type="range"] { width: 100%; accent-color: #00d2ff; }
        .motor-controls { display: flex; gap: 6px; justify-content: center; flex-wrap: wrap; margin-top: 10px; }
        .motor-controls button {
            background: #1e293b;
            color: #cbd5e1;
            border: 1px solid #475569;
            border-radius: 8px;
            padding: 7px 10px;
            cursor: pointer;
            font-size: 12px;
        }
        .motor-controls button.active { background: #00d2ff; color: #0f172a; border-color: #00d2ff; font-weight: bold; }
        .chart-box {
            max-width: 700px;
            margin: 20px auto;
            background: rgba(0,0,0,0.2);
            padding: 15px;
            border-radius: 15px;
            border: 1px solid #334155;
            box-sizing: border-box;
        }
        table { width: 100%; border-collapse: collapse; margin-top: 10px; }
        th { color: #94a3b8; font-size: 12px; text-transform: uppercase; padding: 10px; border-bottom: 1px solid #334155; }
        td { padding: 10px; border-bottom: 1px solid #1e293b; }
        .log-container { max-width: 700px; margin: 20px auto; }
        .history-card { width: 100%; display: block; }
        .empty-row { color: #94a3b8; }
    </style>
</head>
<body>
    <div class="container">
        <h1>Adaptive Home Control System</h1>

        <div class="status-grid">
            <div class="card">
                <div>TEMPERATURE</div>
                <div class="val" id="t">--</div>&deg;C
            </div>

            <div class="card">
                <div>HUMIDITY</div>
                <div class="val" id="h">--</div>%
            </div>

            <div class="card">
                <div>MOTION</div>
                <div class="val" id="m">--</div>
                <div class="subtext" id="lastMotion">Last: Never</div>
            </div>

            <div class="card">
                <div>MOTOR</div>
                <div class="val" id="p">--</div>%
                <div class="subtext" id="motorMode">Mode: Dynamic</div>
                <div class="slider-wrap">
                    <input id="motorSlider" type="range" min="0" max="100" value="0"
                           oninput="previewManualPower(this.value)"
                           onchange="setManualPower(this.value)">
                    <div class="subtext">Manual: <span id="sliderValue">0</span>%</div>
                </div>
                <div class="motor-controls">
                    <button id="btnDynamic" onclick="setDynamicMode()">Dynamic</button>
                </div>
            </div>
        </div>

        <div class="chart-box">
            <canvas id="myChart"></canvas>
        </div>

        <div class="log-container">
            <h3>24-HOUR HISTORY (HOURLY AVG)</h3>
            <div class="card history-card">
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
