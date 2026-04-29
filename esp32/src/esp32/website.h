const char PAGE_DATA[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <link rel="icon" type="image/png" href="data:image/png;base64,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">
    <title>Adaptive Home</title>
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <style>
        body { background: #0f172a; color: white; font-family: sans-serif; text-align: center; margin: 0; }
        .container { padding: 20px; }
        .card { background: rgba(255,255,255,0.05); border-radius: 15px; padding: 20px; display: inline-block; margin: 10px; min-width: 160px; border: 1px solid #334155; }
        .val { font-size: 40px; color: #00d2ff; font-weight: bold; transition: color 0.5s; }
        .chart-box { max-width: 700px; margin: 20px auto; background: rgba(0,0,0,0.2); padding: 15px; border-radius: 15px; border: 1px solid #334155; }
        table { width: 100%; border-collapse: collapse; margin-top: 10px; }
        th { color: #94a3b8; font-size: 12px; text-transform: uppercase; padding: 10px; border-bottom: 1px solid #334155; }
        td { padding: 10px; border-bottom: 1px solid #1e293b; }
        .log-container { max-width: 700px; margin: 20px auto; }
    </style>
</head>
<body>
    <div class="container">
        <h1>Adaptive Home Control System</h1>
        <div class="card"><div>TEMPERATURE</div><div class="val" id="t">--</div>&deg;C</div>
        <div class="card"><div>HUMIDITY</div><div class="val" id="h">--</div>%</div>
        <div class="chart-box"><canvas id="myChart"></canvas></div>
        
        <div class="log-container">
            <h3>24-HOUR HISTORY (HOURLY AVG)</h3>
            <div class="card" style="width:100%; display:block;">
                <table>
                    <thead><tr><th>Hour</th><th>Temp</th><th>Humidity</th></tr></thead>
                    <tbody id="logBody"></tbody>
                </table>
            </div>
        </div>
    </div>
    <script>
        var lbls=[], tPts=[], hPts=[];
        var ctx = document.getElementById('myChart').getContext('2d');
        var chart = new Chart(ctx, {
            type: 'line',
            data: { labels: lbls, datasets: [
                { label: 'Temp', borderColor: '#ff4b2b', data: tPts, tension: 0.3, pointRadius: 0 },
                { label: 'Hum', borderColor: '#00d2ff', data: hPts, tension: 0.3, pointRadius: 0 }
            ]},
            options: { scales: { x: { display: false }, y: { grid: { color: '#334155' } } }, plugins: { legend: { labels: { color: 'white' } } } }
        });

        // Helper function to pick color based on Temp
        function getTempColor(temp) {
            if (temp >= 30) return "#ff4b2b"; // Red
            if (temp >= 28) return "#F28C28"; // Orange 
            if (temp >= 20) return "#2ecc71"; // Green
            return "#3498db";                 // Blue
        }

        function fetchData() {
            fetch('/data').then(r => r.json()).then(d => {
                const tVal = parseFloat(d.t);
                const tElem = document.getElementById('t');
                
                // Update Value and Color
                tElem.innerHTML = d.t;
                tElem.style.color = getTempColor(tVal);
                
                document.getElementById('h').innerHTML = d.h;

                if(lbls.length > 20) { lbls.shift(); tPts.shift(); hPts.shift(); }
                lbls.push(""); tPts.push(tVal); hPts.push(parseFloat(d.h));
                chart.update('none');
            });
        }

        function fetchHistory() {
            fetch('/history').then(r => r.json()).then(data => {
                let html = "";
                data.reverse().forEach(row => {
                    const rowColor = getTempColor(row.t);
                    html += `<tr>
                                <td>${row.hr}:00</td>
                                <td style="color:${rowColor}; font-weight:bold;">${row.t.toFixed(1)}°C</td>
                                <td>${row.h.toFixed(1)}%</td>
                             </tr>`;
                });
                document.getElementById('logBody').innerHTML = html;
            });
        }

        setInterval(fetchData, 2000);
        setInterval(fetchHistory, 60000);
        fetchHistory();
    </script>
</body>
</html>
)=====";