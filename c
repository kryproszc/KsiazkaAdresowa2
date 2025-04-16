from shiny import App, ui, render, reactive, run_app, session
import shinyswatch
import pandas as pd
from htmltools import HTML
from io import BytesIO
from openpyxl import load_workbook
import asyncio
from asyncio import to_thread
import numpy as np
import numbers
import io
from triangle_utils import TriangleCalculator
import matplotlib.pyplot as plt
import plotly.graph_objects as go
from htmltools import HTML
import plotly.graph_objects as go
from htmltools import HTML, TagList, tags
import json
from plotly.utils import PlotlyJSONEncoder
from shiny import req




from shinywidgets import output_widget, render_widget
import plotly.graph_objects as go


calculator = TriangleCalculator()


def calculate_ratios_vectorized(df: pd.DataFrame) -> pd.DataFrame:
    data = df.iloc[:, 1:].to_numpy(dtype=float)  # zakładamy, że kol.0 to AY, dalej wartości
    denominator = data[:, :-1]
    numerator = data[:, 1:]
    mask = denominator != 0
    ratios = np.full_like(denominator, fill_value=np.nan)
    ratios[mask] = numerator[mask] / denominator[mask]
    ratio_df = pd.DataFrame(
        np.hstack([ratios, np.full((ratios.shape[0], 1), np.nan)]),
        columns=df.columns[1:],  # Nazwy kolumn od 2. do końca + 1 kolumna NaN
        index=df.index
    )
    return ratio_df


# === Funkcja tworząca macierz binarną (1 = is not NA, NA = brak danych) ===
def create_binary_df(ratio_df: pd.DataFrame) -> pd.DataFrame:
    return ratio_df.map(lambda x: 1 if pd.notna(x) else np.nan)


# === Customowy kod JS i CSS do podświetlania komórek ===
# *KLUCZOWA ZMIANA* w kodzie klikania: row = index() - 1, aby pominąć nagłówek.

js_code = """
$(document).on('click', '#ratios-table-1 td', function() {
    var row = $(this).closest('tr').index();
    var col = $(this).index();

    if (col >= 0) {
        console.log(`Cell clicked: row=${row}, col=${col}`);

        // Zarządzanie tablicą zaznaczonych komórek
        var index = highlightedCells.findIndex(cell => cell.row === row && cell.col === col);

        if ($(this).hasClass('highlighted')) {
            $(this).removeClass('highlighted');
            Shiny.setInputValue('clicked_cell_ratios_table_1', 
                { row: row, col: col, highlighted: false });
            if (index !== -1) highlightedCells.splice(index, 1);
        } else {
            $(this).addClass('highlighted');
            Shiny.setInputValue('clicked_cell_ratios_table_1', 
                { row: row, col: col, highlighted: true });
            if (index === -1) highlightedCells.push({ row: row, col: col, highlighted: true });
        }
    }
});


let highlightedCells = [];
let isUpdatingFromBackend = false;

function highlight_default_cell(offset) {
    var table = document.getElementById('ratios-table-1');
    if (!table) return;

    console.log(`Highlighting cells with offset: ${offset}`);

    // Jeśli offset jest zero, usuwamy podświetlenia i resetujemy Shiny
    if (offset === 0) {
        console.log("Offset is zero, no cells will be highlighted.");

        // Usuń wszystkie istniejące podświetlenia!
        for (let rowIndex = 0; rowIndex < table.rows.length; rowIndex++) {
            for (let colIndex = 0; colIndex < table.rows[rowIndex].cells.length; colIndex++) {
                let cell = table.rows[rowIndex].cells[colIndex];
                cell.classList.remove('highlighted');
            }
        }

        // Wyślij pustą listę z nonce
        Shiny.setInputValue('all_generated_cells_ratios_table_1', { cells: [], nonce: Math.random() }, { priority: "event" });
        return;
    }

    // Usuń stare podświetlenia
    remove_highlights([]);

    let newHighlightedCells = [];
    let baseOffset = -1;
    let foundBase = false;

    for (let colIndex = 0; colIndex < table.rows[0].cells.length; colIndex++) {
        for (let rowIndex = table.rows.length - 1; rowIndex > 0; rowIndex--) {
            let cell = table.rows[rowIndex].cells[colIndex];
            if (cell.innerText.trim() !== '0') {
                if (!foundBase) {
                    baseOffset = rowIndex;
                    foundBase = true;
                }

                let targetRow = baseOffset - (colIndex - 1) - offset - 2;
                if (targetRow >= 1) {
                    for (let highlightRow = targetRow; highlightRow >= 1; highlightRow--) {
                        let targetCell = table.rows[highlightRow].cells[colIndex];

                        if (!targetCell.classList.contains('highlighted')) {
                            targetCell.classList.add('highlighted');
                            newHighlightedCells.push({ row: highlightRow - 1, col: colIndex, highlighted: true });
                        }
                    }
                }
                break;
            }
        }
    }

    highlightedCells = newHighlightedCells;

    // Wyślij listę podświetlonych komórek do Shiny
    Shiny.setInputValue('all_generated_cells_ratios_table_1', highlightedCells, { priority: "event" });
}

function remove_highlights(newHighlightedCells) {
    var table = document.getElementById('ratios-table-1');
    highlightedCells.forEach(function (cell) {
        var targetCell = table.rows[cell.row].cells[cell.col];
        if (!newHighlightedCells.some(newCell => newCell.row === cell.row && newCell.col === cell.col)) {
            targetCell.classList.remove('highlighted');
        }
    });
}

document.addEventListener('DOMContentLoaded', function () {
    Shiny.addCustomMessageHandler('highlight_cells', function (offset) {
        highlight_default_cell(offset);
    });
});



"""

css_code = """
.highlighted {
    background-color: gray !important;
}
.final-row {
    background-color: #2d8f57 !important;
    color: white;
    font-weight: bold;
}
"""


# === Klasa na *sesyjne* dane użytkownika ===
class UserSessionData:
    def __init__(self):
        self.reactive_data_paid_list = reactive.Value([])
        self.reactive_data_incurred_list = reactive.Value([])
        self.reactive_data_exposure_list = reactive.Value([])
        self.current_triangle = reactive.Value("triangle_paid")
        self.modal_html = reactive.Value(HTML("Brak danych"))
        self.ratio_df_p = reactive.Value(None)
        self.binary_df_p = reactive.Value(None)
        self.dev_selected_history = reactive.Value([])
        self.dev_all_initial = reactive.Value(None)
        self.new_cl_selected_cells = reactive.Value(set())
        self.curve_fit_results = reactive.Value(None)
        self.cl_selected_indices = reactive.Value([])
        self.cl_selected_values = reactive.Value([])
        self.selected_tail_vector = reactive.Value([])
        self.final_factor_vector = reactive.Value([])  # ✅
        self.available_projections = reactive.Value(["dev_all", "FINAL", "Factor curve"])

    def get_current_triangle_df(self):
        triangle_name = self.current_triangle.get()

        if triangle_name == "triangle_paid":
            data_list = self.reactive_data_paid_list.get()
        elif triangle_name == "triangle_incurred":
            data_list = self.reactive_data_incurred_list.get()
        elif triangle_name == "triangle_exposure":
            data_list = self.reactive_data_exposure_list.get()
        else:
            raise ValueError(f"Nieznany typ trójkąta: {triangle_name}")

        if not data_list:
            raise ValueError(f"Brak danych dla: {triangle_name}")

        return data_list[0]



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
    ui.tags.style(css_code),
    ui.tags.script(js_code),

    ui.tags.div(
        {"id": "custom-spinner", "style": """
            display: none; position: fixed; top: 0; left: 0; 
            width: 100%; height: 100%; background: rgba(0, 0, 0, 0.5); 
            z-index: 9999; text-align: center; padding-top: 20%;
        """},
        ui.tags.div({"class": "spinner-border text-light", "role": "status", "style": "width: 5rem; height: 5rem;"},
                    ui.tags.span({"class": "sr-only"}, "Loading...")),
        ui.tags.div(
            {"style": "color: white; margin-top: 20px; font-size: 18px;"},
            "Proszę czekać, dane są wczytywane..."
        )
    ),

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

    ui.tags.script("""
        $(document).on('click', '#reset_volumes_button', function(event) {
            event.preventDefault();
            $('#confirmResetVolumeModal').modal('show');
        });

        $(document).on('click', '#confirmResetVolumeButton', function() {
            $('#confirmResetVolumeModal').modal('hide');
            Shiny.setInputValue("confirm_reset_volume", Math.random());
        });
    """),
    ui.tags.script("""
        $(document).on('click', '#export_history_button', function() {
            var finalRowData = [];
            $('#dev-history-table tbody tr.final-row td.final-data-cell').each(function() {
                var cellText = $(this).text().trim().replace(',', '.');
                var cellValue = parseFloat(cellText);
                finalRowData.push(isNaN(cellValue) ? null : cellValue);
            });
            Shiny.setInputValue("final_vector_ui", finalRowData, {priority: "event"});
        });
    """),

    ui.tags.div(
        {"class": "modal fade", "id": "confirmResetVolumeModal", "tabindex": "-1", "role": "dialog",
         "aria-labelledby": "confirmResetLabel", "aria-hidden": "true"},
        ui.tags.div(
            {"class": "modal-dialog", "role": "document"},
            ui.tags.div(
                {"class": "modal-content"},
                ui.tags.div(
                    {"class": "modal-header"},
                    ui.tags.h5("Potwierdzenie resetu", {"class": "modal-title", "id": "confirmResetLabel"})
                ),
                ui.tags.div(
                    {"class": "modal-body"},
                    "Czy na pewno chcesz usunąć wszystkie współczynniki volume?"
                ),
                ui.tags.div(
                    {"class": "modal-footer"},
                    ui.tags.button("Nie", {"type": "button", "class": "btn btn-secondary", "data-dismiss": "modal"}),
                    ui.tags.button("Tak",
                                   {"type": "button", "class": "btn btn-danger", "id": "confirmResetVolumeButton"})
                )
            )
        )
    ),

    ui.page_navbar(
        ui.nav_panel("REZERWY TESTY", ui.output_image("image")),
        ui.nav_panel("Wprowadź dane",
                     ui.row(
                         ui.column(4, "Wprowadź trójkąt Paid",
                                   ui.input_file("file_input_paid", "Wybierz plik Excel", accept=[".xlsx"],
                                                 multiple=False),
                                   ui.input_select("sheet_select_paid", "Arkusze w pliku:", choices=[]),
                                   ui.input_numeric("start_row", "Wiersz początkowy:", value=2),
                                   ui.input_numeric("start_col", "Kolumna początkowa:", value=2),
                                   ui.input_numeric("end_row", "Wiersz końcowy:", value=5),
                                   ui.input_numeric("end_col", "Kolumna końcowa:", value=4),
                                   ui.input_radio_buttons(
                                       "radio_button_paid",
                                       "Wartości trójkąta",
                                       {"Skumulowane": "Skumulowane", "Inkrementalne": "Inkrementalne"}
                                   ),
                                   ui.input_action_button("load_button", "Wczytaj dane", onclick="showSpinner()"),
                                   ui.input_action_button("check_button_paid", "Sprawdź dane"),
                                   ui.input_action_button("download_button_paid", "Pobierz dane")
                                   ),
                         ui.column(4, "Wprowadź trójkąt Incurred",
                                   ui.input_file("file_input_inc", "Wybierz plik Excel", accept=[".xlsx"],
                                                 multiple=False),
                                   ui.input_select("sheet_select_inc", "Arkusze w pliku:", choices=[]),
                                   ui.input_numeric("start_row_inc", "Wiersz początkowy:", value=2),
                                   ui.input_numeric("start_col_inc", "Kolumna początkowa:", value=2),
                                   ui.input_numeric("end_row_inc", "Wiersz końcowy:", value=5),
                                   ui.input_numeric("end_col_inc", "Kolumna końcowa:", value=4),
                                   ui.input_radio_buttons(
                                       "radio_button_inc",
                                       "Wartości trójkąta",
                                       {"Skumulowane": "Skumulowane", "Inkrementalne": "Inkrementalne"}
                                   ),
                                   ui.input_action_button("load_button_inc", "Wczytaj dane", onclick="showSpinner()"),
                                   ui.input_action_button("check_button_inc", "Sprawdź dane"),
                                   ui.input_action_button("download_button_inc", "Pobierz dane")
                                   ),
                         ui.column(4, "Wprowadź trójkąt ekspozycję",
                                   ui.input_file("file_input_eksp", "Wybierz plik Excel", accept=[".xlsx"],
                                                 multiple=False),
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
                         ui.nav_panel("Trójkąt",
                                      ui.output_ui("triangle_table_ui")
                                      ),
                         ui.nav_panel("Współczynniki CL",
                                      ui.layout_sidebar(
                                          ui.panel_sidebar(
                                              ui.input_numeric("offset_distance", "Wybierz volume", value=0, min=0),
                                              ui.input_numeric("rounding_precision", "Precyzja zaokrąglenia", value=10,
                                                               min=0, max=10),
                                              ui.input_action_button("calculate_button", "Oblicz",
                                                                     class_="btn-primary"),
                                              ui.download_button("export_history_button", "Eksportuj",
                                                                 class_="btn-success"),
                                              ui.download_button("download_history_data", "Pobierz historię",
                                                                 style="display: none;"),

                                              ui.input_action_button("reset_volumes_button", "RESET",
                                                                     class_="btn-danger"),
                                              width=2
                                          ),
                                          ui.panel_main(
                                              ui.output_ui("ratios_table_ui_p"),
                                              # ui.output_ui("table_dev_paid"),
                                              ui.output_ui("table_dev_history"),
                                              #  ui.output_ui("table_dev_final")

                                          )
                                      )
                                      ),
                         ui.nav_panel("Wagi",
                                      ui.output_ui("binary_ratios_table_ui_p")
                                      ),
                         ui.nav_panel("Dopasowanie krzywej CL",
                                      ui.layout_sidebar(
                                          ui.panel_sidebar(
                                              ui.input_action_button("accept_CL", "Dopasuj krzywą",
                                                                     class_="btn-success"),
                                              ui.input_numeric("curve_obs_count",
                                                               "Ilość obserwacji generowane z krzywej (domyślnie 10)",
                                                               value=10, min=1),
                                              ui.input_action_button("generate_curve_values", "Losuj współczynniki"),
                                              width=2
                                          ),
                                          ui.panel_main(
                                              ui.output_ui("final_coefficients_display"),
                                              ui.tags.p("Współczynniki z krzywej", style="margin-top: 20px;"),
                                              ui.output_ui("generated_curve_values_table"),
                                              ui.tags.p("Tabela dopasowanych wartości z krzywych:",
                                                        style="margin-top: 20px;"),
                                              ui.output_ui("r2_cl_paid"),
                                              ui.tags.p("📉 Porównanie FINAL z dopasowanymi krzywymi:",
                                                        style="margin-top: 25px; font-weight: bold;"),
                                              ui.output_plot("plot_curve_vs_final"),
                                          ),
                                      )
                                      ),
                         ui.nav_panel("Wybór krzywej CL",
                                      ui.layout_sidebar(
                                          ui.panel_sidebar(
                                              ui.input_numeric("num_of_first_factor",
                                                               "Pozostaw rzeczywistych współczynników", value=1),
                                              ui.input_selectize("wyb_krzywa_ogona", "Wybierz ogon z krzywej",
                                                                 choices=['Exponential', 'Power', "Weibull",
                                                                          "Inverse Power"], multiple=False),
                                              ui.input_action_button("accept_final_factor", "Zatwierdź",
                                                                     class_="btn-success"),
                                              ui.download_button("export_final_vector", "📥 Eksportuj FINAL do Excela",
                                                                 class_="btn-primary"),

                                              width=2
                                          ),
                                          ui.panel_main(
                                              ui.output_table("Final_Factor"),
                                              ui.output_table("Factors_curve"),
                                              ui.output_table("Tail_Factor")
                                          )
                                      )
                                      )
                         ,
                         ui.nav_panel("Wyniki",
                                      ui.layout_sidebar(
                                          ui.panel_sidebar(
                                    ui.output_ui("projection_selection"),

                                              ui.input_action_button(
                                                  id="run_projection",
                                                  label="Policz"
                                              ),

                                            width=2
                                          ),
                                          ui.panel_main(ui.output_ui("triangle_projection_volume"))
                                      )
                                      )
                     )
                     ),
        shinyswatch.theme.superhero(),
    )
)


def server(input, output, session):
    if not hasattr(session, "user_data"):
        session.user_data = UserSessionData()
    user_data = session.user_data
    current_volume = reactive.Value(None)
    volume_versions = reactive.Value({})
    selected_final_index = reactive.Value("dev_all")  # Domyślnie dev_all
    final_values = reactive.Value([])
    final_vector_external = reactive.Value([])  # 🔥 NOWA zmienna do przekazania do innych zakładek
    user_data.show_curve_modal = reactive.Value(False)
    user_data.curve_fit_results = reactive.Value(None)
    user_data.curve_simulation_results = reactive.Value(None)
    table_dev_history_trigger = reactive.Value(0)  # <-- Dodajesz tutaj ✅

    def hide_spinner():
        session.send_custom_message("hideSpinner", None)

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

    def generate_default_data(rows=10, cols=10):
        # Tworzymy lata rozwoju AY
        base_year = 1980
        years = list(range(base_year + 1, base_year + 1 + rows))

        # Tworzymy słownik danych
        data = {"AY": years}

        # Generujemy dane w postaci numpy array
        random_data = np.random.randint(1000, 20000, size=(rows, cols))

        # Pod przekątną i na prawo od niej ustawiamy None
        for i in range(rows):
            for j in range(cols):
                if j > i:
                    random_data[i, j] = None

        # Przekształcamy do słownika
        for col in range(1, cols + 1):
            data[col] = list(random_data[:, col - 1])

        return pd.DataFrame(data)

    def get_current_data():
        triangle = user_data.current_triangle.get()
        data_map = {
            "triangle_paid": user_data.reactive_data_paid_list,
            "triangle_incurred": user_data.reactive_data_incurred_list,
            "triangle_exposure": user_data.reactive_data_exposure_list
        }
        data_list = data_map.get(triangle)
        # Domyślne dane, jeśli nic nie wczytano (tylko dla Paid)
        if (not data_list or not data_list.get()) and triangle == "triangle_paid":
            default_data = {
                "AY": [1981, 1982, 1983, 1984, 1985, 1986, 1987, 1988, 1989, 1990],
                1: [5012, 106, 3410, 5655, 1092, 1513, 557, 1351, 3133, 2063],
                2: [8269, 4285, 8992, 11555, 9565, 6445, 4020, 6947, 5395, None],
                3: [10907, 5396, 13873, 15766, 15836, 11702, 10946, 13112, None, None],
                4: [11805, 10666, 16141, 21266, 22169, 12935, 12314, None, None, None],
                5: [13539, 13782, 18735, 23425, 25955, 15852, None, None, None, None],
                6: [16181, 15599, 22214, 26083, 26180, None, None, None, None, None],
                7: [18009, 15496, 22863, 27067, None, None, None, None, None, None],
                8: [18608, 16169, 23466, None, None, None, None, None, None, None],
                9: [18662, 16704, None, None, None, None, None, None, None, None],
                10: [18834, None, None, None, None, None, None, None, None, None]
            }
            return pd.DataFrame(default_data)

        if data_list and data_list.get():
            return data_list.get()[-1]
        return None

    def validate_triangle(dataframe: pd.DataFrame):
        errors = set()
        rows, cols = dataframe.shape
        for col_idx in range(1, cols):
            max_row = rows - (col_idx - 1)
            for row_idx in range(1, rows):
                value = dataframe.iloc[row_idx, col_idx]
                if row_idx < max_row and (pd.isna(value) or not isinstance(value, numbers.Number)):
                    errors.add((row_idx, col_idx))
        return errors

    def dataframe_to_html(dataframe: pd.DataFrame, errors):
        html = "<table class='table table-bordered'><thead><tr>"
        for col in dataframe.columns:
            html += f"<th>{col}</th>"
        html += "</tr></thead><tbody>"
        for row_idx in range(dataframe.shape[0]):
            html += "<tr>"
            for col_idx in range(dataframe.shape[1]):
                value = dataframe.iloc[row_idx, col_idx]
                style = "style='color: red; font-weight: bold;'" if (row_idx, col_idx) in errors else ""
                display_val = "" if pd.isna(value) else value
                html += f"<td {style}>{display_val}</td>"
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
                    {
                        "style": f"""
                            background-color: {bg_color};
                            color: {text_color};
                            border: 1px solid {border_color};
                            padding: 20px;
                            border-radius: 5px;
                            text-align: center;
                        """
                    },
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

    async def load_data(
            file_input, sheet_select,
            start_row, start_col, end_row, end_col,
            triangle_type, cumulative_option=None
    ):
        try:
            file_info = file_input()
            if not file_info:
                raise ValueError("Nie wybrano pliku.")
            file_path = file_info[0]["datapath"]
            sheet_name = sheet_select()
            if not sheet_name:
                raise ValueError("Nie wybrano arkusza.")

            data = await to_thread(
                load_excel_data,
                file_path, sheet_name, start_row, start_col, end_row, end_col
            )

            if cumulative_option == "Inkrementalne":
                data.iloc[:, 1:] = data.iloc[:, 1:].cumsum(axis=1)

            if triangle_type == "triangle_paid":
                user_data.reactive_data_paid_list.set([data])
            elif triangle_type == "triangle_incurred":
                user_data.reactive_data_incurred_list.set([data])
            elif triangle_type == "triangle_exposure":
                user_data.reactive_data_exposure_list.set([data])

            hide_spinner()
            show_info_modal("Dane zostały wczytane.")
        except Exception as e:
            hide_spinner()
            show_info_modal(f"Błąd: {e}", error=True)

    @reactive.Effect
    @reactive.event(input.load_button)
    def _():
        user_data.current_triangle.set("triangle_paid")
        asyncio.create_task(load_data(
            input.file_input_paid,
            input.sheet_select_paid,
            input.start_row(),
            input.start_col(),
            input.end_row(),
            input.end_col(),
            "triangle_paid",
            input.radio_button_paid()
        ))

    @reactive.Effect
    @reactive.event(input.load_button_inc)
    def _():
        user_data.current_triangle.set("triangle_incurred")
        asyncio.create_task(load_data(
            input.file_input_inc,
            input.sheet_select_inc,
            input.start_row_inc(),
            input.start_col_inc(),
            input.end_row_inc(),
            input.end_col_inc(),
            "triangle_incurred",
            input.radio_button_inc()
        ))

    @reactive.Effect
    @reactive.event(input.load_button_eksp)
    def _():
        user_data.current_triangle.set("triangle_exposure")
        asyncio.create_task(load_data(
            input.file_input_eksp,
            input.sheet_select_eksp,
            input.start_row_eksp(),
            input.start_col_eksp(),
            input.end_row_eksp(),
            input.end_col_eksp(),
            "triangle_exposure",
            None
        ))

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

    def show_download_modal():
        ui.modal_show(
            ui.modal(
                ui.input_radio_buttons("file_format", "Format pliku:", {"csv": "CSV", "excel": "Excel"},
                                       selected="csv"),
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
    @render.download(
        filename=lambda: f"{user_data.current_triangle.get()}.{'csv' if input.file_format() == 'csv' else 'xlsx'}"
    )
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

    @output
    @render.ui
    def triangle_table_ui():
        data = get_current_data()
        if data is None:
            return ui.tags.p("Dane nie zostały wczytane")

        def format_number(x):
            if pd.isna(x):
                return "-"
            return f"{x:,.0f}".replace(",", "\u2005")

        html_table = data.to_html(
            classes='table table-striped table-hover',
            table_id="triangle-table-1",
            na_rep='-',
            formatters={col: format_number for col in data.columns[1:]},
            index=False,
            index_names=False
        )
        return ui.HTML(html_table)

    @reactive.Effect
    def update_ratios():
        data = get_current_data()
        if data is None:
            user_data.ratio_df_p.set(None)
            user_data.binary_df_p.set(None)
        else:
            ratio_df = calculate_ratios_vectorized(data)
            bin_df = create_binary_df(ratio_df)
            user_data.ratio_df_p.set(ratio_df)
            user_data.binary_df_p.set(bin_df)

            # Dodaj to: zapisujemy dev_all tylko raz!
            if user_data.dev_all_initial.get() is None:
                dev_all = calculator.calculate_dev(data, bin_df, ratio_df)
                user_data.dev_all_initial.set(dev_all)

    @output
    @render.ui
    def ratios_table_ui_p():
        ratio_df_p = user_data.ratio_df_p.get()
        if ratio_df_p is None:
            return ui.tags.p("Dane nie zostały wczytane (brak ratio).")

        html_table = ratio_df_p.to_html(
            classes='table table-striped table-hover',
            table_id="ratios-table-1",
            na_rep='-',
            index=False
        )
        highlight_script = f"highlight_default_cell({input.offset_distance()});"
        return ui.TagList(
            ui.HTML(html_table),
            ui.tags.script(highlight_script)
        )

    #
    #  Kluczowa poprawka: NIE odejmujemy już 1 od row ani col w Pythonie,
    #  bo zrobiliśmy to w JS (row-1 w kliku i row-1 w offset).
    #
    @reactive.Effect
    @reactive.event(input.clicked_cell_ratios_table_1)
    def update_clicked_cell_p():
        cell = input.clicked_cell_ratios_table_1()
        if not cell:
            return
        ratio_df_p = user_data.ratio_df_p.get()
        binary_df_p = user_data.binary_df_p.get()
        if ratio_df_p is None or binary_df_p is None:
            return

        row = cell["row"]  # NIE - 1
        col = cell["col"]  # NIE - 1
        highlighted = cell["highlighted"]

        if 0 <= row < binary_df_p.shape[0] and 0 <= col < binary_df_p.shape[1]:
            df_copy = binary_df_p.copy()  # aby Shiny "zauważył" zmianę
            df_copy.iat[row, col] = 0 if highlighted else 1
            user_data.binary_df_p.set(df_copy)

    @reactive.Effect
    @reactive.event(input.all_generated_cells_ratios_table_1)
    def update_all_generated_cells():
        data = input.all_generated_cells_ratios_table_1()
        cells = data.get("cells", []) if isinstance(data, dict) else data
        binary_df_p = user_data.binary_df_p.get()
        if binary_df_p is None:
            return

        df_copy = binary_df_p.copy()

        # Reset tabeli na same 1 jeśli brak komórek
        if not cells:
            df_copy.iloc[:, :] = 1
        else:
            df_copy.iloc[:, :] = 1
            for c in cells:
                row = c["row"]
                col = c["col"]
                highlighted = c["highlighted"]
                if 0 <= row < df_copy.shape[0] and 0 <= col < df_copy.shape[1]:
                    df_copy.iat[row, col] = 0 if highlighted else 1

        user_data.binary_df_p.set(df_copy)

    @reactive.Effect
    @reactive.event(input.offset_distance)
    async def update_highlight_default_cell():
        offset = input.offset_distance()
        await session.send_custom_message('highlight_cells', offset)

    @output
    @render.ui
    def binary_ratios_table_ui_p():
        binary_df_p = user_data.binary_df_p.get()
        if binary_df_p is None:
            return ui.tags.p("Brak binarnej tabeli (najpierw wczytaj dane).")

        html_table = binary_df_p.to_html(
            classes='table table-striped table-hover',
            table_id="binary-ratios-table-1",
            na_rep='NaN',
            float_format='{:.0f}'.format,
            index=False
        )
        return ui.HTML(html_table)

    @output
    @render.ui
    def table_dev_paid():
        # Pobierz aktualny trójkąt (np. Paid triangle)
        data = get_current_data()
        if data is None:
            return ui.tags.p("Brak danych do wyświetlenia Dev.")

        # Licz tabelę ratios (CL) oraz binarną (wagi pełne = same 1)
        ratio_df = calculator.calculate_ratios_vectorized(data)
        binary_df = calculator.create_binary_df(ratio_df)
        dev_values = calculator.calculate_dev(data, binary_df, ratio_df)
        # Budujemy tabelę HTML
        table_html = "<table class='table table-bordered table-sm'>"

        # Nagłówek
        table_html += "<thead><tr><th>Wybór</th>"
        for idx in range(1, len(dev_values) + 1):
            table_html += f"<th>{idx}</th>"
        table_html += "</tr></thead>"

        # Pierwszy wiersz z dev_all
        table_html += "<tbody><tr><td><strong>dev_all</strong></td>"
        for val in dev_values:
            formatted_val = f"{val:.4f}" if not np.isnan(val) else "-"
            table_html += f"<td>{formatted_val}</td>"
        table_html += "</tr></tbody>"

        table_html += "</table>"

        return ui.HTML(table_html)

    @reactive.Effect
    @reactive.event(input.calculate_button)
    def on_calculate_button_click():
        data = get_current_data()
        binary_df = user_data.binary_df_p.get()
        ratio_df = user_data.ratio_df_p.get()

        if data is None or binary_df is None or ratio_df is None:
            return

        print(pd.DataFrame(binary_df).to_string())

        # Obliczamy współczynniki na podstawie aktualnych wag
        dev_values = calculator.calculate_dev(data, binary_df, ratio_df)
        print(dev_values)

        vol = input.offset_distance()
        current_volume.set(vol)

        # Inkrementacja wersji dla danego volume
        volume_dict = volume_versions.get()
        version = volume_dict.get(vol, 0)
        volume_dict[vol] = version + 1
        volume_versions.set(volume_dict)

        # Budujemy nazwę etykiety
        label = f"volume={vol}" if version == 0 else f"volume={vol}:{version}"

        # Zapisujemy nowy zestaw wyników
        history = user_data.dev_selected_history.get()
        new_history = history + [(label, dev_values)]
        user_data.dev_selected_history.set(new_history)
        labels = [label for label, _ in user_data.dev_selected_history.get()]
        user_data.available_projections.set(["dev_all", "FINAL", "Factor curve"] + labels)
        print(f"Nowa historia: {new_history}")

    @output
    @render.ui
    def table_dev_history():
        precision = input.rounding_precision() or 4
        format_string = f"{{:.{precision}f}}"

        # ----- 1. Pobierz dane reaktywne -----
        history = user_data.dev_selected_history.get()  # Lista [(label, [val1, val2, ...]), ...]
        dev_all_initial = user_data.dev_all_initial.get()  # Lista wartości
        selected_final = selected_final_index.get()  # Np. "dev_all" albo string z liczbą
        current_finals = final_values.get()  # Lista wartości do wiersza FINAL

        # ----- 2. Przygotuj dane i oblicz liczbę kolumn -----
        history_data = history if history else []
        dev_all_data = dev_all_initial if dev_all_initial is not None else []

        max_len_hist = max((len(vals) for _, vals in history_data), default=0)
        max_len_all = len(dev_all_data)
        max_len_curr = len(current_finals)
        max_len = max(max_len_hist, max_len_all, max_len_curr)

        # ----- 3. Styl -----
        table_html = """
            <style>
            .final-highlight td,
            .selected-cell {
                background-color: #d7fbd7 !important;
                color: #000 !important;
            }
            .final-highlight td.final-override {
                background-color: transparent !important;
                color: inherit !important;
            }
            #dev-history-table td:hover {
                background-color: #f9f9f9;
            }
            </style>
        """

        # ----- 4. Budowa tabeli -----
        table_html += "<table id='dev-history-table' class='table table-bordered table-sm'>"
        table_html += "<thead><tr><th>Wybierz</th><th>Wariant</th>"
        for i in range(max_len):
            table_html += f"<th>{i + 1}</th>"
        table_html += "</tr></thead><tbody>"

        # ----- 5. Wiersz dev_all -----
        is_dev_all_final = (selected_final == "dev_all")
        row_class_dev_all = "final-highlight" if is_dev_all_final else ""
        checked_dev_all = "checked" if is_dev_all_final else ""

        table_html += f"<tr class='{row_class_dev_all}'>"
        table_html += f"<td><input type='radio' name='final_select' value='dev_all' {checked_dev_all}></td>"
        table_html += "<td><strong>dev_all</strong></td>"

        for col_idx in range(max_len):
            if col_idx < len(dev_all_data):
                val = dev_all_data[col_idx]
                formatted_val = format_string.format(val) if (
                        isinstance(val, (float, int)) and not np.isnan(val)) else "-"
            else:
                formatted_val = "-"
            table_html += f"<td class='data-cell' data-col='{col_idx}' data-value='{formatted_val}'>{formatted_val}</td>"
        table_html += "</tr>"

        # ----- 6. Wiersze historii -----
        for idx, (label, dev_vals) in enumerate(history_data):
            is_selected = (selected_final == str(idx))
            row_class = "final-highlight" if is_selected else ""
            checked_attr = "checked" if is_selected else ""

            table_html += f"<tr class='{row_class}'>"
            table_html += f"<td><input type='radio' name='final_select' value='{idx}' {checked_attr}></td>"
            table_html += f"<td>{label}</td>"

            for col_idx in range(max_len):
                if col_idx < len(dev_vals):
                    val = dev_vals[col_idx]
                    formatted_val = format_string.format(val) if (
                            isinstance(val, (float, int)) and not np.isnan(val)) else "-"
                else:
                    formatted_val = "-"
                table_html += (
                    f"<td class='data-cell' data-col='{col_idx}' data-value='{formatted_val}'>"
                    f"{formatted_val}</td>"
                )
            table_html += "</tr>"

        # ----- 7. Wiersz FINAL -----
        table_html += "<tr class='final-row'><td colspan='2'><strong>FINAL</strong></td>"

        for col_idx in range(max_len):
            if col_idx < len(current_finals):
                val = current_finals[col_idx]
                formatted_val = format_string.format(val) if isinstance(val, (float, int)) and not np.isnan(
                    val) else str(val)
            else:
                formatted_val = "-"
            table_html += f"<td class='final-data-cell' data-col='{col_idx}' data-value='{formatted_val}'>{formatted_val}</td>"

        table_html += "</tr>"
        table_html += "</tbody></table>"

        # ----- 8. Modal potwierdzenia zmiany FINAL -----
        table_html += """
            <div class="modal fade" id="confirmFinalChangeModal" tabindex="-1" role="dialog" 
                 aria-labelledby="confirmModalLabel" aria-hidden="true">
              <div class="modal-dialog" role="document">
                <div class="modal-content">
                  <div class="modal-header">
                    <h5 class="modal-title" id="confirmModalLabel">Potwierdzenie zmiany FINAL</h5>
                  </div>
                  <div class="modal-body">
                    Czy na pewno chcesz zmienić FINAL? Twoje dotychczasowe zmiany zostaną utracone.
                  </div>
                  <div class="modal-footer">
                    <button type="button" class="btn btn-secondary" data-dismiss="modal">Anuluj</button>
                    <button type="button" class="btn btn-primary" id="confirmFinalChange">Tak, zmień</button>
                  </div>
                </div>
              </div>
            </div>
        """

        # ----- 9. Skrypt wyboru FINAL -----
        table_html += """
            <script>
            var selectedFinalCandidate = null;
            var highlightedCellsHistory = []; // 🔥 zmieniona zmienna

            $(document).on('change', 'input[name="final_select"]', function() {
                selectedFinalCandidate = $(this).val();
                $('#confirmFinalChangeModal').modal('show');
                $('input[name="final_select"]').prop('checked', false);
            });

            $('#confirmFinalChange').on('click', function() {
                $('#confirmFinalChangeModal').modal('hide');
                if (selectedFinalCandidate !== null) {
                    $('input[name="final_select"][value="' + selectedFinalCandidate + '"]').prop('checked', true);
                    $('#dev-history-table tbody tr').removeClass('final-highlight');
                    $('#dev-history-table tbody tr td').removeClass('final-override');
                    $('#dev-history-table .data-cell').removeClass('selected-cell');

                    $('input[name="final_select"][value="' + selectedFinalCandidate + '"]')
                        .closest('tr')
                        .addClass('final-highlight');

                    Shiny.setInputValue("final_selected_index", selectedFinalCandidate, {priority: "event"});
                }
                selectedFinalCandidate = null;
            });
            </script>
        """

        # ----- 10. Skrypt kliknięcia w komórkę -----
        table_html += """
            <script>
            document.querySelectorAll('#dev-history-table .data-cell').forEach(function(cell) {
                cell.addEventListener('click', function() {
                    var colIndex = cell.getAttribute('data-col');
                    var clickedValue = cell.getAttribute('data-value');

                    // Usuń .selected-cell z wszystkich komórek w tej kolumnie
                    document.querySelectorAll('#dev-history-table .data-cell[data-col="' + colIndex + '"]')
                        .forEach(function(c) { c.classList.remove('selected-cell'); });

                    // Dodaj .selected-cell do klikniętej komórki
                    cell.classList.add('selected-cell');

                    // Final row — sprawdzamy wartość
                    var finalRow = document.querySelector('#dev-history-table tbody tr.final-highlight');
                    if (finalRow) {
                        var finalCell = finalRow.querySelector('td[data-col="' + colIndex + '"]');
                        if (finalCell) {
                            var finalValue = finalCell.getAttribute('data-value');
                            if (finalValue === clickedValue) {
                                finalCell.classList.remove('final-override');
                            } else {
                                finalCell.classList.add('final-override');
                            }
                        }
                    }

                    // Zaktualizuj wiersz FINAL
                    var finalRowData = document.querySelector('#dev-history-table tr.final-row');
                    var finalCells = finalRowData.querySelectorAll('td');
                    var targetIndex = parseInt(colIndex) + 1;
                    if (finalCells[targetIndex]) {
                        finalCells[targetIndex].textContent = clickedValue;
                        finalCells[targetIndex].setAttribute('data-value', clickedValue);
                    }

                    // Dodaj do historii podświetleń
                    highlightedCellsHistory.push({ col: parseInt(colIndex), value: clickedValue });

                    var finalRowValues = [];
                    document.querySelectorAll('#dev-history-table tr.final-row td.final-data-cell')
                        .forEach(function(td) {
                            var valStr = td.getAttribute('data-value') || td.textContent.trim();
                            var num = parseFloat(valStr);
                            finalRowValues.push(isNaN(num) ? null : num);
                        });

                    console.log("Wysyłam full_final_vector:", finalRowValues); // (dla debugowania)

                    Shiny.setInputValue("full_final_vector", finalRowValues, { priority: "event" });

                });
            });
            </script>
        """

        return ui.HTML(table_html)

    @output
    @render.ui
    def table_dev_final():
        history = user_data.dev_selected_history.get()
        selected_index = selected_final_index.get()

        table_html = "<table class='table table-bordered table-sm'>"
        table_html += "<thead><tr><th>FINAL</th>"

        # Pobieramy dane źródłowe
        if selected_index == "dev_all":
            dev_values = calculator.calculate_dev(
                get_current_data(),
                user_data.binary_df_p.get(),
                user_data.ratio_df_p.get()
            )
            label = "dev_all"
        else:
            selected_index = int(selected_index)
            if not history or selected_index >= len(history):
                return ui.tags.p("Brak danych do wyświetlenia FINAL.")
            label, dev_values = history[selected_index]

        for idx in range(1, len(dev_values) + 1):
            table_html += f"<th>{idx}</th>"
        table_html += "</tr></thead><tbody>"

        table_html += f"<tr><td>{label}</td>"
        for val in dev_values:
            formatted_val = f"{val:.4f}" if not np.isnan(val) else "-"
            table_html += f"<td>{formatted_val}</td>"
        table_html += "</tr></tbody></table>"

        return ui.HTML(table_html)

    @reactive.Effect
    @reactive.event(input.final_selected_index)
    def handle_final_selection_change():
        print(f"Zmieniono FINAL na: {input.final_selected_index()}")
        selected_final_index.set(input.final_selected_index())
        history = user_data.dev_selected_history.get()
        dev_all_data = user_data.dev_all_initial.get()

        if input.final_selected_index() == "dev_all":
            final_values.set(list(dev_all_data))
        else:
            idx = int(input.final_selected_index())
            if history and idx < len(history):
                _, dev_data = history[idx]
                final_values.set(list(dev_data))

    @reactive.Effect
    @reactive.event(input.cell_click_dev_history)
    def update_final_cell():
        cell = input.cell_click_dev_history()
        if not cell:
            return

        col = cell["col"] - 2  # <- WAŻNE! Odejmujesz 2, bo pierwsze kolumny to radio + etykieta
        value = cell["value"]

        try:
            value = float(value)
        except ValueError:
            return  # Nie liczba? Pomijamy.

        final_values_current = final_values.get()

        # Dodaj debug print
        print(f"Kliknięto w kolumnę: {col}, przypisujemy wartość: {value}")

        if 0 <= col < len(final_values_current):
            # Tworzymy kopię listy żeby reactive działał!
            final_copy = final_values_current.copy()
            final_copy[col] = value
            final_values.set(final_copy)

    @reactive.Effect
    @reactive.event(input.confirm_reset_volume)
    def reset_volume_history():
        print("Resetuję historię volume — potwierdzone!")

        user_data.dev_selected_history.set([])
        volume_versions.set({})
        selected_final_index.set("dev_all")

        dev_all = user_data.dev_all_initial.get()
        if dev_all is not None:
            final_values.set(list(dev_all))

    @output
    @render.download(filename=lambda: f"eksport_wynikow_{pd.Timestamp.now().strftime('%Y-%m-%d_%H-%M-%S')}.xlsx")
    def export_history_button():
        import pandas as pd
        from io import BytesIO
        from openpyxl.utils import get_column_letter
        from openpyxl.styles import Font, PatternFill

        # Pobierz dane dużej tabeli (Tabela CL)
        ratio_df_p = user_data.ratio_df_p.get()
        if ratio_df_p is None:
            ratio_df_p = pd.DataFrame()

        # Pobierz historię dev_selected_history
        history = user_data.dev_selected_history.get()
        final_vector_ui = input.final_vector_ui()  # UI Final vector!
        final_values_list = final_vector_ui or final_values.get()

        # Określ max długość kolumny
        max_len = max(
            len(final_values_list),
            max((len(values) for _, values in history), default=0)
        )

        history_data = {
            "Label": [],
            **{f"Value {i + 1}": [] for i in range(max_len)}
        }

        # Dodaj historię volume
        for label, values in history:
            history_data["Label"].append(label)
            for i in range(max_len):
                value = values[i] if i < len(values) else None
                history_data[f"Value {i + 1}"].append(value)

        # Dodaj FINAL — tylko z input z UI
        history_data["Label"].append("FINAL")
        for i in range(max_len):
            value = final_values_list[i] if i < len(final_values_list) else None
            history_data[f"Value {i + 1}"].append(value)

        history_df = pd.DataFrame(history_data)

        # Eksport do Excela z formatowaniem
        buffer = BytesIO()
        with pd.ExcelWriter(buffer, engine='openpyxl') as writer:
            ratio_df_p.to_excel(writer, index=False, sheet_name='Tabela CL')
            history_df.to_excel(writer, index=False, sheet_name='Historia')

            workbook = writer.book
            worksheet = writer.sheets['Historia']

            # Formatowanie nagłówków
            header_font = Font(bold=True)
            for col_idx, column_cells in enumerate(worksheet.iter_cols(min_row=1, max_row=1), 1):
                for cell in column_cells:
                    cell.font = header_font
                    col_letter = get_column_letter(col_idx)
                    max_length = max(
                        len(str(cell.value)) if cell.value else 0,
                        *(len(str(worksheet.cell(row=row_idx, column=col_idx).value)) for row_idx in
                          range(2, worksheet.max_row + 1))
                    )
                    worksheet.column_dimensions[col_letter].width = max_length + 2

            # Formatowanie FINAL na zielono
            final_row = worksheet.max_row
            fill = PatternFill(start_color="C6EFCE", end_color="C6EFCE", fill_type="solid")
            for cell in worksheet[final_row]:
                cell.fill = fill

        yield buffer.getvalue()

    @reactive.Effect
    @reactive.event(input.full_final_vector)
    async def _update_final_vector():  # <-- async !!!
        final_vector = input.full_final_vector()
        print(f"Nowy wektor FINAL: {final_vector}")
        final_vector_external.set(final_vector)  # <- TAK, do przekazania

    #######
    @output
    @render.ui
    def final_coefficients_display():
        current_finals = final_vector_external.get()

        if not current_finals:
            return ui.tags.p(" ⚠️ Krzywa nie została dopasowana”.",
                             {"style": "color: white; padding: 10px; font-weight: bold;"},)

        # ✅ Pełna inicjalizacja na start
        initial_selected = set()

        for col_idx, val in enumerate(current_finals):
            if isinstance(val, (float, int)) and not np.isnan(val):
                initial_selected.add((0, col_idx, val))

        user_data.new_cl_selected_cells.set(initial_selected)

        # Teraz budujesz tabelę jak miałeś
        table_html = """
            <style>
            .cell-final-cl-selected {
                background-color: #d8f6ce !important;
                color: black !important;
            }
            </style>
        """

        table_html += "<table id='final-cl-table' class='table table-bordered table-sm'>"
        table_html += "<thead><tr>"

        for idx in range(len(current_finals)):
            table_html += f"<th>{idx + 1}</th>"
        table_html += "</tr></thead><tbody><tr>"

        precision = input.rounding_precision() or 4
        format_string = f"{{:.{precision}f}}"

        for col_idx, val in enumerate(current_finals):
            formatted_val = format_string.format(val) if isinstance(val, (float, int)) and not np.isnan(val) else "-"
            table_html += (
                f"<td class='cell-final-cl-selected' "
                f"data-row='0' data-col='{col_idx}' data-value='{formatted_val}'>"
                f"{formatted_val}</td>"
            )
        table_html += "</tr></tbody></table>"

        table_html += """
            <script>
            $(document).on('click', '#final-cl-table td', function() {
                var row = parseInt($(this).attr('data-row'));
                var col = parseInt($(this).attr('data-col'));
                var valueText = $(this).attr('data-value');
                var value = parseFloat(valueText.replace(',', '.'));

                var selected = !$(this).hasClass('cell-final-cl-selected');
                if (selected) {
                    $(this).addClass('cell-final-cl-selected');
                } else {
                    $(this).removeClass('cell-final-cl-selected');
                }

                Shiny.setInputValue('final_cl_cell_click', {
                    row: row,
                    col: col,
                    value: isNaN(value) ? null : value,
                    selected: selected
                }, {priority: 'event'});
            });
            </script>
        """

        return ui.HTML(table_html)

    @reactive.Effect
    @reactive.event(input.final_cl_cell_click)
    def handle_final_cl_cell_click():
        cell = input.final_cl_cell_click()
        if not cell:
            return
        selected_cells = user_data.new_cl_selected_cells.get()
        cell_tuple = (cell["row"], cell["col"], cell["value"])
        new_selected_cells = set(selected_cells)
        if cell["selected"]:
            new_selected_cells.add(cell_tuple)
        else:
            new_selected_cells = {c for c in new_selected_cells if not (c[0] == cell["row"] and c[1] == cell["col"])}
        user_data.new_cl_selected_cells.set(new_selected_cells)
        sorted_cells = sorted(new_selected_cells, key=lambda x: x[1])  # Sortujemy po indeksie kolumny!
        indices = [col + 1 for (_, col, _) in sorted_cells]  # numer kolumny
        values = [value for (_, _, value) in sorted_cells]  # wartość współczynnika
        user_data.cl_selected_indices.set(indices)
        user_data.cl_selected_values.set(values)

        # Printujesz aktualny stan do konsoli
        print("✅ Indeksy kolumn:", indices)
        print("✅ Wartości współczynników:", values)

    @output
    @render.ui
    def curve_success_modal():
        if not user_data.show_curve_modal.get():
            return None

        return ui.modal(
            "✅ Krzywe zostały dopasowane!",
            "Parametry zostały zapisane i możesz teraz przejść do generowania wartości.",
            title="Sukces!",
            easy_close=True,
            footer=ui.modal_button("Zamknij")
        )

    import time

    @reactive.Effect
    @reactive.event(input.accept_CL)
    def fit_curve_parameters():
        indices = user_data.cl_selected_indices.get()
        values = user_data.cl_selected_values.get()

        if not indices or not values:
            ui.notification_show("⚠️ Brak danych do dopasowania.", duration=5, type="warning")
            return

        xs = np.array(indices, dtype=float)
        ys = np.array(values, dtype=float)

        curve_list = ["Exponential", "Weibull", "Power", "Inverse Power"]

        raw_results = calculator.parameters_curve_reservoir(xs=xs, ys=ys, lista_krzywych=curve_list)
        user_data.curve_fit_results.set(raw_results)

        print("✅ Parametry dopasowanych krzywych:")
        print(raw_results)

        ui.insert_ui(
            selector="body",
            where="beforeEnd",
            ui=ui.HTML("""
                <div id="big-message-container" style="
                    position: fixed;
                    top: 0;
                    left: 0;
                    width: 100%;
                    height: 100%;
                    display: flex;
                    align-items: center;
                    justify-content: center;
                    background-color: rgba(20, 20, 30, 0.7);
                    z-index: 9998;
                ">
                    <div id="big-message" style="
                        background: linear-gradient(135deg, #2d3e50, #1c2733);
                        color: #ccffcc;
                        padding: 36px 56px;
                        font-size: 24px;
                        font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
                        border-radius: 12px;
                        box-shadow: 0 12px 24px rgba(0,0,0,0.5);
                        text-align: center;
                        position: relative;
                        animation: fadeInZoom 0.4s ease-out;
                        border: 1px solid #33cc99;
                    ">
                        <button onclick="document.getElementById('big-message-container').remove()" style="
                            position: absolute;
                            top: 10px;
                            right: 12px;
                            background: none;
                            border: none;
                            font-size: 22px;
                            color: #66ffcc;
                            cursor: pointer;
                        ">✖</button>
                        ✅ Dopasowano parametry krzywych!
                    </div>
                </div>

                <style>
                    @keyframes fadeInZoom {
                        from {
                            transform: scale(0.8);
                            opacity: 0;
                        }
                        to {
                            transform: scale(1);
                            opacity: 1;
                        }
                    }
                </style>
            """)
        )

    @reactive.Effect
    @reactive.event(input.close_modal)
    def _():
        ui.modal_remove()

    @reactive.Effect
    @reactive.event(input.generate_curve_values)
    async def generate_curve_values():  # <-- dodane async
        await session.send_custom_message("showSpinner", None)

        parameters_curve = user_data.curve_fit_results.get()
        if parameters_curve is None:
            print("⚠️ Najpierw dopasuj krzywe!")
            await session.send_custom_message("hideSpinner", None)
            return

        obs_count = input.curve_obs_count() or 10
        x_vals = np.arange(1, obs_count + 1)

        list_curve = ["Exponential", "Weibull", "Power", "Inverse Power"]
        parameters_curve = user_data.curve_fit_results.get()
        simulation_results = calculator.sim_data_curve_rezerwy(x_vals, list_curve, parameters_curve)

        print("🎯 Wygenerowane wartości krzywych:")
        print(simulation_results)

        user_data.curve_simulation_results.set(simulation_results)

        ui.insert_ui(
            selector="body",
            where="beforeEnd",
            ui=ui.HTML("""
                <div id="big-message-container" style="
                    position: fixed;
                    top: 0;
                    left: 0;
                    width: 100%;
                    height: 100%;
                    display: flex;
                    align-items: center;
                    justify-content: center;
                    background-color: rgba(20, 20, 30, 0.7);
                    z-index: 9998;
                ">
                    <div id="big-message" style="
                        background: linear-gradient(135deg, #2d3e50, #1c2733);
                        color: #ccffcc;
                        padding: 36px 56px;
                        font-size: 24px;
                        font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
                        border-radius: 12px;
                        box-shadow: 0 12px 24px rgba(0,0,0,0.5);
                        text-align: center;
                        position: relative;
                        animation: fadeInZoom 0.4s ease-out;
                        border: 1px solid #33cc99;
                    ">
                        <button onclick="document.getElementById('big-message-container').remove()" style="
                            position: absolute;
                            top: 10px;
                            right: 12px;
                            background: none;
                            border: none;
                            font-size: 22px;
                            color: #66ffcc;
                            cursor: pointer;
                        ">✖</button>
                        ✅ Współczynniki wygenerowane z dopasowanych krzywych!
                    </div>
                </div>

                <style>
                    @keyframes fadeInZoom {
                        from {
                            transform: scale(0.8);
                            opacity: 0;
                        }
                        to {
                            transform: scale(1);
                            opacity: 1;
                        }
                    }
                </style>
            """)
        )

        await session.send_custom_message("hideSpinner", None)

    @output
    @render.ui
    def generated_curve_values_table():
        df = user_data.curve_simulation_results.get()
        if df is None or df.empty:
            return ui.tags.p(" ⚠️ Brak wygenerowanych współczynników. Kliknij „Losuj współczynniki”.",
                             {"style": "color: white; padding: 10px; font-weight: bold;"},)

        styled_html = df.to_html(
            classes="table table-bordered table-sm table-striped text-center",
            float_format="%.10f"
        )

        return ui.HTML(f"""
            <div style="
                max-height: 260px;
                overflow-x: auto;
                overflow-y: auto;
                border: 1px solid #333;
                padding: 6px;
                margin-top: 5px;
            ">
                {styled_html}
            </div>
        """)


###########r kwadrat

    def change_ind_to_col(df_data, name_col):
        if (name_col in df_data.columns.to_list()):
            df_data = df_data.copy()
        else:
            df_data.insert(0, name_col, df_data.index)
        return (df_data)

    @reactive.Calc
    def r2_df_freq_lc_interaktywny():
        indices = user_data.cl_selected_indices.get()
        values = user_data.cl_selected_values.get()

        if not indices or not values:
            return None

        xs = np.array(indices, dtype=float)
        ys = np.array(values, dtype=float)

        parameters_curve = user_data.curve_fit_results.get()

        # 💥 Dodaj to:
        if parameters_curve is None:
            return None

        list_curve = ['Exponential', 'Weibull', 'Power', 'Inverse Power']
        f_curves_graph_real_choose = calculator.sim_data_curve_rezerwy(ys, list_curve, parameters_curve)
        r2_curves_df = calculator.r2_curves(f_curves_graph_real_choose, ys)

        return r2_curves_df

    @output
    @render.ui
    def r2_cl_paid():
        df = r2_df_freq_lc_interaktywny()
        if df is None:
            return ui.div(
                {"style": "color: orange; padding: 10px; font-weight: bold;"},
                "⚠️ Jeszcze nie dopasowano krzywych."
            )
        # Wracamy do komponentu data_frame
        return ui.output_data_frame("r2_cl_paid_table")

    @output
    @render.data_frame
    def r2_cl_paid_table():
        df = r2_df_freq_lc_interaktywny()
        r2_freq_lc = change_ind_to_col(df, name_col='Krzywa')
        df_clean = r2_freq_lc.T
        r2_freq_lc_reset = df_clean.reset_index().rename(columns={'index': 'Krzywa'})

        # Jeśli pierwsza komórka jest pusta — zwróć pustą tabelę
        if r2_freq_lc_reset.empty or pd.isna(r2_freq_lc_reset.iloc[0, 0]) or r2_freq_lc_reset.iloc[0, 0] == '':
            return pd.DataFrame({"Krzywa": [], "Błąd": []})

        # Tworzymy końcową tabelę
        return render.DataGrid(
            r2_freq_lc_reset,
            width="25%",
            height="auto",
            summary=False,
        )

    @output
    @render.plot
    def plot_curve_vs_final():
        final = final_vector_external.get()
        curves_df = user_data.curve_simulation_results.get()

        if not final or curves_df is None or curves_df.empty:
            return

        x_vals = list(range(1, len(final) + 1))
        fig, ax = plt.subplots(figsize=(10, 5))

        # Rysuj FINAL
        ax.plot(x_vals, final, label="FINAL", color="limegreen", linewidth=3, marker="o")
        for i, y in enumerate(final):
            ax.text(x_vals[i], y + 0.1, f"{y:.2f}", ha='center', fontsize=9, color='green')

        # Rysuj każdą krzywą
        for curve_name, row in curves_df.iterrows():
            y_vals = row.values[:len(final)]
            ax.plot(x_vals, y_vals, label=curve_name, linestyle='--', alpha=0.8)

        ax.set_title("Porównanie FINAL i krzywych dopasowanych")
        ax.set_xlabel("dp (Development period)")
        ax.set_ylabel("Wartość współczynnika")
        ax.legend()
        ax.grid(True)
        return fig
###########################################

    @output
    @render.ui
    def Final_Factor():
        final_factors = final_vector_external.get()
        if not final_factors:
            return ui.tags.p("⚠️ Brak współczynników FINAL.")

        fixed_n = input.num_of_first_factor() or 0
        total_len = len(final_factors)

        styled_html = """
            <style>
            .fixed-real {
                background-color: #b9fbc0 !important;
                color: #000 !important;
            }
            </style>
            <table class='table table-bordered table-sm table-striped text-center'>
            <thead><tr><th>Final</th>"""
        for i in range(total_len):
            styled_html += f"<th>{i + 1}</th>"
        styled_html += "</tr></thead><tbody><tr><td><strong>Final</strong></td>"

        for idx, val in enumerate(final_factors):
            td_class = "fixed-real" if idx < fixed_n else ""
            styled_html += f"<td class='{td_class}'>{val:.10f}</td>"

        styled_html += "</tr></tbody></table>"

        return ui.HTML(f"""
            <div style="
                max-height: 200px;
                overflow-x: auto;
                overflow-y: auto;
                border: 1px solid #333;
                padding: 6px;
                margin-bottom: 10px;
            ">
                {styled_html}
            </div>
        """)

    @output
    @render.ui
    def Factors_curve():
        df = user_data.curve_simulation_results.get()
        if df is None or df.empty:
            return ui.tags.p(" ⚠️ Brak wygenerowanych współczynników. Kliknij „Losuj współczynniki”.",
                             {"style": "color: white; padding: 10px; font-weight: bold;"})

        tail_vector = user_data.selected_tail_vector.get() or []
        tail_vector_rounded = [round(val, 10) if val is not None else None for val in tail_vector]
        fixed_n = input.num_of_first_factor() or 0

        styled_html = """
            <style>
            .selected-cell {
                background-color: #d7fbd7 !important;
                color: #000 !important;
            }
            .final-fixed {
                background-color: #eeeeee !important;
                color: #888 !important;
                pointer-events: none;
            }
            </style>
            <table class='table table-bordered table-sm table-striped text-center'>
            <thead><tr><th></th><th>Krzywa</th>"""
        for i in range(1, len(df.columns) + 1):
            styled_html += f"<th>dp: {i}</th>"
        styled_html += "</tr></thead><tbody>"

        for curve_name, row in df.iterrows():
            styled_html += f"<tr>"
            styled_html += f"<td><input type='radio' name='curve_select' value='{curve_name}'></td>"
            styled_html += f"<td><b>{curve_name}</b></td>"

            for col_idx, val in enumerate(row.values):
                val_rounded = round(val, 10)
                cell_class = "curve-cell"
                style_inline = ""

                if col_idx < fixed_n:
                    cell_class += " final-fixed"
                elif col_idx < len(tail_vector_rounded) and tail_vector_rounded[col_idx] == val_rounded:
                    cell_class += " selected-cell"

                styled_html += (
                    f"<td class='{cell_class}' style='{style_inline}' "
                    f"data-row='{curve_name}' data-col='{col_idx}' data-value='{val}'>"
                    f"{val:.10f}</td>"
                )
            styled_html += "</tr>"
        styled_html += "</tbody></table>"

        # --- Skrypt ---
        styled_html += """
            <script>
            let lastClicked = null;

            $(document).on('click', '.curve-cell', function(e) {
                e.stopPropagation();

                const curveName = $(this).data('row');
                const colIndex = $(this).data('col');
                const value = parseFloat($(this).data('value'));

                const payload = {
                    curve: curveName,
                    col: colIndex,
                    value: value,
                    type: "cell"
                };

                if (JSON.stringify(payload) === JSON.stringify(lastClicked)) {
                    return;
                }
                lastClicked = payload;

                Shiny.setInputValue("clicked_curve_value", payload, { priority: "event" });
            });

            $(document).on('change', 'input[name="curve_select"]', function() {
                const curveName = $(this).val();
                Shiny.setInputValue("clicked_curve_row", curveName, { priority: "event" });
            });
            </script>
        """

        return ui.HTML(f"""
            <div style="
                max-height: 260px;
                overflow-x: auto;
                overflow-y: auto;
                border: 1px solid #333;
                padding: 6px;
                margin-top: 5px;
            ">
                {styled_html}
            </div>
        """)

    @reactive.Effect
    @reactive.event(input.clicked_curve_row)
    def handle_curve_row_click():
        curve = input.clicked_curve_row()
        all_curves = user_data.curve_simulation_results.get()
        final_vec = final_vector_external.get()

        if curve not in all_curves.index or not final_vec:
            print("⛔ Nie znaleziono danych.")
            return

        num_real = input.num_of_first_factor() or 0
        full_curve = list(all_curves.loc[curve])
        total_len = len(final_vec)

        from_final = list(final_vec[:num_real])  # <- TUTAJ! rzutowanie na listę
        from_curve = full_curve[num_real:total_len]

        combined = from_final + from_curve

        while len(combined) < total_len:
            combined.append(None)

        user_data.selected_tail_vector.set(combined)
        print(f"✅ Factor curve = FINAL[0:{num_real}] + KRZYWA[{num_real}:]")

    @reactive.Effect
    @reactive.event(input.num_of_first_factor, input.clicked_curve_row, input.wyb_krzywa_ogona)
    def update_tail_curve_vector():
        final_vec = final_vector_external.get()
        curves_df = user_data.curve_simulation_results.get()
        num_final = input.num_of_first_factor() or 0

        krzywa = input.clicked_curve_row() or input.wyb_krzywa_ogona()

        if not final_vec or curves_df is None or krzywa not in curves_df.index:
            user_data.selected_tail_vector.set([])
            print("⚠️ Brak danych krzywej lub błędny wybór.")
            return

        curve_row = list(curves_df.loc[krzywa])
        total_len = max(len(final_vec), len(curve_row))

        # 🧠 Składamy: final + krzywa
        combined = list(final_vec[:num_final]) + curve_row[num_final:]

        # 🔧 Uzupełnij None jeśli czegoś brakuje
        while len(combined) < total_len:
            combined.append(None)

        user_data.selected_tail_vector.set(combined)
        print(f"✅ Factor curve: final[:{num_final}] + krzywa[{num_final}:] (łącznie: {len(combined)} wartości)")

    @reactive.Effect
    @reactive.event(input.clicked_curve_value)
    def update_tail_vector():
        clicked = input.clicked_curve_value()
        if not clicked:
            return

        col = int(clicked["col"])
        value = float(clicked["value"])
        num_real = input.num_of_first_factor() or 0
        total_len = len(final_vector_external.get())

        # 🧠 Upewnij się, że ogon ma odpowiednią długość
        tail_len = total_len - num_real
        current = user_data.selected_tail_vector.get() or []
        new_tail = list(current)

        # Jeśli ogon jest za krótki – uzupełnij None
        while len(new_tail) < total_len:
            new_tail.append(None)

        idx_in_tail = col
        if idx_in_tail < num_real:
            print("⛔ To pole pochodzi z FINAL i nie może być nadpisane.")
            return

        # Właściwy indeks w krzywej
        idx_in_tail = col
        new_tail[idx_in_tail] = value

        user_data.selected_tail_vector.set(new_tail)
        print(f"📌 Tail updated @dp:{col + 1} -> {value}")

    @reactive.Effect
    @reactive.event(input.num_of_first_factor, input.wyb_krzywa_ogona)
    def auto_update_factor_curve():
        final_vec = final_vector_external.get()
        curve_df = user_data.curve_simulation_results.get()

        if not final_vec or curve_df is None:
            return

        selected_curve = input.wyb_krzywa_ogona()
        num_fixed = int(input.num_of_first_factor() or 0)

        if selected_curve not in curve_df.index:
            return

        full_curve = list(curve_df.loc[selected_curve])
        total_len = len(final_vec)

        final_part = list(final_vec[:num_fixed])
        curve_part = full_curve[num_fixed:total_len]

        combined = final_part + curve_part

        # Uzupełnienie braków None, jeśli długości się nie zgadzają
        combined = combined[:total_len]
        while len(combined) < total_len:
            combined.append(None)

        user_data.selected_tail_vector.set(combined)

    @output
    @render.ui
    def Tail_Factor():
        combined = user_data.selected_tail_vector.get()
        final = final_vector_external.get()
        if not combined or not final:
            return ui.tags.p("⚠️ Brak wybranych danych.")

        # 🧠 Używamy dłuższej z list
        total_len = max(len(combined), len(final))

        # 🔧 Zabezpiecz długość combined
        combined = combined[:total_len]
        while len(combined) < total_len:
            combined.append(None)

        df = pd.DataFrame([combined], index=["Factor curve"])
        df.columns = [f"{i + 1}" for i in range(total_len)]

        html = df.to_html(
            classes="table table-bordered table-sm table-striped text-center",
            float_format="%.10f"
        )

        return ui.HTML(f"""
            <div style="overflow-x: auto;">
                {html}
            </div>
        """)

    @output
    @render.download(filename=lambda: f"eksport_FINAL_i_krzywe_{pd.Timestamp.now().strftime('%Y-%m-%d_%H-%M-%S')}.xlsx")
    def export_final_vector():
        import pandas as pd
        from io import BytesIO
        from openpyxl.utils import get_column_letter
        from openpyxl.styles import Font, PatternFill

        final = user_data.final_factor_vector.get() or []
        tail = user_data.selected_tail_vector.get() or []
        curves_df = user_data.curve_simulation_results.get() or pd.DataFrame()

        df_final = pd.DataFrame([final], index=["FINAL"]).T
        df_final.columns = ["Współczynnik"]
        df_final.index.name = "dp"

        df_tail = pd.DataFrame([tail], index=["Factor curve"]).T
        df_tail.columns = ["Współczynnik"]
        df_tail.index.name = "dp"

        buffer = BytesIO()
        with pd.ExcelWriter(buffer, engine='openpyxl') as writer:
            df_final.to_excel(writer, sheet_name="FINAL_Vector")
            df_tail.to_excel(writer, sheet_name="Factor_Curve")

            if not curves_df.empty:
                curves_df.T.to_excel(writer, sheet_name="Generated_Curves", index_label="dp")

            # Formatowanie
            workbook = writer.book
            ws_final = writer.sheets["FINAL_Vector"]
            ws_tail = writer.sheets["Factor_Curve"]

            # Styl nagłówków
            bold_font = Font(bold=True)
            for ws in [ws_final, ws_tail]:
                for col_idx, column_cells in enumerate(ws.iter_cols(min_row=1, max_row=1), 1):
                    for cell in column_cells:
                        cell.font = bold_font
                        col_letter = get_column_letter(col_idx)
                        max_length = max(
                            len(str(cell.value)) if cell.value else 0,
                            *(len(str(ws.cell(row=row_idx, column=col_idx).value)) for row_idx in
                              range(2, ws.max_row + 1))
                        )
                        ws.column_dimensions[col_letter].width = max_length + 2

            # Zielone tło dla FINAL_Vector
            green_fill = PatternFill(start_color="C6EFCE", end_color="C6EFCE", fill_type="solid")
            for cell in ws_final.iter_rows(min_row=2, max_row=ws_final.max_row, min_col=2, max_col=2):
                for c in cell:
                    c.fill = green_fill

        yield buffer.getvalue()

    @output
    @render.ui
    def triangle_projection_table():
        triangle = get_current_data()
        final = final_vector_external.get()

        if triangle is None or not final:
            return ui.tags.p("⚠️ Brak danych wejściowych lub FINAL.")

        # Testowo od 5 kolumny
        start_index = 5

        projected_df = calculator.triangle_forward(triangle, final, start_index)

        html_table = projected_df.to_html(
            classes="table table-bordered table-sm table-striped text-center",
            float_format="%.2f",
            na_rep="–"
        )

        return ui.HTML(f"""
            <div style="overflow-x: auto; max-height: 400px;">
                {html_table}
            </div>
        """)

    @output
    @render.ui
    def triangle_projection_volume():
        req(input.run_projection())
        selected_labels = input.projection_selection()

        df_triangle = get_current_data()
        if df_triangle is None:
            return ui.tags.p("⚠️ Brak danych trójkąta.")

        df_triangle = df_triangle.copy()
        df_triangle["AY"] = df_triangle.index
        result_df = pd.DataFrame({"AY": df_triangle["AY"]})

        # Mapa dostępnych współczynników
        factor_map = {
            "dev_all": user_data.dev_all_initial.get(),
            "FINAL": final_values.get(),
            "Factor curve": user_data.selected_tail_vector.get(),
        }

        for label, values in user_data.dev_selected_history.get():
            factor_map[label] = values

        # Dodaj projekcje
        for label in selected_labels:
            f = factor_map.get(label)
            if f is not None and len(f) > 0:
                df_proj = calculator.triangle_forward(df_triangle.drop(columns="AY"), f, 1)
                result_df[label] = df_proj.iloc[:, -1]

        if result_df.shape[1] == 1:
            return ui.tags.p("⚠️ Nie wybrano żadnych współczynników do projekcji.")

        # Dodaj SUMĘ
        suma_row = {"AY": "SUMA"}
        for col in result_df.columns[1:]:
            suma_row[col] = round(result_df[col].sum(), 2)
        result_df = pd.concat([result_df, pd.DataFrame([suma_row])], ignore_index=True)

        html = result_df.to_html(
            classes="table table-bordered table-sm table-striped text-center",
            index=False,
            float_format="%.6f"
        )

        return ui.TagList(
            ui.tags.h5("📈 Projekcja ostatniej kolumny wg współczynników"),
            ui.HTML(html)
        )

    @render.ui
    def projection_selection():
        return ui.input_checkbox_group(
            id="projection_selection",
            label="Wybierz współczynniki do projekcji:",
            choices=user_data.available_projections.get(),
            selected=["dev_all"]
        )


app = App(app_ui, server)

if __name__ == "__main__":
    run_app(app, port=8003)
