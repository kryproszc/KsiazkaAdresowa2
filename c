from shiny import App, ui, render, reactive, run_app
import shinyswatch
import pandas as pd
from htmltools import HTML
import numbers
import os
from metody_jednoroczne_copy import YearHorizont
import numpy as np
import pandas as pd
from openpyxl import load_workbook

yh = YearHorizont()

modal_html = reactive.Value(HTML("Brak danych"))
# Na sztywno dane testowe
data = {
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
df = pd.DataFrame(data)

def load_excel_data(folder_path, file_name, sheet_name, start_row, start_col, end_row, end_col):
    # Zmiana ukośników z \ na / w ścieżce do folderu
    folder_path = folder_path.replace("\\", "/")
    file_path = os.path.join(folder_path, file_name)

    # Nowa walidacja
    from openpyxl import load_workbook

    if not os.path.isfile(file_path):
        raise FileNotFoundError(f"Plik '{file_path}' nie istnieje.")

    workbook = load_workbook(file_path)
    if sheet_name not in workbook.sheetnames:
        raise ValueError(f"W pliku '{file_name}' nie znaleziono arkusza o nazwie '{sheet_name}'.")

    # Wczytanie danych
    data = pd.read_excel(file_path,
                         sheet_name=sheet_name,
                         header=None,
                         engine="openpyxl",
                         skiprows=start_row - 1,
                         usecols=range(start_col - 1, end_col),
                         nrows=end_row - start_row + 1)
    return data


# UI
app_ui = ui.page_fluid(ui.page_navbar(
    shinyswatch.theme.superhero(),
    ui.nav_panel("REZERWY TESTY", ui.output_image("image")),
    ui.nav_panel("Wprowadź dane",
        ui.row(
            ui.column(
                4,
                "Wprowadź trójkąt Paid",
                ui.input_text("folder_path", "Ścieżka do folderu:", value="C:/Users/Documents"),
                ui.input_text("file_name", "Nazwa pliku Excel:", value="plik.xlsx"),
                ui.input_text("sheet_name", "Nazwa arkusza:", value="Arkusz1"),
                ui.input_numeric("start_row", "Wiersz początkowy:", value=2),
                ui.input_numeric("start_col", "Kolumna początkowa:", value=2),
                ui.input_numeric("end_row", "Wiersz końcowy:", value=5),
                ui.input_numeric("end_col", "Kolumna końcowa:", value=4),
                ui.input_radio_buttons(
                    "radio_button_paid",
                    "Wartości trójkata",
                    {
                        "Skumulowane": "Skumulowane",
                        "Inkrementalne": "Inkrementalne"
                    }),
                ui.input_action_button("load_button", "Wczytaj dane"),
                ui.input_action_button("check_button_paid", "Sprawdź dane"),
                ui.input_action_button("download_button_paid", "Pobierz dane")

            ),
            ui.column(
                4,
                "Wprowadź trójkąt Incurred",
                ui.input_text("folder_path_inc", "Ścieżka do folderu:", value="C:/Users/Documents"),
                ui.input_text("file_name_inc", "Nazwa pliku Excel:", value="plik.xlsx"),
                ui.input_text("sheet_name_inc", "Nazwa arkusza:", value="Arkusz1"),
                ui.input_numeric("start_row_inc", "Wiersz początkowy:", value=2),
                ui.input_numeric("start_col_inc", "Kolumna początkowa:", value=2),
                ui.input_numeric("end_row_inc", "Wiersz końcowy:", value=5),
                ui.input_numeric("end_col_inc", "Kolumna końcowa:", value=4),
                ui.input_radio_buttons(
                    "radio_button_inc",
                    "Wartości trójkata",
                    {
                        "Skumulowane": "Skumulowane",
                        "Inkrementalne": "Inkrementalne"
                    }),
                ui.input_action_button("load_button_inc", "Wczytaj dane"),
                ui.input_action_button("check_button_inc", "Sprawdź dane"),
                ui.input_action_button("download_button_inc", "Pobierz dane")

            ),
            ui.column(
                4,
                "Wprowadź trójkąt ekspozycję",
                ui.input_text("folder_path_eksp", "Ścieżka do folderu:", value="C:/Users/Documents"),
                ui.input_text("file_name_eksp", "Nazwa pliku Excel:", value="plik.xlsx"),
                ui.input_text("sheet_name_eksp", "Nazwa arkusza:", value="Arkusz1"),
                ui.input_numeric("start_row_eksp", "Wiersz początkowy:", value=2),
                ui.input_numeric("start_col_eksp", "Kolumna początkowa:", value=2),
                ui.input_numeric("end_row_eksp", "Wiersz końcowy:", value=5),
                ui.input_numeric("end_col_eksp", "Kolumna końcowa:", value=4),
                ui.input_action_button("load_button_eksp", "Wczytaj dane"),
                ui.input_action_button("check_button_eksp", "Sprawdź dane"),
                ui.input_action_button("download_button_eksp", "Pobierz dane")

            ),
        ),
    ),
    ui.nav_panel("Metody deterministyczne",
        ui.navset_tab(
            ui.nav_panel("Jednoroczne",
                ui.output_table("triangle_output")
            )
        )
    )
))

# Server
def server(input, output, session):
    # Reactive storage

    # --- PAID ---
    @reactive.Effect
    @reactive.event(input.load_button)
    def load_data_paid():
        try:
            folder_path = input.folder_path().replace("\\", "/")
            file_name = input.file_name()
            sheet_name = input.sheet_name()
            start_row = input.start_row()
            start_col = input.start_col()
            end_row = input.end_row()
            end_col = input.end_col()
            file_path = f"{folder_path}/{file_name}"
            if not os.path.isfile(file_path):
                show_info_modal("W podanej ścieżce nie istnieje plik o takiej nazwie.", error=True)
                return

            data = load_excel_data(folder_path, file_name, sheet_name, start_row + 1, start_col, end_row, end_col)

            # Skumulowanie jeśli trzeba
            if input.radio_button_paid() == "Inkrementalne":
                data.iloc[:, 1:] = data.iloc[:, 1:].cumsum(axis=1)

           # data = yh.change_value_less_diagonal(data, np.nan)
            lista = reactive_data_paid_list.get()
            lista.append(data)
            reactive_data_paid_list.set(lista)
            show_info_modal("Dane Paid zostały wczytane.")
        except Exception as e:
            show_info_modal(f"Wystąpił błąd: {e}", error=True)

    # --- INCURRED ---
    @reactive.Effect
    @reactive.event(input.load_button_inc)
    def load_data_incurred():
        try:
            folder_path = input.folder_path_inc().replace("\\", "/")
            file_name = input.file_name_inc()
            sheet_name = input.sheet_name_inc()
            start_row = input.start_row_inc()
            start_col = input.start_col_inc()
            end_row = input.end_row_inc()
            end_col = input.end_col_inc()

            file_path = f"{folder_path}/{file_name}"
            if not os.path.isfile(file_path):
                show_info_modal("W podanej ścieżce nie istnieje plik o takiej nazwie.", error=True)
                return

            data = load_excel_data(folder_path, file_name, sheet_name, start_row + 1, start_col, end_row, end_col)

            if input.radio_button_inc() == "Inkrementalne":
                data.iloc[:, 1:] = data.iloc[:, 1:].cumsum(axis=1)

            #data = yh.change_value_less_diagonal(data, np.nan)
            lista = reactive_data_incurred_list.get()
            lista.append(data)
            reactive_data_incurred_list.set(lista)
            show_info_modal("Dane Incurred zostały wczytane.")
        except Exception as e:
            show_info_modal(f"Wystąpił błąd: {e}", error=True)

    # --- EXPOSURE ---
    @reactive.Effect
    @reactive.event(input.load_button_eksp)
    def load_data_exposure():
        try:
            folder_path = input.folder_path_eksp().replace("\\", "/")
            file_name = input.file_name_eksp()
            sheet_name = input.sheet_name_eksp()
            start_row = input.start_row_eksp()
            start_col = input.start_col_eksp()
            end_row = input.end_row_eksp()
            end_col = input.end_col_eksp()

            file_path = f"{folder_path}/{file_name}"
            if not os.path.isfile(file_path):
                show_info_modal("W podanej ścieżce nie istnieje plik o takiej nazwie.", error=True)
                return

            data = load_excel_data(folder_path, file_name, sheet_name, start_row + 1, start_col, end_row, end_col)

           # data = yh.change_value_less_diagonal(data, np.nan)
            lista = reactive_data_exposure_list.get()
            lista.append(data)
            reactive_data_exposure_list.set(lista)
            show_info_modal("Dane Ekspozycja zostały wczytane.")
        except Exception as e:
            show_info_modal(f"Wystąpił błąd: {e}", error=True)

    reactive_data_paid_list = reactive.Value([])
    reactive_data_incurred_list = reactive.Value([])
    reactive_data_exposure_list = reactive.Value([])
    file_name = reactive.Value("dane")
    current_triangle = reactive.Value("triangle")  # domyślnie

    # Modal data holder
    modal_data = reactive.Value(pd.DataFrame())

    # Modal render table
    @output
    @render.ui
    def modal_table():
        return modal_html.get()

    # Modal show function
    def show_modal(title):
        from htmltools import TagList
        from shiny import ui

        ui.modal_show(
            ui.modal(
                TagList(
                    ui.tags.style("""
                        /* Powiększamy całe okno modala */
                        .modal-dialog {
                            max-width: 90%;
                            width: 90%;
                        }

                        /* Treść modala: maksymalna wysokość i scroll pionowy przy dużej liczbie wierszy */
                        .modal-body {
                            max-height: 80vh;
                            overflow-y: auto;
                        }

                        /* Stylowanie tabeli wewnątrz modala */
                        .shiny-table-output table {
                            width: 100%;
                            white-space: nowrap;
                        }

                        /* Scroll poziomy, ale pojawia się tylko gdy potrzeba */
                        .shiny-table-output {
                            overflow-x: auto;
                        }
                    """),
                    ui.output_ui("modal_table"),
                    ui.modal_button("Zamknij")
                ),
                title=title,
                easy_close=True,
                footer=None,
                size="l"
            )
        )

    def show_info_modal(message, error=False):
        from shiny import ui
        from htmltools import TagList

        # Ikona: zielona lub czerwona
        icon = "✅" if not error else "❌"

        # Styl czcionki dopasowany do motywu
        color = "#f8f9fa" if not error else "#f8f9fa"  # jasna jak przycisk „Zamknij”
        font_family = "'Segoe UI', Tahoma, Geneva, Verdana, sans-serif"
        font_size = "15px"
        font_weight = "normal"  # Możesz dać "bold" dla wyrazistości

        ui.modal_show(
            ui.modal(
                TagList(
                    ui.p(HTML(
                        f"<span style='color: {color}; font-family: {font_family}; "
                        f"font-size: {font_size}; font-weight: {font_weight};'>"
                        f"{icon} {message}</span>"
                    )),
                    ui.modal_button("Zamknij")
                ),
                title="Informacja",
                easy_close=True,
                footer=None
            )
        )

    def validate_triangle(dataframe):
        errors = set()
        rows, cols = dataframe.shape

        for col_idx in range(1, cols):
            max_row = rows - (col_idx - 1)

            for row_idx in range(rows):
                value = dataframe.iloc[row_idx, col_idx]

                if row_idx < max_row:
                    if pd.isna(value) or not isinstance(value, numbers.Number):
                        errors.add((row_idx, col_idx))
        return errors

    def dataframe_to_html(dataframe, errors):
        html = "<table class='table table-bordered'>"
        html += "<thead><tr>"
        for col in dataframe.columns:
            html += f"<th>{col}</th>"
        html += "</tr></thead>"

        html += "<tbody>"
        for row_idx in range(dataframe.shape[0]):
            html += "<tr>"
            for col_idx in range(dataframe.shape[1]):
                value = dataframe.iloc[row_idx, col_idx]
                style = ""
                if (row_idx, col_idx) in errors:
                    style = "style='color: red; font-weight: bold;'"
                html += f"<td {style}>{'' if pd.isna(value) else value}</td>"
            html += "</tr>"
        html += "</tbody></table>"
        return HTML(html)

    def get_current_data():
        if current_triangle.get() == "triangle_paid":
            lista = reactive_data_paid_list.get()
            return lista[-1] if lista else None
        elif current_triangle.get() == "triangle_incurred":
            lista = reactive_data_incurred_list.get()
            return lista[-1] if lista else None
        elif current_triangle.get() == "triangle_exposure":
            lista = reactive_data_exposure_list.get()
            return lista[-1] if lista else None
        else:
            return None

    def check_data(title):
        data = get_current_data()
        if data is None:
            modal_html.set(HTML("<p style='color: red;'>Brak danych do sprawdzenia.</p>"))
        else:
            errors = validate_triangle(data)
            table_html = dataframe_to_html(data, errors)
            modal_html.set(table_html)

        show_modal("Podgląd wczytanego trójkąta.")  # <--- ZMIANA

    # Sprawdź dane Paid
    @reactive.Effect
    @reactive.event(input.check_button_paid)
    def _():
        current_triangle.set("triangle_paid")
        check_data("Dane Paid")

    @reactive.Effect
    @reactive.event(input.check_button_inc)
    def _():
        current_triangle.set("triangle_incurred")
        check_data("Dane Incurred")

    @reactive.Effect
    @reactive.event(input.check_button_eksp)
    def _():
        current_triangle.set("triangle_exposure")
        check_data("Dane Ekspozycja")

    @reactive.Effect
    @reactive.event(input.download_button_paid)
    def _():
        current_triangle.set("triangle_paid")
        show_download_modal()

    @reactive.Effect
    @reactive.event(input.download_button_inc)
    def _():
        current_triangle.set("triangle_incurred")
        show_download_modal()

    @reactive.Effect
    @reactive.event(input.download_button_eksp)
    def _():
        current_triangle.set("triangle_exposure")
        show_download_modal()

    # Wyświetlanie w zakładce "Metody deterministyczne"
    @output
    @render.table
    def triangle_output():
        data = get_current_data()
        if data is not None:
            return data
        else:
            return pd.DataFrame({"Info": ["Brak danych"]})

    @output
    @render.download(filename=lambda: f"{current_triangle.get()}.{'csv' if input.file_format() == 'csv' else 'xlsx'}")
    def download_modal_data():
        data = get_current_data()

        if data is None:
            yield b""
        else:
            if input.file_format() == "csv":
                yield data.to_csv(index=False).encode("utf-8")
            else:
                import io
                buffer = io.BytesIO()
                data.to_excel(buffer, index=False)
                yield buffer.getvalue()

    def show_download_modal():
        from shiny import ui
        from htmltools import TagList

        ui.modal_show(
            ui.modal(
                TagList(
                    ui.input_radio_buttons(
                        "file_format",
                        "Format pliku:",
                        {"csv": "CSV", "excel": "Excel"},
                        selected="csv"
                    ),
                    ui.download_button("download_modal_data", "Pobierz"),
                    ui.modal_button("Zamknij")
                ),
                title="Pobierz dane",
                easy_close=True,
                footer=None
            )
        )
    def every_triangle():
        # Listy już przechowują wszystkie trójkąty
        lista_trojkatow_paid = reactive_data_paid_list.get()
        lista_trojkatow_incurred = reactive_data_incurred_list.get()
        lista_expo_list = []

        # Ekspozycja — zbieramy pierwszy słupek z każdego DataFrame w liście
        lista_expo = reactive_data_exposure_list.get()
        if lista_expo:
            lista_expo_list = [df.iloc[:, 0].tolist() for df in lista_expo]

        # (Opcjonalnie) jeżeli potrzebujesz jakiejś stałej listy np. linii biznesowych
        l_bis = ["MTPL_I"]

        # Możesz na końcu zwrócić listy, jeśli potrzebujesz ich do analizy w innych miejscach
        return lista_trojkatow_paid, lista_trojkatow_incurred, lista_expo_list, l_bis



app = App(app_ui, server)
run_app(app)
