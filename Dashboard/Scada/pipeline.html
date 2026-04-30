<!DOCTYPE html>
<html>
<head>
<title>Industrial SCADA PRO</title>

<script src="https://cdn.socket.io/4.0.1/socket.io.min.js"></script>
<script src="https://cdn.jsdelivr.net/npm/chart.js"></script>

<style>
body {
    margin:0;
    background:#081421;
    color:white;
    font-family:Arial;
}

.sidebar {
    width:260px;
    position:fixed;
    height:100%;
    background:#1b2435;
    padding:20px;
}

.main {
    margin-left:280px;
    padding:20px;
}

.card {
    background:#222c40;
    padding:15px;
    margin-bottom:15px;
    border-radius:10px;
}

.safe { color:#00ff99; }
.warning { color:orange; }
.critical { color:red; animation: blink 1s infinite; }

@keyframes blink {
    50% { opacity:0.3; }
}
</style>
</head>

<body>

<div class="sidebar">
<h2>🏭 SCADA</h2>

<div class="card">
Gas: <span id="gas">0</span><br>
Flow: <span id="flow">0</span><br>
Pressure: <span id="pressure">0</span>
</div>

<div class="card">
Status: <div id="statusBox" class="safe">SAFE</div>
<div id="alerts"></div>
</div>

<div class="card">
<canvas id="chart"></canvas>
</div>

</div>

<div class="main">
<canvas id="canvas"></canvas>
</div>

<script>
// SOCKET
const socket = io();
let data = {flow:0, gas:0, pressure:0, vibration:0};

// ---------------- RECEIVE DATA ----------------
socket.on("update", d=>{
    data = d;

    gas.innerText = d.gas;
    flow.innerText = d.flow;
    pressure.innerText = d.pressure;

    updateStatus(d);
    updateChart(d);
});

// ---------------- STATUS LOGIC ----------------
function updateStatus(d){
    let alerts = [];

    if(d.gas > 300) alerts.push("🚨 GAS LEAK");
    if(d.flow < 0.3) alerts.push("⚠ LOW FLOW");
    if(d.pressure > 50) alerts.push("⚠ HIGH PRESSURE");
    if(d.vibration > 5) alerts.push("⚠ VIBRATION");

    let statusBox = document.getElementById("statusBox");
    let alertDiv = document.getElementById("alerts");

    if(alerts.length === 0){
        statusBox.innerText = "SAFE";
        statusBox.className = "safe";
        alertDiv.innerHTML = "";
    } 
    else if(alerts.length === 1){
        statusBox.innerText = "WARNING";
        statusBox.className = "warning";
    } 
    else {
        statusBox.innerText = "CRITICAL";
        statusBox.className = "critical";
    }

    alertDiv.innerHTML = alerts.join("<br>");
}

// ---------------- CHART ----------------
const chart = new Chart(document.getElementById("chart"), {
    type:"line",
    data:{
        labels:[],
        datasets:[
            {label:"Pressure", data:[], borderColor:"cyan", tension:0.4},
            {label:"Gas", data:[], borderColor:"red", tension:0.4},
            {label:"Flow", data:[], borderColor:"lime", tension:0.4}
        ]
    },
    options:{
        plugins:{ legend:{ labels:{ color:"white" } } },
        scales:{
            x:{ display:false },
            y:{ ticks:{ color:"white" } }
        }
    }
});

function updateChart(d){
    chart.data.labels.push("");
    chart.data.datasets[0].data.push(d.pressure);
    chart.data.datasets[1].data.push(d.gas);
    chart.data.datasets[2].data.push(d.flow);

    if(chart.data.labels.length > 40){
        chart.data.labels.shift();
        chart.data.datasets.forEach(ds => ds.data.shift());
    }

    chart.update();
}

// ---------------- CANVAS ----------------
const canvas = document.getElementById("canvas");
const ctx = canvas.getContext("2d");

canvas.width = window.innerWidth - 300;
canvas.height = 350;

let particles = [];
let smoke = [];
let wavesOffset = 0;

// -------- GAS PARTICLES --------
function spawnGas(flow){
    for(let i=0;i<flow*2;i++){
        particles.push({
            x:50,
            y:175 + (Math.random()*10-5),
            speed:1 + flow,
            size:1 + Math.random()*2,
            alpha:1
        });
    }
}

// -------- GAS LEAK SMOKE --------
function spawnSmoke(){
    smoke.push({
        x:canvas.width/2,
        y:160,
        size:5,
        alpha:1
    });
}

// -------- DRAW LOOP --------
function draw(){
    ctx.clearRect(0,0,canvas.width,canvas.height);

    let flow = data.flow;
    let gas = data.gas;
    let pressure = data.pressure;

    // PIPE GLOW
    ctx.lineWidth = 18;
    ctx.strokeStyle = "#00cfff";
    ctx.shadowColor = "#00cfff";
    ctx.shadowBlur = 15;

    ctx.beginPath();
    ctx.moveTo(50,175);
    ctx.lineTo(canvas.width-50,175);
    ctx.stroke();

    ctx.shadowBlur = 0;

    // INNER PIPE
    ctx.lineWidth = 10;
    ctx.strokeStyle = "#003344";
    ctx.beginPath();
    ctx.moveTo(50,175);
    ctx.lineTo(canvas.width-50,175);
    ctx.stroke();

    // GAS FLOW PARTICLES
    if(flow > 0.2){
        spawnGas(flow);
    }

    particles.forEach((p,i)=>{
        p.x += p.speed;
        p.alpha -= 0.01;

        ctx.fillStyle = `rgba(0,255,150,${p.alpha})`;
        ctx.beginPath();
        ctx.arc(p.x,p.y,p.size,0,Math.PI*2);
        ctx.fill();

        if(p.x > canvas.width || p.alpha<=0){
            particles.splice(i,1);
        }
    });

    // VALVE
    ctx.fillStyle = flow>1 ? "lime" : "yellow";
    ctx.beginPath();
    ctx.arc(canvas.width/2,175,18,0,Math.PI*2);
    ctx.fill();

    // GAS LEAK ONLY IF GAS HIGH
    if(gas > 300){
        for(let i=0;i<2;i++) spawnSmoke();
    }

    smoke.forEach((s,i)=>{
        s.y -= 0.5;
        s.size += 0.2;
        s.alpha -= 0.01;

        ctx.fillStyle = `rgba(200,200,200,${s.alpha})`;
        ctx.beginPath();
        ctx.arc(s.x,s.y,s.size,0,Math.PI*2);
        ctx.fill();

        if(s.alpha <= 0) smoke.splice(i,1);
    });

    // FLOW WAVES (BOTTOM FILL FIX)
    ctx.strokeStyle = "rgba(0,255,255,0.2)";
    ctx.beginPath();

    for(let x=50;x<canvas.width-50;x++){
        let y = 250 + Math.sin((x + wavesOffset)/30) * flow * 2;
        ctx.lineTo(x,y);
    }

    ctx.stroke();
    wavesOffset += flow;

    // TEXT
    ctx.fillStyle = "white";
    ctx.fillText("Flow: "+flow.toFixed(2), canvas.width/2-40,220);

    requestAnimationFrame(draw);
}

draw();
</script>

</body>
</html>
