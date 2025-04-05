from shiny import App, ui, render, reactive
import datetime

logfile = "log.txt"

# Funkcja do zapisu do logu
def log_interaction(name, number):
    with open(logfile, "a") as f:
        now = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        f.write(f"{now} - {name} selected number {number}\n")

app_ui = ui.page_fluid(
    ui.h2("Interactive Test App 🧩"),
    ui.input_text("username", "Enter your name:", placeholder="Your name"),
    ui.input_slider("num", "Choose a number:", min=1, max=100, value=50),
    ui.input_action_button("submit", "Submit"),
    ui.hr(),
    ui.output_text("result"),
    ui.hr(),
    ui.output_text_verbatim("log_display")
)

def server(input, output, session):

    # Reaktywna funkcja dla przycisku
    @reactive.Effect
    @reactive.event(input.submit)
    def _():
        name = input.username() or "Anonymous"
        number = input.num()
        log_interaction(name, number)

    # Wynik dla użytkownika na żywo
    @output
    @render.text
    def result():
        name = input.username() or "Anonymous"
        number = input.num()
        return f"{name}, you selected number {number}. Its double is {number * 2}!"

    # Wyświetlenie logów na stronie (na żywo)
    @output
    @render.text
    def log_display():
        try:
            with open(logfile, "r") as f:
                return f.read()
        except FileNotFoundError:
            return "No logs yet."

app = App(app_ui, server)



shiny run --host 0.0.0.0 --port 8000 app.py
http://172.25.210.87:8000

