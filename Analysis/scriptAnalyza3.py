import pandas as pd
import plotly.graph_objects as go

# Načítanie CSV súboru
df = pd.read_csv("output.csv", encoding="ISO-8859-1")

# Skontrolovanie hodnoty 'Time (seconds)' pred a po konverzii
print("Hodnoty v Time (seconds) pred konverziou:")
print(df["Time (seconds)"].head())

# Konverzia času na sekundy (ak sú hodnoty vo formáte 'HH:MM:SS')
df["Time (seconds)"] = pd.to_timedelta(df["Time (seconds)"]).dt.total_seconds()  # Prepočítanie na sekundy

# Prepočítanie času na dni, hodiny, minúty
df["Days"] = df["Time (seconds)"] // (3600 * 24)  # Dni
df["Hours"] = (df["Time (seconds)"] % (3600 * 24)) // 3600  # Hodiny
df["Minutes"] = (df["Time (seconds)"] % 3600) // 60  # Minúty

# Vytvorenie nový stĺpec s časom vo formáte 'dni hodiny:minúty'
df["Formatted Time"] = df["Days"].astype(int).astype(str) + " d " + df["Hours"].astype(int).astype(str) + ":" + df["Minutes"].astype(int).astype(str).str.zfill(2)

# Skontrolovanie maximálnej hodnoty po konverzii
print("Maximálny čas v formáte 'd HH:MM':", df["Formatted Time"].iloc[-1])

# Vytvorenie grafu
fig = go.Figure()

# Pridanie línie s interaktívnymi bodmi
fig.add_trace(go.Scatter(
    x=df["Time (seconds)"] / 3600,  # Prepočítanie na hodiny pre graf
    y=df["Temperature (°C)"], 
    mode="lines", 
    name="Teplota (°C)",
    line=dict(color="blue"),
    hoverinfo="x+y"  # Zobrazenie času + teploty pri hoveri
))

# Nastavenie grafu
fig.update_layout(
    title="Teplota v čase",
    xaxis_title="Čas (hodiny)",
    yaxis_title="Teplota (°C)",
    hovermode="x",  # Zobrazí len hodnoty, ktoré majú rovnaký X (čas)
    xaxis=dict(
        showspikes=True, spikemode="across", spikethickness=1, spikecolor="gray",
        rangeslider=dict(visible=True),  # Posuvník na X osi
        fixedrange=False  # Povolenie zoomovania len na X
    ),
    yaxis=dict(
        showspikes=True, spikemode="across", spikethickness=1, spikecolor="gray",
        fixedrange=True  # Zamknutie zoomovania na Y osi
    )
)

# Povolenie zoomovania kolieskom iba na os X
fig.show(config={"scrollZoom": True})
