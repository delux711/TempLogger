import pandas as pd
import matplotlib.pyplot as plt

# Načítanie súboru
df = pd.read_csv("output.csv", encoding="ISO-8859-1")

# Konverzia času na timedelta
df["Time (seconds)"] = pd.to_timedelta(df["Time (seconds)"])
df["Temperature (°C)"] = df["Temperature (°C)"].astype(float)

# Vykreslenie grafu
plt.figure(figsize=(12, 6))
plt.plot(df["Time (seconds)"].dt.total_seconds() / 3600, df["Temperature (°C)"], label="Teplota (°C)", color="blue")
plt.xlabel("Čas (hodiny)")
plt.ylabel("Teplota (°C)")
plt.title("Teplota v čase")
plt.legend()
plt.grid()

# Zobrazenie grafu
plt.show()
