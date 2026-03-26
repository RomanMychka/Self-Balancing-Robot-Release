#pragma once


const char* controlPageHTML = R"rawHTML(
<!DOCTYPE html>
<html lang="uk">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
<title>Balance Bot</title>
<style>
  * { margin:0; padding:0; box-sizing:border-box; }
  body {
    font-family: 'Segoe UI', sans-serif;
    background: linear-gradient(135deg, #1a1a2e, #16213e, #0f3460);
    display: flex; flex-direction: column;
    height: 100vh; overflow: hidden;
    user-select: none; -webkit-user-select: none;
    touch-action: none;
  }
  header {
    background: rgba(255,255,255,0.05);
    backdrop-filter: blur(10px);
    color: white; padding: 14px 20px;
    display: flex; justify-content: space-between; align-items: center;
    border-bottom: 1px solid rgba(255,255,255,0.1);
  }
  .title { font-size: 20px; font-weight: 600; letter-spacing: 1px; }
  .conn  { display: flex; align-items: center; gap: 8px; font-size: 13px; color: rgba(255,255,255,0.7); }
  .dot   { width:10px; height:10px; border-radius:50%; background:#4ade80;
           box-shadow: 0 0 8px #4ade80; animation: pulse 2s infinite; }
  .dot.off { background:#ef4444; box-shadow: 0 0 8px #ef4444; }
  @keyframes pulse { 0%,100%{opacity:1} 50%{opacity:.4} }
  .main  { flex:1; display:flex; flex-direction:column; justify-content:center; align-items:center; gap:20px; }
  .joystick-wrap { position:relative; width:280px; height:280px; }
  canvas { width:100%; height:100%; border-radius:50%; touch-action:none;
           background:rgba(255,255,255,0.04);
           box-shadow: 0 0 40px rgba(99,102,241,0.3), inset 0 0 0 1px rgba(255,255,255,0.1); }
  .vals  { color:rgba(255,255,255,0.6); font-size:13px; font-family:monospace;
           background:rgba(255,255,255,0.05); padding:8px 20px; border-radius:20px; }
  @media(max-width:400px){ .joystick-wrap{width:240px;height:240px;} }
</style>
</head>
<body>
<header>
  <div class="title">⚖️ Balance Bot</div>
  <div class="conn"><div class="dot" id="dot"></div><span id="connText">OK</span></div>
</header>
<div class="main">
  <div class="joystick-wrap"><canvas id="c"></canvas></div>
  <div class="vals">X: <b id="xv">0</b> &nbsp;|&nbsp; Y: <b id="yv">0</b></div>
</div>
<script>
const canvas = document.getElementById('c');
const ctx    = canvas.getContext('2d');
const dot    = document.getElementById('dot');

// Розмір canvas = фізичний розмір * DPI
const SIZE = canvas.parentElement.offsetWidth;
canvas.width  = SIZE * devicePixelRatio;
canvas.height = SIZE * devicePixelRatio;
ctx.scale(devicePixelRatio, devicePixelRatio);

const cx = SIZE/2, cy = SIZE/2;
const R  = SIZE * 0.36;  // радіус зони джойстика

let sx=0, sy=0, active=false;

function draw(){
  ctx.clearRect(0,0,SIZE,SIZE);
  // Зовнішнє коло
  ctx.beginPath(); ctx.arc(cx,cy,R,0,Math.PI*2);
  ctx.strokeStyle='rgba(255,255,255,0.15)'; ctx.lineWidth=1.5; ctx.stroke();
  // Хрестик по центру
  ctx.strokeStyle='rgba(255,255,255,0.1)'; ctx.lineWidth=1;
  ctx.beginPath(); ctx.moveTo(cx-R,cy); ctx.lineTo(cx+R,cy); ctx.stroke();
  ctx.beginPath(); ctx.moveTo(cx,cy-R); ctx.lineTo(cx,cy+R); ctx.stroke();
  // Стік
  const px = cx + (sx/100)*R;
  const py = cy - (sy/100)*R;
  const g  = ctx.createRadialGradient(px,py,5,px,py,36);
  g.addColorStop(0,'rgba(165,180,252,0.95)');
  g.addColorStop(1,'rgba(99,102,241,0.4)');
  ctx.beginPath(); ctx.arc(px,py,36,0,Math.PI*2);
  ctx.fillStyle=g; ctx.fill();
  ctx.strokeStyle = active ? '#4ade80' : 'rgba(255,255,255,0.5)';
  ctx.lineWidth=2.5; ctx.stroke();
}

function update(cx2, cy2){
  const rect = canvas.getBoundingClientRect();
  const dx = cx2 - rect.left  - cx;
  const dy = cy  - (cy2 - rect.top);
  const d  = Math.sqrt(dx*dx+dy*dy);
  const fx = d>R ? dx/d*R : dx;
  const fy = d>R ? dy/d*R : dy;
  sx = Math.round(fx/R*100);
  sy = Math.round(fy/R*100);
  draw();
  document.getElementById('xv').textContent=sx;
  document.getElementById('yv').textContent=sy;
}
function stop(){ sx=0; sy=0; active=false; draw();
  document.getElementById('xv').textContent=0;
  document.getElementById('yv').textContent=0; }

canvas.addEventListener('pointerdown', e=>{ active=true; update(e.clientX,e.clientY); });
canvas.addEventListener('pointermove', e=>{ if(active) update(e.clientX,e.clientY); });
canvas.addEventListener('pointerup',   stop);
canvas.addEventListener('pointerleave',stop);

// Відправка команд 20 Гц
let lx=null, ly=null;
setInterval(()=>{
  if(sx===lx && sy===ly) return;
  lx=sx; ly=sy;
  fetch('/control?x='+sx+'&y='+sy)
    .then(r=>{ dot.className=r.ok?'dot':'dot off'; })
    .catch(()=>{ dot.className='dot off'; });
},50);

draw();
</script>
</body>
</html>
)rawHTML";