# install: pip install pandas plotly

import pandas as pd
import plotly.graph_objects as go

# Načítanie CSV súboru
df = pd.read_csv("output.csv", encoding="ISO-8859-1")

# Konverzia času na hodiny
df["Time (hours)"] = pd.to_timedelta(df["Time (seconds)"]).dt.total_seconds() / 3600

# Vytvorenie grafu
fig = go.Figure()

# Pridanie línie s interaktívnymi bodmi
fig.add_trace(go.Scatter(
    x=df["Time (hours)"], 
    y=df["Temperature (°C)"], 
    mode="lines", 
    name="Teplota (°C)",
    line=dict(color="blue"),
    hoverinfo="x+y"  # Zobrazenie času + teploty pri hoveri
))

# Nastavenie crosshair (plusko)
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
