import React, { useEffect, useState } from "react";
import "./App.css";
import DigitalTwin3D from "./DigitalTwin3D";

import {
  LineChart,
  Line,
  XAxis,
  YAxis,
  Tooltip,
  ResponsiveContainer,
  CartesianGrid,
  Legend
} from "recharts";

function App() {
  const [data, setData] = useState([]);
  const [latest, setLatest] = useState({});
  const [error, setError] = useState("");

  useEffect(() => {
    const interval = setInterval(() => {
      fetch("http://127.0.0.1:5000/data")
        .then((res) => {
          if (!res.ok) throw new Error();
          return res.json();
        })
        .then((newData) => {
          setError("");

          const formatted = {
            ...newData,
            time: new Date(newData.timestamp * 1000).toLocaleTimeString()
          };

          setLatest(formatted);
          setData((prev) => [...prev, formatted].slice(-20));
        })
        .catch(() => setError("⚠ Backend not connected"));
    }, 1000);

    return () => clearInterval(interval);
  }, []);

  const getStatusClass = (status) => {
    if (status === "CRITICAL") return "critical";
    if (status === "WARNING") return "warning";
    if (status === "LOW_FLOW") return "lowflow";
    return "safe";
  };

  const displayTemp = (temp) => {
    return temp === -1 || temp === undefined ? "N/A" : temp;
  };

  return (
    <div className="app">
      <h1>🚀 Smart Industrial Dashboard</h1>

      {error && <div className="error-box">{error}</div>}

      {/* CARDS */}
      <div className="grid">
        <div className="card">Gas<span>{latest.gas ?? 0}</span></div>
        <div className="card">Pressure<span>{latest.pressure ?? 0}</span></div>
        <div className="card">Flow<span>{latest.flow ?? 0}</span></div>
        <div className="card">Temp<span>{displayTemp(latest.temp)}</span></div>
      </div>

      {/* STATUS */}
      <div className="status">
        System:
        <span className={getStatusClass(latest.status)}>
          {latest.status || "SAFE"}
        </span>
      </div>

      <div className="status">
        AI:
        <span className={getStatusClass(latest.ai_status)}>
          {latest.ai_status || "SAFE"}
        </span>
      </div>

      {/* ALERTS */}
      {latest.status === "CRITICAL" && (
        <div className="alert">🚨 CRITICAL ALERT</div>
      )}

      {latest.status === "LOW_FLOW" && (
        <div className="alert lowflow-alert">⚠ LOW FLOW</div>
      )}

      {/* GRAPH */}
      <div className="chart-card">
        <h3>Sensor Trends</h3>

        <ResponsiveContainer width="100%" height={300}>
          <LineChart data={data}>
            <CartesianGrid stroke="#1f2a40" />
            <XAxis dataKey="time" stroke="#aaa" />
            <YAxis stroke="#aaa" />
            <Tooltip />
            <Legend />

            <Line dataKey="gas" stroke="#ff4d4d" strokeWidth={3} />
            <Line dataKey="pressure" stroke="#00d4ff" />
            <Line dataKey="flow" stroke="#00ff88" />
          </LineChart>
        </ResponsiveContainer>
      </div>

      {/* DIGITAL TWIN */}
      <DigitalTwin3D data={latest} />
    </div>
  );
}

export default App;
