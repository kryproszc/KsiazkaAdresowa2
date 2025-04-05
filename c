from shiny import App, ui

# UI - to, co widać w przeglądarce
app_ui = ui.page_fluid(
    "Hello, world!"  # Prosty tekst
)

# Serwer - logika aplikacji (tu na razie pusto)
def server(input, output, session):
    pass

# Tworzymy aplikację
app = App(app_ui, server)
