import React, { useEffect, useRef } from "react";

function DigitalTwin({ data }) {
  const canvasRef = useRef();

  useEffect(() => {
    const canvas = canvasRef.current;
    const ctx = canvas.getContext("2d");

    canvas.width = window.innerWidth - 40;
    canvas.height = 300;

    let particles = [];

    function spawnFlow(flow) {
      for (let i = 0; i < flow * 2; i++) {
        particles.push({
          x: 120,
          y: 150,
          speed: 1 + flow,
          alpha: 1
        });
      }
    }

    function draw() {
      ctx.clearRect(0, 0, canvas.width, canvas.height);

      const flow = data.flow || 0;
      const gas = data.gas || 0;
      const pressure = data.pressure || 0;

      // 🔵 TANK
      ctx.fillStyle = "#0077ff";
      ctx.fillRect(40, 100, 60, 100);

      ctx.fillStyle = "white";
      ctx.fillText("TANK", 45, 95);

      // 🟡 VALVE
      ctx.fillStyle = flow > 0.5 ? "lime" : "yellow";
      ctx.beginPath();
      ctx.arc(160, 150, 20, 0, Math.PI * 2);
      ctx.fill();

      ctx.fillStyle = "white";
      ctx.fillText("VALVE", 140, 90);

      // 🔴 COMPRESSOR
      ctx.fillStyle = pressure > 15 ? "red" : "#ff8800";
      ctx.fillRect(220, 120, 60, 60);

      ctx.fillStyle = "white";
      ctx.fillText("COMP", 230, 110);

      // ⚪ OUTPUT
      ctx.fillStyle = "#aaa";
      ctx.fillRect(320, 120, 60, 60);

      ctx.fillStyle = "white";
      ctx.fillText("OUT", 335, 110);

      // 🔗 CONNECTION LINES
      ctx.strokeStyle = "#00cfff";
      ctx.lineWidth = 4;

      ctx.beginPath();
      ctx.moveTo(100, 150);
      ctx.lineTo(140, 150);
      ctx.lineTo(200, 150);
      ctx.lineTo(220, 150);
      ctx.lineTo(280, 150);
      ctx.lineTo(320, 150);
      ctx.stroke();

      // 💨 FLOW PARTICLES
      if (flow > 0.2) spawnFlow(flow);

      particles.forEach((p, i) => {
        p.x += p.speed;
        p.alpha -= 0.01;

        ctx.fillStyle = `rgba(0,255,150,${p.alpha})`;
        ctx.beginPath();
        ctx.arc(p.x, p.y, 3, 0, Math.PI * 2);
        ctx.fill();

        if (p.x > canvas.width || p.alpha <= 0) {
          particles.splice(i, 1);
        }
      });

      // 🚨 GAS ALERT EFFECT
      if (gas > 300) {
        ctx.fillStyle = "rgba(255,0,0,0.2)";
        ctx.fillRect(0, 0, canvas.width, canvas.height);

        ctx.fillStyle = "red";
        ctx.font = "20px Arial";
        ctx.fillText("GAS LEAK DETECTED!", 120, 50);
      }

      requestAnimationFrame(draw);
    }

    draw();
  }, [data]);

  return <canvas ref={canvasRef} />;
}

export default DigitalTwin;