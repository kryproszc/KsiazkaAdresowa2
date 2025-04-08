# === Importy ===
from shiny import App, ui, render, reactive, run_app, session
import shinyswatch
import pandas as pd
from htmltools import HTML
from io import BytesIO
from openpyxl import load_workbook
import asyncio
from asyncio import to_thread
import numbers
import io

# === Klasa na dane sesyjne użytkownika ===
class UserSessionData:
    def __init__(self):
        self.reactive_data_paid_list = reactive.Value([])
        self.reactive_data_incurred_list = reactive.Value([])
        self.reactive_data_exposure_list = reactive.Value([])
        self.current_triangle = reactive.Value("triangle")
        self.modal_html = reactive.Value(HTML("Brak danych"))

# === Funkcja ładowania danych z pliku Excel ===
def load_excel_data(file_path, sheet_name, start_row, start_col, end_row, end_col):
    workbook = load_workbook(file_path)
    if sheet_name not in workbook.sheetnames:
        raise ValueError(f"W pliku nie znaleziono arkusza '{sheet_name}'.")
    data = pd.read_excel(
        file_path,
        sheet_name=sheet_name,
        header=0,
        engine="openpyxl",
        skiprows=start_row - 1,
        usecols=range(start_col - 1, end_col),
        nrows=end_row - start_row + 1
    )
    return data

# === UI ===
app_ui = ui.page_fluid(

    # Spinner frontendowy
    ui.tags.div(
        {"id": "custom-spinner", "style": "display: none; position: fixed; top: 0; left: 0; width: 100%; height: 100%; background: rgba(0, 0, 0, 0.5); z-index: 9999; text-align: center; padding-top: 20%;"},
        ui.tags.div({"class": "spinner-border text-light", "role": "status", "style": "width: 5rem; height: 5rem;"},
                    ui.tags.span({"class": "sr-only"}, "Loading...")),
        ui.tags.div({"style": "color: white; margin-top: 20px; font-size: 18px;"}, "Proszę czekać, dane są wczytywane...")
    ),

    # Frontendowy skrypt do spinnera
    ui.tags.script("""
        function showSpinner() {
            document.getElementById('custom-spinner').style.display = 'block';
        }
        function hideSpinner() {
            document.getElementById('custom-spinner').style.display = 'none';
        }
        document.addEventListener('DOMContentLoaded', function() {
            hideSpinner();
        });
        Shiny.addCustomMessageHandler("hideSpinner", function(message) {
            hideSpinner();
        });
        const observer = new MutationObserver(() => {
            const modals = document.querySelectorAll('.modal');
            if (modals.length > 0 && modals[0].style.display !== 'none') {
                hideSpinner();
            }
        });
        observer.observe(document.body, { childList: true, subtree: true });
        setTimeout(() => hideSpinner(), 30000);
    """),

    ui.page_navbar(
        ui.nav_panel("REZERWY TESTY", ui.output_image("image")),
        ui.nav_panel("Wprowadź dane",
            ui.row(
                # Paid
                ui.column(4, "Wprowadź trójkąt Paid",
                    ui.input_file("file_input_paid", "Wybierz plik Excel", accept=[".xlsx"], multiple=False),
                    ui.input_select("sheet_select_paid", "Arkusze w pliku:", choices=[]),
                    ui.input_numeric("start_row", "Wiersz początkowy:", value=2),
                    ui.input_numeric("start_col", "Kolumna początkowa:", value=2),
                    ui.input_numeric("end_row", "Wiersz końcowy:", value=5),
                    ui.input_numeric("end_col", "Kolumna końcowa:", value=4),
                    ui.input_radio_buttons("radio_button_paid", "Wartości trójkąta", {"Skumulowane": "Skumulowane", "Inkrementalne": "Inkrementalne"}),
                    ui.input_action_button("load_button", "Wczytaj dane", onclick="showSpinner()"),
                    ui.input_action_button("check_button_paid", "Sprawdź dane"),
                    ui.input_action_button("download_button_paid", "Pobierz dane")
                ),
                # Incurred
                ui.column(4, "Wprowadź trójkąt Incurred",
                    ui.input_file("file_input_inc", "Wybierz plik Excel", accept=[".xlsx"], multiple=False),
                    ui.input_select("sheet_select_inc", "Arkusze w pliku:", choices=[]),
                    ui.input_numeric("start_row_inc", "Wiersz początkowy:", value=2),
                    ui.input_numeric("start_col_inc", "Kolumna początkowa:", value=2),
                    ui.input_numeric("end_row_inc", "Wiersz końcowy:", value=5),
                    ui.input_numeric("end_col_inc", "Kolumna końcowa:", value=4),
                    ui.input_radio_buttons("radio_button_inc", "Wartości trójkąta", {"Skumulowane": "Skumulowane", "Inkrementalne": "Inkrementalne"}),
                    ui.input_action_button("load_button_inc", "Wczytaj dane", onclick="showSpinner()"),
                    ui.input_action_button("check_button_inc", "Sprawdź dane"),
                    ui.input_action_button("download_button_inc", "Pobierz dane")
                ),
                # Exposure
                ui.column(4, "Wprowadź trójkąt ekspozycję",
                    ui.input_file("file_input_eksp", "Wybierz plik Excel", accept=[".xlsx"], multiple=False),
                    ui.input_select("sheet_select_eksp", "Arkusze w pliku:", choices=[]),
                    ui.input_numeric("start_row_eksp", "Wiersz początkowy:", value=2),
                    ui.input_numeric("start_col_eksp", "Kolumna początkowa:", value=2),
                    ui.input_numeric("end_row_eksp", "Wiersz końcowy:", value=5),
                    ui.input_numeric("end_col_eksp", "Kolumna końcowa:", value=4),
                    ui.input_action_button("load_button_eksp", "Wczytaj dane", onclick="showSpinner()"),
                    ui.input_action_button("check_button_eksp", "Sprawdź dane"),
                    ui.input_action_button("download_button_eksp", "Pobierz dane")
                )
            )
        ),
        ui.nav_panel("Metody deterministyczne",
            ui.navset_tab(
                ui.nav_panel("Chain Ladder Paid", ui.output_table("triangle_output"))
            )
        ),
        shinyswatch.theme.superhero(),
    )
)

# === Serwer ===
def server(input, output, session):
    if not hasattr(session, "user_data"):
        session.user_data = UserSessionData()
    user_data = session.user_data

    def hide_spinner():
        session.send_custom_message("hideSpinner", None)

    # Dynamiczne ładowanie arkuszy
    def setup_file_input(file_input_id, sheet_select_id):
        @reactive.Effect
        @reactive.event(getattr(input, file_input_id))
        def _():
            file_info = getattr(input, file_input_id)()
            if not file_info:
                ui.update_select(sheet_select_id, choices=[])
                return
            with open(file_info[0]["datapath"], "rb") as f:
                excel_bytes = BytesIO(f.read())
            try:
                wb = load_workbook(excel_bytes, read_only=True)
                sheet_names = wb.sheetnames
                ui.update_select(sheet_select_id, choices=sheet_names, selected=sheet_names[0])
            except Exception as e:
                print(f"Błąd przy odczycie pliku Excel: {e}")
                ui.update_select(sheet_select_id, choices=[])

    setup_file_input("file_input_paid", "sheet_select_paid")
    setup_file_input("file_input_inc", "sheet_select_inc")
    setup_file_input("file_input_eksp", "sheet_select_eksp")

    def get_current_data():
        triangle = user_data.current_triangle.get()
        data_map = {
            "triangle_paid": user_data.reactive_data_paid_list,
            "triangle_incurred": user_data.reactive_data_incurred_list,
            "triangle_exposure": user_data.reactive_data_exposure_list
        }
        data_list = data_map.get(triangle)
        return data_list.get()[-1] if data_list and data_list.get() else None

    def validate_triangle(dataframe):
        errors = set()
        rows, cols = dataframe.shape

        # Zakładamy, że pierwszy wiersz i pierwsza kolumna to nagłówki
        for col_idx in range(1, cols):
            max_row = rows - (col_idx - 1)
            for row_idx in range(1, rows):
                value = dataframe.iloc[row_idx, col_idx]
                if row_idx < max_row and (pd.isna(value) or not isinstance(value, numbers.Number)):
                    errors.add((row_idx, col_idx))
        return errors

    def dataframe_to_html(dataframe, errors):
        html = "<table class='table table-bordered'><thead><tr>"
        for col in dataframe.columns:
            html += f"<th>{col}</th>"
        html += "</tr></thead><tbody>"
        for row_idx in range(dataframe.shape[0]):
            html += "<tr>"
            for col_idx in range(dataframe.shape[1]):
                value = dataframe.iloc[row_idx, col_idx]
                style = "style='color: red; font-weight: bold;'" if (row_idx, col_idx) in errors else ""
                html += f"<td {style}>{'' if pd.isna(value) else value}</td>"
            html += "</tr>"
        html += "</tbody></table>"
        return HTML(html)

    def show_info_modal(message, error=False):
        icon = "✅" if not error else "❌"
        bg_color = "#d4edda" if not error else "#f8d7da"
        text_color = "#155724" if not error else "#721c24"
        border_color = "#c3e6cb" if not error else "#f5c6cb"
        ui.modal_show(
            ui.modal(
                ui.tags.div(
                    {"style": f"""
                        background-color: {bg_color};
                        color: {text_color};
                        border: 1px solid {border_color};
                        padding: 20px;
                        border-radius: 5px;
                        text-align: center;
                    """},
                    ui.tags.h4(f"{icon} {message}")
                ),
                title="Informacja",
                easy_close=True,
                footer=None
            )
        )

    def show_modal(title):
        ui.modal_show(
            ui.modal(
                ui.tags.div(
                    {"style": "overflow-x: auto; overflow-y: auto; max-height: 80vh;"},
                    ui.output_ui("modal_table")
                ),
                title=title,
                easy_close=True,
                footer=None,
                size="l"
            )
        )

    @output
    @render.ui
    def modal_table():
        return user_data.modal_html.get()

    async def load_data(file_input, sheet_select, start_row, start_col, end_row, end_col, triangle_type, cumulative_option=None):
        try:
            file_info = file_input()
            if not file_info:
                raise ValueError("Nie wybrano pliku.")
            file_path = file_info[0]["datapath"]
            sheet_name = sheet_select()
            if not sheet_name:
                raise ValueError("Nie wybrano arkusza.")

            await asyncio.sleep(0.1)
            data = await to_thread(load_excel_data, file_path, sheet_name, start_row, start_col, end_row, end_col)

            if cumulative_option == "Inkrementalne":
                data.iloc[:, 1:] = data.iloc[:, 1:].cumsum(axis=1)

            if triangle_type == "triangle_paid":
                user_data.reactive_data_paid_list.get().append(data)
            elif triangle_type == "triangle_incurred":
                user_data.reactive_data_incurred_list.get().append(data)
            elif triangle_type == "triangle_exposure":
                user_data.reactive_data_exposure_list.get().append(data)

            hide_spinner()
            show_info_modal("Dane zostały wczytane.")

        except Exception as e:
            hide_spinner()
            show_info_modal(f"Błąd: {e}", error=True)

    # Obsługa przycisków ładowania danych
    @reactive.Effect
    @reactive.event(input.load_button)
    def _():
        asyncio.create_task(load_data(input.file_input_paid, input.sheet_select_paid, input.start_row(), input.start_col(), input.end_row(), input.end_col(), "triangle_paid", input.radio_button_paid()))

    @reactive.Effect
    @reactive.event(input.load_button_inc)
    def _():
        asyncio.create_task(load_data(input.file_input_inc, input.sheet_select_inc, input.start_row_inc(), input.start_col_inc(), input.end_row_inc(), input.end_col_inc(), "triangle_incurred", input.radio_button_inc()))

    @reactive.Effect
    @reactive.event(input.load_button_eksp)
    def _():
        asyncio.create_task(load_data(input.file_input_eksp, input.sheet_select_eksp, input.start_row_eksp(), input.start_col_eksp(), input.end_row_eksp(), input.end_col_eksp(), "triangle_exposure"))

    # === Sprawdzanie danych ===
    def check_data(triangle):
        user_data.current_triangle.set(triangle)
        data = get_current_data()
        if data is None:
            user_data.modal_html.set(HTML("<p style='color: red;'>Brak danych do sprawdzenia.</p>"))
        else:
            errors = validate_triangle(data)
            user_data.modal_html.set(dataframe_to_html(data, errors))
        show_modal("Podgląd wczytanego trójkąta")

    @reactive.Effect
    @reactive.event(input.check_button_paid)
    def _():
        check_data("triangle_paid")

    @reactive.Effect
    @reactive.event(input.check_button_inc)
    def _():
        check_data("triangle_incurred")

    @reactive.Effect
    @reactive.event(input.check_button_eksp)
    def _():
        check_data("triangle_exposure")

    # === Pobieranie danych ===
    def show_download_modal():
        ui.modal_show(
            ui.modal(
                ui.input_radio_buttons("file_format", "Format pliku:", {"csv": "CSV", "excel": "Excel"}, selected="csv"),
                ui.download_button("download_modal_data", "Pobierz"),
                ui.modal_button("Zamknij"),
                title="Pobierz dane",
                easy_close=True,
                footer=None
            )
        )

    @reactive.Effect
    @reactive.event(input.download_button_paid)
    def _():
        user_data.current_triangle.set("triangle_paid")
        show_download_modal()

    @reactive.Effect
    @reactive.event(input.download_button_inc)
    def _():
        user_data.current_triangle.set("triangle_incurred")
        show_download_modal()

    @reactive.Effect
    @reactive.event(input.download_button_eksp)
    def _():
        user_data.current_triangle.set("triangle_exposure")
        show_download_modal()

    @output
    @render.download(filename=lambda: f"{user_data.current_triangle.get()}.{'csv' if input.file_format() == 'csv' else 'xlsx'}")
    def download_modal_data():
        data = get_current_data()
        if data is None:
            yield b""
        else:
            if input.file_format() == "csv":
                yield data.to_csv(index=False).encode("utf-8")
            else:
                buffer = io.BytesIO()
                data.to_excel(buffer, index=False)
                yield buffer.getvalue()

    # === Wyświetlanie danych w zakładce "Metody deterministyczne" ===
    @output
    @render.table
    def triangle_output():
        data = get_current_data()
        return data if data is not None else pd.DataFrame({"Info": ["Brak danych"]})

# === Uruchomienie aplikacji ===
app = App(app_ui, server)
run_app(app, port=8003)
