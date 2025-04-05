from shiny import App, ui, render

# UI aplikacji
app_ui = ui.page_fluid(
    ui.h2("🚀 Test Shiny w sieci lokalnej"),
    ui.input_slider("value", "Wybierz wartość:", 1, 100, 50),
    ui.output_text("result"),
)

# Logika serwera
def server(input, output, session):
    @output
    @render.text
    def result():
        return f"Wybrałeś wartość: {input.value()}"

# Utworzenie aplikacji
app = App(app_ui, server)


shiny run --host 0.0.0.0 --port 8000
http://172.16.16.216:8000
