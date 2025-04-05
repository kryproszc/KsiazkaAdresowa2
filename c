import numpy as np
import matplotlib.pyplot as plt
from shiny import App, ui, render, reactive

# --- UI aplikacji ---

app_ui = ui.page_fluid(
    ui.navset_tab(
        ui.nav("🏠 Start",
            ui.h2("Witaj w aplikacji testowej 🚀"),
            ui.p("Przejdź do zakładki 'Symulacja', aby wygenerować dane.")
        ),
        ui.nav("🎲 Symulacja",
            ui.h3("Symulacja danych"),
            ui.input_numeric("sample_size", "Rozmiar próbki", value=1000, min=100, max=10000),
            ui.input_numeric("mean", "Średnia", value=0),
            ui.input_numeric("std_dev", "Odchylenie standardowe", value=1),
            ui.input_action_button("generate", "Generuj dane"),
            ui.output_text("sim_status")
        ),
        ui.nav("📊 Wyniki",
            ui.h3("Wyniki symulacji"),
            ui.output_text("summary_text"),
            ui.output_plot("histogram_plot")
        ),
        ui.nav("ℹ️ Informacje",
            ui.h3("Informacje o aplikacji"),
            ui.p("Ta aplikacja pozwala na testowanie wielozakładkowej struktury z przepływem danych pomiędzy zakładkami."),
            ui.p("Idealna do testów wieloużytkownikowych.")
        )
    )
)

# --- Serwer aplikacji ---

def server(input, output, session):

    # Reaktywna zmienna do przechowywania danych
    data_store = reactive.Value(np.array([]))

    @reactive.Effect
    @reactive.event(input.generate)
    def generate_data():
        size = input.sample_size()
        mean = input.mean()
        std_dev = input.std_dev()

        data = np.random.normal(loc=mean, scale=std_dev, size=size)
        data_store.set(data)

    @output
    @render.text
    def sim_status():
        data = data_store.get()
        if data.size == 0:
            return "Brak danych. Wygeneruj próbkę!"
        else:
            return "Dane wygenerowane! Przejdź do zakładki 'Wyniki'."

    @output
    @render.text
    def summary_text():
        data = data_store.get()
        if data.size == 0:
            return "Brak danych. Najpierw wygeneruj próbkę w zakładce 'Symulacja'."
        else:
            return (
                f"Liczba obserwacji: {len(data)}\n"
                f"Średnia: {np.mean(data):.2f}\n"
                f"Odchylenie standardowe: {np.std(data):.2f}"
            )

    @output
    @render.plot
    def histogram_plot():
        data = data_store.get()
        if data.size == 0:
            fig, ax = plt.subplots()
            ax.text(0.5, 0.5, 'Brak danych', fontsize=14, ha='center')
            ax.axis('off')
            return fig
        else:
            fig, ax = plt.subplots()
            ax.hist(data, bins=30, color='skyblue', edgecolor='black')
            ax.set_title("Histogram wygenerowanych danych")
            ax.set_xlabel("Wartość")
            ax.set_ylabel("Częstość")
            return fig

app = App(app_ui, server)
