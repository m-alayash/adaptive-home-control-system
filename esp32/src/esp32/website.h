const char PAGE_DATA[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>LPC Dashboard</title>
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
        <h1>SMART STATION</h1>
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