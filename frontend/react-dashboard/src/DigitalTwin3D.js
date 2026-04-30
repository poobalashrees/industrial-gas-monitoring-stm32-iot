import React, { useRef, useState } from "react";
import { Canvas, useFrame } from "@react-three/fiber";
import { OrbitControls } from "@react-three/drei";

/* 🎨 HEATMAP */
function getHeatColor(value) {
  if (value > 400) return "red";
  if (value > 300) return "orange";
  if (value > 200) return "yellow";
  return "#00d4ff";
}

/* 🟢 FLOW */
function FlowParticle({ speed, type, offset }) {
  const ref = useRef();

  useFrame(({ clock }) => {
    if (!ref.current || speed === 0) return;

    const t = clock.elapsedTime * speed + offset;

    if (type === "x") {
      ref.current.position.set(-5 + (t % 10), 0, 0);
    }

    if (type === "z") {
      ref.current.position.set(5, 0, -(t % 6));
    }
  });

  return (
    <mesh ref={ref}>
      <sphereGeometry args={[0.2]} />
      <meshStandardMaterial color="lime" emissive="lime" emissiveIntensity={2} />
    </mesh>
  );
}

/* 🧱 PIPE */
function Pipe({ data, position, rotation, length = 10, onClick, selected }) {
  const [hovered, setHovered] = useState(false);
  const color = getHeatColor(data.gas);

  return (
    <mesh
      position={position}
      rotation={rotation}
      onClick={onClick}
      onPointerOver={() => setHovered(true)}
      onPointerOut={() => setHovered(false)}
    >
      <cylinderGeometry args={[0.4, 0.4, length, 64]} />
      <meshStandardMaterial
        color={color}
        transparent
        opacity={hovered || selected ? 0.6 : 0.35}
        emissive={color}
        emissiveIntensity={hovered || selected ? 0.5 : 0.2}
      />
    </mesh>
  );
}

/* 🔲 CORNER */
function CornerJoint({ data }) {
  const color = getHeatColor(data.gas);

  return (
    <mesh position={[5, 0, 0]}>
      <boxGeometry args={[0.8, 0.8, 0.8]} />
      <meshStandardMaterial color={color} />
    </mesh>
  );
}

/* 🛢️ TANK */
function Tank({ data }) {
  const color = getHeatColor(data.gas);

  return (
    <mesh position={[-5, 0, 0]}>
      <cylinderGeometry args={[1.5, 1.5, 3, 64]} />
      <meshStandardMaterial color={color} emissive={color} emissiveIntensity={0.2} />
    </mesh>
  );
}

/* 🎬 SCENE */
function Scene({ data, selected, setSelected }) {
  const speed = data.flow > 0 ? data.flow * 0.5 : 0;

  return (
    <>
      <ambientLight intensity={0.5} />
      <directionalLight position={[5, 10, 5]} intensity={1.5} />

      <Tank data={data} />

      <Pipe
        data={data}
        position={[0, 0, 0]}
        rotation={[0, 0, Math.PI / 2]}
        onClick={() => setSelected("Horizontal Pipe")}
        selected={selected === "Horizontal Pipe"}
      />

      <Pipe
        data={data}
        position={[5, 0, -3]}
        rotation={[Math.PI / 2, 0, 0]}
        length={6}
        onClick={() => setSelected("Vertical Pipe")}
        selected={selected === "Vertical Pipe"}
      />

      <CornerJoint data={data} />

      {speed > 0 && (
        <>
          {[...Array(10)].map((_, i) => (
            <FlowParticle key={i} speed={speed} type="x" offset={i} />
          ))}
          {[...Array(8)].map((_, i) => (
            <FlowParticle key={i + 20} speed={speed} type="z" offset={i} />
          ))}
        </>
      )}

      <OrbitControls />
    </>
  );
}

/* 💎 DATA CARD UI */
function DataCard({ data, selected }) {
  if (!selected) return null;

  return (
    <div
      style={{
        position: "absolute",
        top: 100,
        right: 30,
        padding: "20px",
        width: "220px",
        borderRadius: "16px",
        background: "rgba(255,255,255,0.08)",
        backdropFilter: "blur(10px)",
        color: "white",
        boxShadow: "0 8px 32px rgba(0,0,0,0.3)"
      }}
    >
      <h3 style={{ marginBottom: "10px", color: "#00d4ff" }}>
        📊 {selected}
      </h3>

      <p>Gas: <b>{data.gas}</b></p>
      <p>Temp: <b>{data.temp}</b></p>
      <p>Flow: <b>{data.flow}</b></p>
      <p>Status: <b>{data.status}</b></p>
    </div>
  );
}

/* 🎮 MAIN */
export default function DigitalTwin3D({ data }) {
  const [selected, setSelected] = useState(null);

  return (
    <div style={{ marginTop: "20px", position: "relative" }}>
      <h2 style={{ textAlign: "center", color: "#00d4ff" }}>
        🧠 Smart Digital Twin
      </h2>

      {/* 💎 UI CARD */}
      <DataCard data={data} selected={selected} />

      <div style={{ height: "500px" }}>
        <Canvas camera={{ position: [10, 8, 12] }}>
          <Scene data={data} selected={selected} setSelected={setSelected} />
        </Canvas>
      </div>
    </div>
  );
}