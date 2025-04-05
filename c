import numpy as np
import matplotlib.pyplot as plt
from shiny import App, ui, render, reactive

app_ui = ui.page_fluid(
    ui.h2("🎲 Random Variable Generator and Histogram"),
    ui.input_numeric("sample_size", "Sample size", value=1000, min=10, max=10000),
    ui.input_numeric("mean", "Mean", value=0),
    ui.input_numeric("std_dev", "Standard deviation", value=1),
    ui.input_action_button("generate", "Generate Sample"),
    ui.hr(),
    ui.output_plot("histogram_plot"),
    ui.output_text("summary_text")
)

def server(input, output, session):

    @reactive.Calc
    @reactive.event(input.generate)
    def generate_data():
        size = input.sample_size()
        mean = input.mean()
        std_dev = input.std_dev()

        # Generujemy dane z rozkładu normalnego
        data = np.random.normal(loc=mean, scale=std_dev, size=size)
        return data

    @output
    @render.plot
    def histogram_plot():
        data = generate_data()
        fig, ax = plt.subplots()
        ax.hist(data, bins=30, color='skyblue', edgecolor='black')
        ax.set_title("Histogram of Generated Data")
        ax.set_xlabel("Value")
        ax.set_ylabel("Frequency")
        return fig

    @output
    @render.text
    def summary_text():
        data = generate_data()
        return (
            f"Sample generated!\n"
            f"Size: {len(data)}\n"
            f"Mean: {np.mean(data):.2f}\n"
            f"Standard deviation: {np.std(data):.2f}"
        )

app = App(app_ui, server)
