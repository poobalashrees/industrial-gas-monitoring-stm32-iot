import pandas as pd
import numpy as np
from sklearn.ensemble import RandomForestClassifier
import joblib

data = []

for _ in range(1000):
    gas = np.random.randint(50, 500)
    pressure = np.random.randint(0, 25)
    flow = np.random.uniform(0, 3)
    temp = np.random.randint(20, 60)

    if gas > 350:
        status = 2
    elif gas > 200:
        status = 1
    else:
        status = 0

    data.append([gas, pressure, flow, temp, status])

df = pd.DataFrame(data, columns=["gas","pressure","flow","temp","status"])

X = df[["gas","pressure","flow","temp"]]
y = df["status"]

model = RandomForestClassifier()
model.fit(X, y)

joblib.dump(model, "model.pkl")

print("✅ Model saved as model.pkl")