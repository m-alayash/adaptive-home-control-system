#ifndef WEBSITE_H
#define WEBSITE_H

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
        .val { font-size: 40px; color: #00d2ff; font-weight: bold; }
        .chart-box { max-width: 700px; margin: 20px auto; background: rgba(0,0,0,0.2); padding: 15px; border-radius: 15px; border: 1px solid #334155; }
    </style>
</head>
<body>
    <div class="container">
        <h1>SMART STATION</h1>
        <div class="card"><div>TEMPERATURE</div><div class="val" id="t">--</div>&deg;C</div>
        <div class="card"><div>HUMIDITY</div><div class="val" id="h">--</div>%</div>
        <div class="chart-box"><canvas id="myChart"></canvas></div>
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
        function fetchData() {
            fetch('/data').then(r => r.json()).then(d => {
                document.getElementById('t').innerHTML = d.t;
                document.getElementById('h').innerHTML = d.h;
                if(lbls.length > 20) { lbls.shift(); tPts.shift(); hPts.shift(); }
                lbls.push(""); tPts.push(parseFloat(d.t)); hPts.push(parseFloat(d.h));
                chart.update('none');
            }).catch(err => console.error(err));
        }
        setInterval(fetchData, 2000);
    </script>
</body>
</html>
)=====";

#endif