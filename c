import shinyswatch
import pandas as pd
import numpy as np
from shiny import App, Inputs, Outputs, Session, render, ui,run_app,reactive
from shiny.types import FileInfo
from shiny import experimental as x
import matplotlib.pyplot as plt
from shiny.types import ImgData
import shiny
import openpyxl
from metody_jednoroczne_copy import YearHorizont
yh = YearHorizont()
# Dane
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

def calculate_ratios(df):
   # print(df.to_string())
    ind_all, m_i, m_first = yh.index_all(df.iloc[:,1:])
    macierz_wsp_l = yh.l_i_j(df.iloc[:,1:], ind_all)


    #macierz_wsp_l_out = macierz_wsp_l.iloc[:-1,:-1]

    return macierz_wsp_l

def calculate_ratios_p_lr(df,expo):
    macierz_wsp_l = yh.wspolczynniki_LR_i_j(df.iloc[:,1:], expo)
    return macierz_wsp_l

def calculate_ratios_i_lr(df,expo):
    macierz_wsp_l = yh.wspolczynniki_LR_i_j(df.iloc[:,1:], expo)
    return macierz_wsp_l

def calculate_ratios_pi(df_paid,df_inc):
    macierz_wsp_pi =np.where(df_inc!=0,df_paid/df_inc,np.nan)
    macierz_wsp_pi_pd = pd.DataFrame(macierz_wsp_pi,columns=df_paid.columns)
    return macierz_wsp_pi_pd


def create_binary_df(ratio_df):
    binary_df = ratio_df.map(lambda x: 1 if pd.notna(x) else np.nan)
    return binary_df

#ratio_df = calculate_ratios(df)
#binary_df = create_binary_df(ratio_df)

js_code ="""
$(document).on('click', '#ratios-table-1 td', function() {
    var row = $(this).closest('tr').index();
    var col = $(this).index();
    if (col >= 0) {  // Uwzględnij pierwszą kolumnę
        console.log(`Cell clicked: row=${row}, col=${col}`);
        if ($(this).hasClass('highlighted')) {
            $(this).removeClass('highlighted');
            Shiny.setInputValue('clicked_cell_ratios_table_1', 
                { row: row, col: col, highlighted: false });
        } else {
            $(this).addClass('highlighted');
            Shiny.setInputValue('clicked_cell_ratios_table_1', 
                { row: row, col: col, highlighted: true });
        }
    }
});

let highlightedCells = [];

function highlight_default_cell(offset) {
    var table = document.getElementById('ratios-table-1');
    if (table) {
        console.log(`Highlighting cells with offset: ${offset}`);

        // Usuwanie istniejących podświetleń przed dodaniem nowych
        remove_highlights([]);

        if (offset === 0) {
            console.log("Offset is zero, no cells will be highlighted.");
            return;
        }

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

                    let targetRow = baseOffset - (colIndex - 1) - offset-1;
                    if (targetRow >= 1) {
                        for (let highlightRow = targetRow; highlightRow >= 1; highlightRow--) {
                            let targetCell = table.rows[highlightRow].cells[colIndex];

                            if (!targetCell.classList.contains('highlighted')) {
                                targetCell.classList.add('highlighted');
                                newHighlightedCells.push({ row: highlightRow-1, col: colIndex, highlighted: true });
                            }
                        }
                    }
                    break;
                }
            }
        }

        highlightedCells = newHighlightedCells;

        // Wysyłanie wszystkich podświetlonych komórek jednocześnie do Shiny
        Shiny.setInputValue('all_generated_cells_ratios_table_1', highlightedCells, { priority: "event" });
    }
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

js_code_pi_i = """
$(document).on('click', '#ratios-table-2 td', function() {
    var row = $(this).closest('tr').index();
    var col = $(this).index();
    if (col >= 0) {  // Uwzględnij pierwszą kolumnę
        console.log(`Cell clicked: row=${row}, col=${col}`);
        if ($(this).hasClass('highlighted')) {
            $(this).removeClass('highlighted');
            Shiny.setInputValue('clicked_cell_ratios_table_2', 
                { row: row, col: col, highlighted: false });
        } else {
            $(this).addClass('highlighted');
            Shiny.setInputValue('clicked_cell_ratios_table_2', 
                { row: row, col: col, highlighted: true });
        }
    }
});

let highlightedCellsIncurred = [];

function highlight_default_cell_incurred(offset) {
    var table = document.getElementById('ratios-table-2');
    if (table) {
        console.log(`Highlighting cells with offset: ${offset}`);

        // Usuwanie istniejących podświetleń przed dodaniem nowych
        remove_highlights_incurred([]);

        if (offset === 0) {
            console.log("Offset is zero, no cells will be highlighted.");
            return;
        }

        let newHighlightedCellsIncurred = [];
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

                    let targetRow = baseOffset - (colIndex - 1) - offset-1;
                    if (targetRow >= 1) {
                        for (let highlightRow = targetRow; highlightRow >= 1; highlightRow--) {
                            let targetCell = table.rows[highlightRow].cells[colIndex];

                            if (!targetCell.classList.contains('highlighted')) {
                                targetCell.classList.add('highlighted');
                                newHighlightedCellsIncurred .push({ row: highlightRow-1, col: colIndex, highlighted: true });
                            }
                        }
                    }
                    break;
                }
            }
        }

        highlightedCellsIncurred = newHighlightedCellsIncurred ;

        // Wysyłanie wszystkich podświetlonych komórek jednocześnie do Shiny
        Shiny.setInputValue('all_generated_cells_ratios_table_2', highlightedCellsIncurred, { priority: "event" });
    }
}

function remove_highlights_incurred(newHighlightedCellsIncurred) {
    var table = document.getElementById('ratios-table-2');
    highlightedCellsIncurred.forEach(function (cell) {
        var targetCell = table.rows[cell.row].cells[cell.col];
        if (!newHighlightedCellsIncurred.some(newCell => newCell.row === cell.row && newCell.col === cell.col)) {
            targetCell.classList.remove('highlighted');
        }
    });
}

document.addEventListener('DOMContentLoaded', function () {
    Shiny.addCustomMessageHandler('highlight_cells', function (offset) {
        highlight_default_cell_incurred(offset);
    });
});

"""

js_code_p_lr ="""
 $(document).on('click', '#ratios-table-4 td', function() {
    var row = $(this).closest('tr').index();
    var col = $(this).index();
    if (col >= 0) {  // Uwzględnij pierwszą kolumnę
        console.log(`Cell clicked: row=${row}, col=${col}`);
        if ($(this).hasClass('highlighted')) {
            $(this).removeClass('highlighted');
            Shiny.setInputValue('clicked_cell_ratios_table_4', 
                { row: row, col: col, highlighted: false });
        } else {
            $(this).addClass('highlighted');
            Shiny.setInputValue('clicked_cell_ratios_table_4', 
                { row: row, col: col, highlighted: true });
        }
    }
});

let highlightedCells_p_lr = [];

function highlight_default_cell_p_lr(offset) {
    var table = document.getElementById('ratios-table-4');
    if (table) {
        console.log(`Highlighting cells with offset: ${offset}`);

        // Usuwanie istniejących podświetleń przed dodaniem nowych
        remove_highlights_p_lr([]);

        if (offset === 0) {
            console.log("Offset is zero, no cells will be highlighted.");
            return;
        }

        let newhighlightedCells_p_lr = [];
        let baseOffset = -1;
        let foundBase = false;

        for (let colIndex = 1; colIndex < table.rows[0].cells.length; colIndex++) {
            let foundBase = false;
            let baseOffset = -1;
            for (let rowIndex = table.rows.length-1; rowIndex > 0; rowIndex--) {
                let cell = table.rows[rowIndex].cells[colIndex];
                if (cell.innerText.trim() !== '0') {
                    if (!foundBase) {
                        baseOffset = rowIndex;
                        foundBase = true;
                    }

                    let targetRow = baseOffset - (colIndex ) - offset+1;
                    if (targetRow > 0) {
                        for (let highlightRow = targetRow; highlightRow >= 1; highlightRow--) {
                            let targetCell = table.rows[highlightRow].cells[colIndex];

                            if (!targetCell.classList.contains('highlighted')) {
                                targetCell.classList.add('highlighted');
                                newhighlightedCells_p_lr .push({ row: highlightRow-1, col: colIndex-1, highlighted: true });
                            }
                        }
                    }
                    break;
                }
            }
        }

        highlightedCells_p_lr = newhighlightedCells_p_lr ;

        // Wysyłanie wszystkich podświetlonych komórek jednocześnie do Shiny
        Shiny.setInputValue('all_generated_cells_ratios_table_4', highlightedCells_p_lr, { priority: "event" });
    }
}

function remove_highlights_p_lr(newhighlightedCells_p_lr) {
    var table = document.getElementById('ratios-table-4');
    highlightedCells_p_lr.forEach(function (cell) {
        var targetCell = table.rows[cell.row].cells[cell.col];
        if (!newhighlightedCells_p_lr.some(newCell => newCell.row === cell.row && newCell.col === cell.col)) {
            targetCell.classList.remove('highlighted');
        }
    });
}

document.addEventListener('DOMContentLoaded', function () {
    Shiny.addCustomMessageHandler('highlight_cells_p_lr', function (offset) {
        highlight_default_cell_p_lr(offset);
    });
});

"""



js_code_i_lr ="""
 $(document).on('click', '#ratios-table-5 td', function() {
    var row = $(this).closest('tr').index();
    var col = $(this).index();
    if (col >= 0) {  // Uwzględnij pierwszą kolumnę
        console.log(`Cell clicked: row=${row}, col=${col}`);
        if ($(this).hasClass('highlighted')) {
            $(this).removeClass('highlighted');
            Shiny.setInputValue('clicked_cell_ratios_table_5', 
                { row: row, col: col, highlighted: false });
        } else {
            $(this).addClass('highlighted');
            Shiny.setInputValue('clicked_cell_ratios_table_5', 
                { row: row, col: col, highlighted: true });
        }
    }
});

let highlightedCells_i_lr = [];

function highlight_default_cell_i_lr(offset) {
    var table = document.getElementById('ratios-table-5');
    if (table) {
        console.log(`Highlighting cells with offset: ${offset}`);

        // Usuwanie istniejących podświetleń przed dodaniem nowych
        remove_highlights_i_lr([]);

        if (offset === 0) {
            console.log("Offset is zero, no cells will be highlighted.");
            return;
        }

        let newhighlightedCells_i_lr = [];
        let baseOffset = -1;
        let foundBase = false;

        for (let colIndex = 1; colIndex < table.rows[0].cells.length; colIndex++) {
            let foundBase = false;
            let baseOffset = -1;
            for (let rowIndex = table.rows.length-1; rowIndex > 0; rowIndex--) {
                let cell = table.rows[rowIndex].cells[colIndex];
                if (cell.innerText.trim() !== '0') {
                    if (!foundBase) {
                        baseOffset = rowIndex;
                        foundBase = true;
                    }

                    let targetRow = baseOffset - (colIndex ) - offset+1;
                    if (targetRow > 0) {
                        for (let highlightRow = targetRow; highlightRow >= 1; highlightRow--) {
                            let targetCell = table.rows[highlightRow].cells[colIndex];

                            if (!targetCell.classList.contains('highlighted')) {
                                targetCell.classList.add('highlighted');
                                newhighlightedCells_i_lr .push({ row: highlightRow-1, col: colIndex-1, highlighted: true });
                            }
                        }
                    }
                    break;
                }
            }
        }

        highlightedCells_i_lr = newhighlightedCells_i_lr ;

        // Wysyłanie wszystkich podświetlonych komórek jednocześnie do Shiny
        Shiny.setInputValue('all_generated_cells_ratios_table_5', highlightedCells_i_lr, { priority: "event" });
    }
}

function remove_highlights_i_lr(newhighlightedCells_i_lr) {
    var table = document.getElementById('ratios-table-5');
    highlightedCells_i_lr.forEach(function (cell) {
        var targetCell = table.rows[cell.row].cells[cell.col];
        if (!newhighlightedCells_i_lr.some(newCell => newCell.row === cell.row && newCell.col === cell.col)) {
            targetCell.classList.remove('highlighted');
        }
    });
}

document.addEventListener('DOMContentLoaded', function () {
    Shiny.addCustomMessageHandler('highlight_cells_i_lr', function (offset) {
        highlight_default_cell_i_lr(offset);
    });
});

"""


js_code_pi_lr =  """
$(document).on('click', '#ratios-table-6 td', function() {
    var row = $(this).closest('tr').index();
    var col = $(this).index();
    if ($(this).hasClass('highlighted')) {
        $(this).removeClass('highlighted');
        Shiny.setInputValue('clicked_cell_ratios_table_6', {row: row, col: col - 1, highlighted: false});
    } else {
        $(this).addClass('highlighted');
        Shiny.setInputValue('clicked_cell_ratios_table_6', {row: row, col: col - 1, highlighted: true});
    }
});
"""

css_code = """
.highlighted {
    background-color: gray !important;
}
"""


import os


# Funkcja do wczytywania danych z Excela
def load_excel_data(folder_path, file_name, sheet_name, start_row, start_col, end_row, end_col):
    # Zmiana ukośników z \ na / w ścieżce do folderu
    folder_path = folder_path.replace("\\", "/")

    # Tworzymy pełną ścieżkę do pliku Excel
    file_path = os.path.join(folder_path, file_name)

    # Używamy pandas do wczytania danych z określonego arkusza i zakresu
    data = pd.read_excel(file_path,
                         sheet_name=sheet_name,
                         header=None,  # Oznacza, że nie traktujemy żadnego wiersza jako nagłówka
                         engine="openpyxl",
                         skiprows=start_row - 1,  # Pomijamy wiersze przed startowym wierszem
                         usecols=range(start_col - 1, end_col),  # Wczytujemy tylko wybrany zakres kolumn
                         nrows=end_row - start_row + 1)
    #data.insert(0, "Czas szkody", data.index.tolist())
    #data.insert(0, "Czas szkody", data.index.tolist())
    return data


##############


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
                    }, ),
                ui.input_action_button("load_button", "Wczytaj dane"),

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
                        "Skumulowane":"Skumulowane",
                        "Inkrementalne":"Inkrementalne"
                    },),
                ui.input_action_button("load_button_inc", "Wczytaj dane")
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
                ui.input_action_button("load_button_eksp", "Wczytaj dane")
            ),
        ),

           ),
    ui.nav_panel("Metody deterministyczne",ui.navset_tab(
        ui.nav_panel('Chain-Ladder Paid', ui.layout_sidebar(ui.panel_sidebar(ui.input_selectize("linie_biznesowe_CL_Paid", "Wybierz linię biznesową", choices=['MTPL_I'], multiple=False),
                                                                       ui.input_numeric("offset_distance",
                                                                                        "Pozostawiona liczba przekątnych", value=0,
                                                                                        min=0),

                                                                       ui.input_checkbox("dop_all_factor",
                                                                               "Dopasuj do wszystkich współczynników",
                                                                               True),
                                                             ui.input_selectize('chose_CL',
                                                                                'Wybierz CL do dopasowania krzywej',
                                                                                choices=[int(x) for x in range(1, 1000)], multiple=True),
                                                             ui.input_action_button("accept_CL", "Dopasuj krzywą",
                                                                                    class_="btn-success"),
                                                             ui.input_numeric("num_of_first_factor",
                                                                              "Pozostaw rzeczywistych współczynników",
                                                                              value=1),
                                                             ui.input_numeric("num_tail",
                                                                              "Ilość obserwacji w ogonie",
                                                                              value=0),
                                                             ui.input_action_button("accept_final_factor", "Zatwierdź",
                                                                                    class_="btn-success"),
                                                             ui.input_selectize("wyb_krzywa_ogona",
                                                                                "Wybierz ogon z krzywej",
                                                                                choices=['Exponential', 'Power',
                                                                                         "Weibull", "Inverse Power"],
                                                                                multiple=False),
                                                            ui.input_text("folder_path_save_p_cl",
                                                                                           "Ścieżka do folderu:",
                                                                                           value="C:/Users/Documents"),
                                                            ui.input_text("file_name_save_p_cl",
                                                                                           "Nazwa pliku Excel:",
                                                                                           value="plik.xlsx"),
                                                            ui.input_action_button("save_button_save_p_cl",
                                                                                                    "Zapisz tabelę"),

                                          width = 2,),
            ui.panel_main(
                ui.navset_tab(
                    ui.nav_panel("Trójkąt",
                        ui.output_ui("triangle_table"),
                    ),
                    ui.nav_panel("Współczynniki CL",
                           ui.output_ui("ratios_table_ui_p")
                           ),
                    ui.nav_panel("Wagi",
                           ui.output_ui("binary_ratios_table_ui_p")
                           ),
                    ui.nav_panel("Dopasowanie krzywej CL",
                            ui.page_fillable(shiny.ui.page_fillable(x.ui.card(ui.output_data_frame("macierz_wspol_CL_interaktywna"), ),
                                                                       height=180)),
                                    ui.page_fillable(shiny.ui.page_fillable( x.ui.card(
                                        ui.output_data_frame("wspol_z_krzywej_CL_paid_interaktywna"), ), height=230)),
                                       shiny.ui.page_fillable(

                                           x.ui.card(
                                               ui.output_plot("plot_wspolczynniki_dopasowane_interaktywny"),
                                           ),
                                           height=400),
                                   ui.page_fillable(shiny.ui.page_fillable( x.ui.card(
                                       ui.output_data_frame("r2_cl_paid"), ),
                                                                              height=100)),
                           ),
                    ui.nav_panel("Wybór krzywej CL",
                           shiny.ui.page_fillable( x.ui.card(
                               ui.output_data_frame(
                                   "Factors_curve_and_initial"), ),
                                                   height=300),
                           ui.page_fillable(
                               shiny.ui.page_fillable( x.ui.card(
                                   ui.output_data_frame(
                                       "Final_Factor"), ), height=300)),

                           ui.page_fillable(
                               shiny.ui.page_fillable( x.ui.card(
                                   ui.output_data_frame(
                                       "Choose_Factor"), ), height=300)),

                           ),
                    ui.nav_panel("Wyniki",
                        ui.page_fillable(shiny.ui.page_fillable( x.ui.card(
                            ui.output_data_frame("Ult_BE_data_interaktywne"), ), height=750)),
                           ),
                ),
                ui.tags.style(css_code),
                ui.tags.script(js_code)
            )
                                            )),
        ui.nav_panel('Chain-Ladder Incurred', ui.layout_sidebar(ui.panel_sidebar(
            ui.input_selectize("linie_biznesowe_CL_Incurred", "Wybierz linię biznesową", choices=['MTPL_I'],
                               multiple=False),
            ui.input_numeric("offset_distance_incurred", "Pozostawiona liczba przekątnych", value=0, min=0),
            ui.input_checkbox("dop_all_factor_i_cl",
                              "Dopasuj do wszystkich współczynników",
                              True),
            ui.input_selectize('chose_CL_i_cl',
                               'Wybierz CL do dopasowania krzywej',
                               choices=[int(x) for x in range(1, 1000)], multiple=True),
            ui.input_action_button("accept_CL_i_cl", "Dopasuj krzywą",
                                   class_="btn-success"),
            ui.input_numeric("num_of_first_factor_i_cl",
                             "Pozostaw rzeczywistych współczynników",
                             value=1),
            ui.input_numeric("num_tail_i_cl",
                             "Ilość obserwacji w ogonie",
                             value=0),
            ui.input_action_button("accept_final_factor_i_cl", "Zatwierdź",
                                   class_="btn-success"),
            ui.input_selectize("wyb_krzywa_ogona_i_cl",
                               "Wybierz ogon z krzywej",
                               choices=['Exponential', 'Power',
                                        "Weibull", "Inverse Power"],
                               multiple=False),
            ui.input_text("folder_path_save_i_cl",
                          "Ścieżka do folderu:",
                          value="C:/Users/Documents"),
            ui.input_text("file_name_save_i_cl",
                          "Nazwa pliku Excel:",
                          value="plik.xlsx"),
            ui.input_action_button("save_button_save_i_cl",
                                   "Zapisz tabelę"),
            width=2, ),
            ui.navset_tab(
                ui.nav_panel("Trójkąt Inc",
                       ui.output_ui("triangle_table_pi_i"),
                       ),
                ui.nav_panel("Współczynniki CL",
                       ui.output_ui("ratios_table_ui_pi_i")
                       ),
                ui.nav_panel("Wagi",
                       ui.output_ui("binary_ratios_table_ui_pi_i")
                       ),
                ui.nav_panel("Dopasowanie krzywej CL",
                       ui.page_fillable(shiny.ui.page_fillable( x.ui.card(
                           ui.output_data_frame("macierz_wspol_CL_interaktywna_i_cl"), ),
                                                                  height=180)),
                       ui.page_fillable(shiny.ui.page_fillable( x.ui.card(
                           ui.output_data_frame("wspol_z_krzywej_CL_paid_interaktywna_i_cl"), ), height=230)),
                       shiny.ui.page_fillable(

                           x.ui.card(
                               ui.output_plot("plot_wspolczynniki_dopasowane_interaktywny_i_cl"),
                           ),
                           height=400),
                       ui.page_fillable(shiny.ui.page_fillable( x.ui.card(
                           ui.output_data_frame("r2_cl_paid_i_cl"), ),
                                                                  height=100)),
                       ),
                ui.nav_panel("Wybór krzywej CL",
                       shiny.ui.page_fillable( x.ui.card(
                           ui.output_data_frame(
                               "Factors_curve_and_initial_i_cl"), ),
                                               height=300),
                       ui.page_fillable(
                           shiny.ui.page_fillable( x.ui.card(
                               ui.output_data_frame(
                                   "Final_Factor_i_cl"), ), height=300)),

                       ui.page_fillable(
                           shiny.ui.page_fillable( x.ui.card(
                               ui.output_data_frame(
                                   "Choose_Factor_i_cl"), ), height=300)),

                       ),
                ui.nav_panel("Wyniki",
                       ui.page_fillable(shiny.ui.page_fillable( x.ui.card(
                           ui.output_data_frame("Ult_BE_data_interaktywne_i_cl"), ), height=750)),

                       ),

            ),
            ui.tags.style(css_code),
            ui.tags.script(js_code_pi_i),
            #ui.tags.script(js_code_pi_pi)


        )
               ),
        ui.nav_panel('Addytywna Paid', ui.layout_sidebar(ui.panel_sidebar(
            ui.input_selectize("linie_biznesowe_LR_Paid", "Wybierz linię biznesową", choices=['MTPL_I'],
                               multiple=False),
            ui.input_numeric("offset_distance_p_lr", "Pozostawiona liczba przekątnych", value=0, min=0),

            ui.input_checkbox("dop_all_factor_p_lr",
                              "Dopasuj do wszystkich współczynników",
                              True),
            ui.input_selectize('chose_CL_p_lr',
                               'Wybierz CL do dopasowania krzywej',
                               choices=[int(x) for x in range(1, 1000)], multiple=True),
            ui.input_action_button("accept_p_lr", "Dopasuj krzywą",
                                   class_="btn-success"),
            ui.input_numeric("num_of_first_factor_p_lr",
                             "Pozostaw rzeczywistych współczynników",
                             value=1),
            ui.input_numeric("num_tail_p_lr",
                             "Ilość obserwacji w ogonie",
                             value=0),
            ui.input_action_button("accept_final_factor_p_lr", "Zatwierdź",
                                   class_="btn-success"),
            ui.input_selectize("wyb_krzywa_ogona_p_lr",
                               "Wybierz ogon z krzywej",
                               choices=['Exponential', 'Power',
                                        "Weibull", "Inverse Power"],
                               multiple=False),
            ui.input_text("folder_path_save_p_lr",
                          "Ścieżka do folderu:",
                          value="C:/Users"),
            ui.input_text("file_name_save_p_lr",
                          "Nazwa pliku Excel:",
                          value="plik.xlsx"),
            ui.input_action_button("save_button_save_p_lr",
                                   "Zapisz tabelę"),
            width=2, ),
            ui.panel_main(
                ui.navset_tab(
                    ui.nav_panel("Trójkąt",
                        ui.output_ui("triangle_table_p_lr"),
                    ),
                    ui.nav_panel("Współczynniki LR",
                           ui.output_ui("ratios_table_ui_p_lr")
                           ),
                    ui.nav_panel("Wagi",
                           ui.output_ui("binary_ratios_table_ui_p_lr")
                           ),
                    ui.nav_panel("Dopasowanie krzywej CL",
                           ui.page_fillable(shiny.ui.page_fillable( x.ui.card(
                               ui.output_data_frame("macierz_wspol_CL_interaktywna_p_lr"), ),
                                                                      height=180)),
                           ui.page_fillable(shiny.ui.page_fillable( x.ui.card(
                               ui.output_data_frame("wspol_z_krzywej_CL_paid_interaktywna_p_lr"), ), height=230)),
                           shiny.ui.page_fillable(

                               x.ui.card(
                                   ui.output_plot("plot_wspolczynniki_dopasowane_interaktywny_p_lr"),
                               ),
                               height=400),
                           ui.page_fillable(shiny.ui.page_fillable(x.ui.card(
                               ui.output_data_frame("r2_cl_paid_p_lr"), ),
                                                                      height=100)),
                           ),
                    ui.nav_panel("Wybór krzywej CL",
                           shiny.ui.page_fillable(x.ui.card(
                               ui.output_data_frame(
                                   "Factors_curve_and_initial_p_lr"), ),
                                                   height=300),
                           ui.page_fillable(
                               shiny.ui.page_fillable( x.ui.card(
                                   ui.output_data_frame(
                                       "Final_Factor_p_lr_out"), ), height=300)),

                           ui.page_fillable(
                               shiny.ui.page_fillable( x.ui.card(
                                   ui.output_data_frame(
                                       "Choose_Factor_p_lr"), ), height=300)),

                           ),
                    ui.nav_panel("Wyniki",
                           ui.page_fillable(shiny.ui.page_fillable( x.ui.card(
                               ui.output_data_frame("Ult_BE_data_interaktywne_p_lr"), ), height=750)),

                           ),

                ),
                ui.tags.style(css_code),
                ui.tags.script(js_code_p_lr)
            )

        )
               ),
        ui.nav_panel('Addytywna Incurred', ui.layout_sidebar(ui.panel_sidebar(
            ui.input_selectize("linie_biznesowe_LR_Incurred", "Wybierz linię biznesową",
                               choices=['MTPL_I'],
                               multiple=False),
            ui.input_numeric("offset_distance_i_lr", "Pozostawiona liczba przekątnych", value=0, min=0),

            ui.input_checkbox("dop_all_factor_i_lr",
                              "Dopasuj do wszystkich współczynników",
                              True),
            ui.input_selectize('chose_CL_i_lr',
                               'Wybierz CL do dopasowania krzywej',
                               choices=[int(x) for x in range(1, 1000)], multiple=True),
            ui.input_action_button("accept_i_lr", "Dopasuj krzywą",
                                   class_="btn-success"),
            ui.input_numeric("num_of_first_factor_i_lr",
                             "Pozostaw rzeczywistych współczynników",
                             value=1),
            ui.input_numeric("num_tail_i_lr",
                             "Ilość obserwacji w ogonie",
                             value=0),
            ui.input_action_button("accept_final_factor_i_lr", "Zatwierdź",
                                   class_="btn-success"),
            ui.input_selectize("wyb_krzywa_ogona_i_lr",
                               "Wybierz ogon z krzywej",
                               choices=['Exponential', 'Power',
                                        "Weibull", "Inverse Power"],
                               multiple=False),
            ui.input_text("folder_path_save_i_lr",
                          "Ścieżka do folderu:",
                          value="C:/Users/Documents"),
            ui.input_text("file_name_save_i_lr",
                          "Nazwa pliku Excel:",
                          value="plik.xlsx"),
            ui.input_action_button("save_button_save_i_lr",
                                   "Zapisz tabelę"),
            width=2, ),
            ui.panel_main(
                ui.navset_tab(
                    ui.nav_panel("Trójkąt",
                        ui.output_ui("triangle_table_i_lr"),
                    ),
                    ui.nav_panel("Współczynniki LR",
                           ui.output_ui("ratios_table_ui_i_lr")
                           ),
                    ui.nav_panel("Wagi",
                           ui.output_ui("binary_ratios_table_ui_i_lr")
                           ),
                    ui.nav_panel("Dopasowanie krzywej CL",
                           ui.page_fillable(shiny.ui.page_fillable( x.ui.card(
                               ui.output_data_frame("macierz_wspol_CL_interaktywna_i_lr"), ),
                                                                      height=180)),
                           ui.page_fillable(shiny.ui.page_fillable( x.ui.card(
                               ui.output_data_frame("wspol_z_krzywej_CL_paid_interaktywna_i_lr"), ), height=230)),
                           shiny.ui.page_fillable(

                               x.ui.card(
                                   ui.output_plot("plot_wspolczynniki_dopasowane_interaktywny_i_lr"),
                               ),
                               height=400),
                           ui.page_fillable(shiny.ui.page_fillable( x.ui.card(
                               ui.output_data_frame("r2_cl_paid_i_lr"), ),
                                                                      height=100)),
                           ),
                    ui.nav_panel("Wybór krzywej CL",
                           shiny.ui.page_fillable( x.ui.card(
                               ui.output_data_frame(
                                   "Factors_curve_and_initial_i_lr"), ),
                                                   height=300),
                           ui.page_fillable(
                               shiny.ui.page_fillable( x.ui.card(
                                   ui.output_data_frame(
                                       "Final_Factor_i_lr_out"), ), height=300)),

                           ui.page_fillable(
                               shiny.ui.page_fillable( x.ui.card(
                                   ui.output_data_frame(
                                       "Choose_Factor_i_lr"), ), height=300)),

                           ),
                    ui.nav_panel("Wyniki",
                           ui.page_fillable(shiny.ui.page_fillable( x.ui.card(
                               ui.output_data_frame("Ult_BE_data_interaktywne_i_lr"), ), height=750)),

                           ),

                ),
                ui.tags.style(css_code),
                ui.tags.script(js_code_i_lr),
                ui.tags.script(js_code_pi_lr),

            )

        )
               ),)),
    title="",

),
ui.tags.style("""
    #triangle_table, #triangle_table th, #triangle_table td {
        white-space: nowrap;  /* Zapobiega zawijaniu tekstu w komórkach */
        text-align: right;  /* Wyśrodkowanie liczb */
    }
        #triangle_table_pi_i, #triangle_table_pi_i th, #triangle_table_pi_i td {
        white-space: nowrap;  /* Zapobiega zawijaniu tekstu w komórkach */
        text-align: right;  /* Wyśrodkowanie liczb */
    }
        #triangle_table_p_lr, #triangle_table_p_lr th, #triangle_table_p_lr td {
        white-space: nowrap;  /* Zapobiega zawijaniu tekstu w komórkach */
        text-align: right;  /* Wyśrodkowanie liczb */
    }
        #triangle_table_i_lr, #triangle_table_i_lr th, #triangle_table_i_lr td {
        white-space: nowrap;  /* Zapobiega zawijaniu tekstu w komórkach */
        text-align: right;  /* Wyśrodkowanie liczb */
    }
""")
)


def server(input: Inputs, output: Outputs, session: Session):
    clicked_cells_p = reactive.Value([])
    update_trigger_p = reactive.Value(0)
    update_trigger_pi_i = reactive.Value(0)
    update_trigger_pi_pi = reactive.Value(0)
    update_trigger_p_lr = reactive.Value(0)
    update_trigger_i_lr = reactive.Value(0)
    update_trigger_pi_lr = reactive.Value(0)
    reactive_data = reactive.Value(pd.DataFrame())
    reactive_data_inc = reactive.Value(pd.DataFrame())
    reactive_data_eksp = reactive.Value(pd.DataFrame())


    linia_biznesowa = reactive.Value(0)
    #print(reactive.Value(0))
    global ratio_df_p, binary_df_p
    global ratio_df_pi_i, binary_df_pi_i
    global ratio_df_pi_pi, binary_df_pi_pi
    global ratio_df_p_lr, binary_df_p_lr
    global ratio_df_i_lr, binary_df_i_lr
    global ratio_df_pi_lr, binary_df_pi_lr

    ########################################## paid CL
    @output
    @render.ui
    def triangle_table():
        trian_list = every_triangle()[0]
        trian_for_line = trian_list[index_lini_CL_Paid()]
        index_linii = index_lini_CL_Paid()
        update_global_variables(index_linii)
        df_copy = trian_for_line.copy()

        df_copy.iloc[:,1:] = df_copy.iloc[:,1:].map(lambda x: f"{x:,.2f}".replace(",", " ") if isinstance(x, (int, float)) else x)


        return ui.HTML(
            df_copy.to_html(classes='table table-striped table-hover'))

    def update_global_variables(ind_linii):
        global ratio_df_p, binary_df_p
        trian_list = every_triangle()[0]
        trian_for_line = trian_list[ind_linii]
        ratio_df_p = calculate_ratios(trian_for_line)
        binary_df_p = create_binary_df(ratio_df_p)

    clicked_cells_p = reactive.Value([])

   # def update_global_variables(ind_linii):
    #    global ratio_df_p, binary_df_p
    #    trian_list = every_triangle()[0]
     #   trian_for_line = trian_list[ind_linii]
     #   ratio_df_p = calculate_ratios(trian_for_line)
     #   binary_df_p = create_binary_df(ratio_df_p)

    clicked_cells_p = reactive.Value([])

    @reactive.Effect
    @reactive.event(lambda: input.clicked_cell_ratios_table_1())
    def update_clicked_cell_p():
        cell = input.clicked_cell_ratios_table_1()
        if cell:
            row, col, highlighted = cell['row'], cell['col']-1, cell['highlighted']
            if row < binary_df_p.shape[0] and col < binary_df_p.shape[1]:  # Check bounds
                if highlighted:
                    binary_df_p.iat[row, col] = 0
                else:
                    binary_df_p.iat[row, col] = 1
                update_trigger_p.set(update_trigger_p.get() + 1)

    @reactive.Effect
    @reactive.event(input.offset_distance)
    async def update_highlight_default_cell():
        offset = input.offset_distance()
        if offset == 0:
            await session.send_custom_message('highlight_cells', 0)
        else:
            await session.send_custom_message('highlight_cells', offset)

    @reactive.Effect
    @reactive.event(lambda: input.all_generated_cells_ratios_table_1())
    def update_all_generated_cells():
        cells = input.all_generated_cells_ratios_table_1()
        if cells:
            for cell in cells:
                row, col, highlighted = cell['row']-1 , cell['col'], cell['highlighted']
                if row < binary_df_p.shape[0] and col < binary_df_p.shape[1]:  # Sprawdzenie zakresów
                    if highlighted:
                        binary_df_p.iat[row, col] = 0
                    else:
                        binary_df_p.iat[row, col] = 1
            update_trigger_p.set(update_trigger_p.get() + 1)

    @output
    @render.ui
    def ratios_table_ui_p():
        update_global_variables(linia_biznesowa.get())
        return ui.HTML(
            ratio_df_p.to_html(classes='table table-striped table-hover', table_id="ratios-table-1")
        ), ui.tags.script("highlight_default_cell(" + str(input.offset_distance()) + ");")

    @output
    @render.ui
    def binary_ratios_table_ui_p():
        update_trigger_p.get()
        df = binary_df_p.copy()
        return ui.HTML(
            df.to_html(classes='table table-striped table-hover', table_id="binary-ratios-table-1", na_rep='NaN',
                       float_format='{:.0f}'.format))

    # @reactive.Calc
    #@reactive.Calc
    @reactive.event(input.clicked_cell_ratios_table_1)
    def wspolczynniki_multiplikatywna_interaktywna():
        trian_list = every_triangle()[0]
        trian_for_line = trian_list[index_lini_CL_Paid()]
        triagnle = trian_for_line.iloc[:, 1:]
        binary_df_deterministic = create_binary_df(triagnle)
        ind_all, m_i, m_first = yh.index_all(triagnle)
        macierz_wsp_l = yh.l_i_j(triagnle, ind_all)
        Dev_j_deterministic = yh.Dev(triagnle, binary_df_deterministic, macierz_wsp_l, ind_all)
        Dev_j = yh.Dev(triagnle, binary_df_p, macierz_wsp_l, ind_all)
        sigma_j = yh.sigma(triagnle, binary_df_p, macierz_wsp_l, Dev_j, ind_all)
        sd_j = yh.wspolczynnik_sd(triagnle, binary_df_p, sigma_j, ind_all)
        sd_pd = [x ** 2 for x in sd_j]
        I_dataframe = pd.DataFrame(index=['CL_base', 'CL'],
                                   columns=["Wybór"] + ['dp: ' + str(j) for j in range(1, len(Dev_j) + 1)])
        I_dataframe.iloc[0, :] = ["CL_base"] + Dev_j_deterministic
        I_dataframe.iloc[1, :] = ["CL"] + Dev_j
        return (I_dataframe)

    @output
    @render.data_frame
    def macierz_wspol_CL_interaktywna():
        df_out_mult= wspolczynniki_multiplikatywna_interaktywna()
        return render.DataGrid(
            df_out_mult,
            width="100%",
            height="150%",
        )

    @reactive.Calc
    @reactive.event(input.accept_CL, ignore_none=False)
    def wsp_mult_paid():
        ys = []
        xs = []
        y = wspolczynniki_multiplikatywna_interaktywna().iloc[1, 1:].tolist()
        if (input.dop_all_factor() == True):
            ii = 0
            for k in range(1, len(y) + 1):
                ii = ii + 1
                if (y[k - 1] > 1):
                    ys.append(y[k - 1])
                    xs.append(ii)
        elif (input.dop_all_factor() == False):
            for chose_factor in input.chose_CL():
                ys.append(y[int(chose_factor) - 1])
                xs.append(int(chose_factor))
        return ([np.array(xs, dtype=np.float64), np.array(ys, dtype=np.float64)])

    #   @reactive.Effect
    #  def _():
    #      ui.update_selectize('chose_CL', choices=[str(x) for x in range(1, wspolczynniki_multiplikatywna_interaktywna.shape[1])])

    @reactive.Calc
    @reactive.event(input.accept_CL, ignore_none=False)
    def dopasowanie_krzywej_factor_interaktywne():
        ys = wsp_mult_paid()[1]
        xs = wsp_mult_paid()[0]
        list_curve = ['Exponential', "Weibull", "Power", "Inverse Power"]
        parameters_curve = yh.parameters_curve_rezerwy(xs, ys, list_curve, "Paid")

        return (parameters_curve)



    @reactive.Calc
    # @reactive.event(input.accept_CL, ignore_none=False)
    def predykcja_krzywej_factor_interaktywne():
        parameters_curve = dopasowanie_krzywej_factor_interaktywne()
        list_curve = ['Exponential', "Weibull", "Power", "Inverse Power"]
        xs_pred = np.array(np.arange(1, len(wspolczynniki_multiplikatywna_interaktywna().iloc[1, 1:]) + 1))
        f_curves_graph_real = yh.sim_data_curve_rezerwy(xs_pred, list_curve, parameters_curve)
        f_curves_graph_real.insert(0, "Krzywa", list_curve)
        return (f_curves_graph_real)

    def change_ind_to_col(df_data, name_col):
        if (name_col in df_data.columns.to_list()):
            df_data = df_data.copy()
        else:
            df_data.insert(0, name_col, df_data.index)
        return (df_data)

    @reactive.Calc
    # @reactive.event(input.accept_CL, ignore_none=False)
    def r2_df_freq_lc_interaktywny():
        ys = wsp_mult_paid()[1]
        xs = wsp_mult_paid()[0]
        list_curve = ['Exponential', "Weibull", "Power", "Inverse Power"]
        parameters_curve = dopasowanie_krzywej_factor_interaktywne()
        f_curves_graph_real_choose = yh.sim_data_curve_rezerwy(xs, list_curve, parameters_curve)
        r2_curves_df = yh.r2_curves(f_curves_graph_real_choose, ys)
        return (r2_curves_df)

    @output
    @render.data_frame
    def wspol_z_krzywej_CL_paid_interaktywna():
        II_dataframe = predykcja_krzywej_factor_interaktywne()
        return render.DataGrid(
            II_dataframe,
            width="100%",
            height="100%",
            summary=False
            #row_selection_mode='single'
        )

    @output
    @render.data_frame
    def r2_cl_paid():
        r2_freq_lc = change_ind_to_col(r2_df_freq_lc_interaktywny(), 'Wartość')
        return render.DataGrid(
            r2_freq_lc,
            width="100%",
            height="100%",
            summary=False,
        )

    @output
    @render.plot()
    def plot_wspolczynniki_dopasowane_interaktywny():
        factor_paid = wspolczynniki_multiplikatywna_interaktywna().iloc[1, 1:]
        xs_input = np.arange(1, len(factor_paid) + 1)
        f_curves_tail_graph = predykcja_krzywej_factor_interaktywne().iloc[:, 1:]
        fig = plt.figure()
        for index_loop, row in f_curves_tail_graph.iterrows():
            plt.plot(row, '-', label=str(index_loop))
        plt.plot(factor_paid, 'o-', linewidth=1, label='Initial Selection')
        plt.legend()
        fig.autofmt_xdate()
        return fig


    @output
    @render.data_frame
    def Factors_curve_and_initial():
        f_curves_tail_graph_cop = predykcja_krzywej_factor_interaktywne()
        f_curves_tail_graph_cop.loc[len(f_curves_tail_graph_cop)] = ['Initial'] + wspolczynniki_multiplikatywna_interaktywna().iloc[
                                                                                     1, 1:].tolist()
        return render.DataGrid(
            f_curves_tail_graph_cop,
            width="100%",
            height="100%",
            summary=False,
        )

    def predykcja_krzywej_factor_interaktywne_tail():
        parameters_curve = dopasowanie_krzywej_factor_interaktywne()
        list_curve = ['Exponential', "Weibull", "Power", "Inverse Power"]
        xs_pred = np.array(
            np.arange(1, len(wspolczynniki_multiplikatywna_interaktywna().iloc[1, 1:]) + 1 + int(input.num_tail())))
        f_curves_graph_real = yh.sim_data_curve_rezerwy(xs_pred, list_curve, parameters_curve)
        f_curves_graph_real.insert(0, "Krzywa", list_curve)
        return (f_curves_graph_real)

    @reactive.Calc
    @reactive.event(input.accept_final_factor, ignore_none=False)
    def calc_Final_Factor():
        df_all_trin_factor = predykcja_krzywej_factor_interaktywne_tail()
        fact_real = wspolczynniki_multiplikatywna_interaktywna().iloc[1, 1:].tolist()
        Final_factors = yh.chose_factor_indywidualny(df_all_trin_factor, fact_real, input.num_of_first_factor())
        return (Final_factors)

    @output
    @render.data_frame
    def Final_Factor():
        Final_factors = calc_Final_Factor()
        return render.DataGrid(
            Final_factors,
            width="100%",
            height="100%",
            summary=False,
        )

    @output
    @render.data_frame
    def Choose_Factor():
        Final_factors = calc_Final_Factor()
        Choose_factors = pd.DataFrame(
                                      columns=['Choose'] + ['dp: ' + str(x) for x in range(1, Final_factors.shape[1])],
                                      index=["Wybrana krzywa"])
        Choose_factors.iloc[0, :] = Final_factors.loc[input.wyb_krzywa_ogona()].to_list()
        return render.DataGrid(
            Choose_factors,
            width="100%",
            height="100%",
            summary=False,
        )

    @reactive.Calc
    def calc_chainladder_interaktywne():
        Final_factors = calc_Final_Factor()
        trian_list = every_triangle()[0]
        triangle = trian_list[index_lini_CL_Paid()].iloc[:, 1:]
        CL_fit = Final_factors.loc[input.wyb_krzywa_ogona()].to_list()
        Dev_j_base = wspolczynniki_multiplikatywna_interaktywna().iloc[0, 1:].to_list()
        Dev_j_z_wagami = wspolczynniki_multiplikatywna_interaktywna().iloc[1, 1:].to_list()
        data_output = pd.DataFrame(0, index=np.arange(0, triangle.shape[0] + 1),
                                   columns=['Rok/Suma', 'Ult_base', 'IBNR_base', 'Ult z wagami', 'IBNR z wagami',
                                            'Ult z krzywą', 'IBNR z krzywą'])
        data_output.iloc[:, 0] = np.arange(0, triangle.shape[0] + 1)
        k = 1

        for wspolczynniki in [Dev_j_base, Dev_j_z_wagami, CL_fit[1:]]:
            proj_triangle = yh.triangle_forward(triangle, wspolczynniki, 0)
            diag = yh.reverse_list(yh.trian_diag(triangle))
            Ultimate_Param_ReservingRisk = proj_triangle.iloc[:, int(proj_triangle.columns[-1]) - 1].to_list()
            data_output.iloc[:, k] = Ultimate_Param_ReservingRisk + [np.sum(Ultimate_Param_ReservingRisk)]
            k = k + 1
            BE_Param_ReservingRisk = [x - y for x, y in zip(Ultimate_Param_ReservingRisk, diag)]
            data_output.iloc[:, k] = BE_Param_ReservingRisk + [np.sum(BE_Param_ReservingRisk)]
            k = k + 1
        return (data_output)

    @output
    @render.data_frame
    def Ult_BE_data_interaktywne():
        df = calc_chainladder_interaktywne()
        trian_list = every_triangle()[0]
        triangle = trian_list[index_lini_CL_Paid()].iloc[:, 0].tolist()
        df.iloc[:, 0] = triangle + ["Suma"]
        df.iloc[:,1:] = df.iloc[:,1:].map(lambda x: f"{x:,.2f}".replace(",", " ") if isinstance(x, (int, float)) else x)
        return render.DataGrid(
            df,
            width="100%",
            height="150%",
        )




#######################################   KONIEC paid CL


    ##########################################   START Incurred CL
    @output
    @render.ui
    def triangle_table_pi_i():
        trian_list = every_triangle()[2]
        trian_for_line = trian_list[index_lini_CL_Incurred()]
        index_linii = index_lini_CL_Incurred()
        update_global_variables_pi_i(index_linii)
        df_copy = trian_for_line.copy()
        df_copy.iloc[:,1:] = df_copy.iloc[:,1:].map(lambda x: f"{x:,.2f}".replace(",", " ") if isinstance(x, (int, float)) else x)
        return ui.HTML(
            df_copy.to_html(classes='table table-striped table-hover'))


    def update_global_variables_pi_i(ind_linii):
        global ratio_df_pi_i, binary_df_pi_i
        trian_list = every_triangle()[2]
        trian_for_line = trian_list[ind_linii]
        ratio_df_pi_i = calculate_ratios(trian_for_line)
        binary_df_pi_i = create_binary_df(ratio_df_pi_i)

    clicked_cells_pi_i = reactive.Value([])

    @reactive.Effect
    @reactive.event(lambda: input.clicked_cell_ratios_table_2())
    def update_clicked_cell_pi_i():
        cell = input.clicked_cell_ratios_table_2()
        if cell:
            row, col, highlighted = cell['row'], cell['col']-1, cell['highlighted']
            if row < binary_df_pi_i.shape[0] and col < binary_df_pi_i.shape[1]:  # Check bounds
                if highlighted:
                    binary_df_pi_i.iat[row, col] = 0
                else:
                    binary_df_pi_i.iat[row, col] = 1
                update_trigger_pi_i.set(update_trigger_pi_i.get() + 1)

    @reactive.Effect
    @reactive.event(input.offset_distance_incurred)
    async def update_highlight_default_cell_pi_i():
        offset = input.offset_distance_incurred()
        if offset == 0:
            await session.send_custom_message('highlight_cells', 0)
        else:
            await session.send_custom_message('highlight_cells', offset)

    @reactive.Effect
    @reactive.event(lambda: input.all_generated_cells_ratios_table_2())
    def update_all_generated_cells_pi_i():
        cells = input.all_generated_cells_ratios_table_2()
        if cells:
            for cell in cells:
                row, col, highlighted = cell['row'] - 1, cell['col'], cell['highlighted']
                if row < binary_df_pi_i.shape[0] and col < binary_df_pi_i.shape[1]:  # Sprawdzenie zakresów
                    if highlighted:
                        binary_df_pi_i.iat[row, col] = 0
                    else:
                        binary_df_pi_i.iat[row, col] = 1
            update_trigger_pi_i.set(update_trigger_pi_i.get() + 1)

    @output
    @render.ui
    def ratios_table_ui_pi_i():
        update_global_variables_pi_i(linia_biznesowa.get())
        return ui.HTML(
            ratio_df_pi_i.to_html(classes='table table-striped table-hover', table_id="ratios-table-2")
        ), ui.tags.script("highlight_default_cell_incurred(" + str(input.offset_distance_incurred()) + ");")

    @output
    @render.ui
    def binary_ratios_table_ui_pi_i():
        update_trigger_pi_i.get()
        df = binary_df_pi_i.copy()
        return ui.HTML(
            df.to_html(classes='table table-striped table-hover', table_id="binary-ratios-table-2", na_rep='NaN',
                       float_format='{:.0f}'.format))


    @reactive.event(input.clicked_cell_ratios_table_2)
    def wspolczynniki_multiplikatywna_interaktywna_i_cl():
        trian_list = every_triangle()[2]
        trian_for_line = trian_list[index_lini_CL_Incurred()]
        triagnle = trian_for_line.iloc[:, 1:]
        binary_df_deterministic = create_binary_df(triagnle)
        ind_all, m_i, m_first = yh.index_all(triagnle)
        macierz_wsp_l = yh.l_i_j(triagnle, ind_all)
        Dev_j_deterministic = yh.Dev(triagnle, binary_df_deterministic, macierz_wsp_l, ind_all)
        Dev_j = yh.Dev(triagnle, binary_df_pi_i, macierz_wsp_l, ind_all)
       # sigma_j = yh.sigma(triagnle, binary_df_pi_i, macierz_wsp_l, Dev_j, ind_all)
        #sd_j = yh.wspolczynnik_sd(triagnle, binary_df_pi_i, sigma_j, ind_all)
        #sd_pd = [x ** 2 for x in sd_j]
        I_dataframe = pd.DataFrame(index=['CL_base', 'CL'],
                                   columns=["Wybór"] + ['dp: ' + str(j) for j in range(1, len(Dev_j) + 1)])
        I_dataframe.iloc[0, :] = ["CL_base"] + Dev_j_deterministic
        I_dataframe.iloc[1, :] = ["CL"] + Dev_j
        return (I_dataframe)

    @output
    @render.data_frame
    def macierz_wspol_CL_interaktywna_i_cl():
        df_out_mult= wspolczynniki_multiplikatywna_interaktywna_i_cl()
        return render.DataGrid(
            df_out_mult,
            width="100%",
            height="150%",
        )

    @reactive.Calc
    @reactive.event(input.accept_CL_i_cl, ignore_none=False)
    def wsp_mult_paid_i_cl():
        ys = []
        xs = []
        y = wspolczynniki_multiplikatywna_interaktywna_i_cl().iloc[1, 1:].tolist()
        if (input.dop_all_factor_i_cl() == True):
            ii = 0
            for k in range(1, len(y) + 1):
                ii = ii + 1
                if (y[k - 1] > 1):
                    ys.append(y[k - 1])
                    xs.append(ii)
        elif (input.dop_all_factor_i_cl() == False):
            for chose_factor in input.chose_CL_i_cl():
                ys.append(y[int(chose_factor) - 1])
                xs.append(int(chose_factor))
        return ([np.array(xs, dtype=np.float64), np.array(ys, dtype=np.float64)])


    @reactive.Calc
    @reactive.event(input.accept_CL_i_cl, ignore_none=False)
    def dopasowanie_krzywej_factor_interaktywne_i_cl():
        ys = wsp_mult_paid_i_cl()[1]
        xs = wsp_mult_paid_i_cl()[0]
        list_curve = ['Exponential', "Weibull", "Power", "Inverse Power"]
        parameters_curve = yh.parameters_curve_rezerwy(xs, ys, list_curve, "Paid")

        return (parameters_curve)

    @reactive.Calc
    # @reactive.event(input.accept_CL, ignore_none=False)
    def predykcja_krzywej_factor_interaktywne_i_cl():
        parameters_curve = dopasowanie_krzywej_factor_interaktywne_i_cl()
        list_curve = ['Exponential', "Weibull", "Power", "Inverse Power"]
        xs_pred = np.array(np.arange(1, len(wspolczynniki_multiplikatywna_interaktywna_i_cl().iloc[1, 1:]) + 1))
        f_curves_graph_real = yh.sim_data_curve_rezerwy(xs_pred, list_curve, parameters_curve)
        f_curves_graph_real.insert(0, "Krzywa", list_curve)
        return (f_curves_graph_real)

    def change_ind_to_col(df_data, name_col):
        if (name_col in df_data.columns.to_list()):
            df_data = df_data.copy()
        else:
            df_data.insert(0, name_col, df_data.index)
        return (df_data)

    @reactive.Calc
    # @reactive.event(input.accept_CL, ignore_none=False)
    def r2_df_freq_lc_interaktywny_i_cl():
        ys = wsp_mult_paid_i_cl()[1]
        xs = wsp_mult_paid_i_cl()[0]
        list_curve = ['Exponential', "Weibull", "Power", "Inverse Power"]
        parameters_curve = dopasowanie_krzywej_factor_interaktywne()
        f_curves_graph_real_choose = yh.sim_data_curve_rezerwy(xs, list_curve, parameters_curve)
        r2_curves_df = yh.r2_curves(f_curves_graph_real_choose, ys)
        return (r2_curves_df)

   #     ys = np.array(wspolczynniki_multiplikatywna_interaktywna_i_cl().iloc[1, 1:], dtype=np.float64)
    #    f_curves_to_r2 = predykcja_krzywej_factor_interaktywne_i_cl().iloc[:, 1:]
     #   r2_curves_df = yh.r2_curves(f_curves_to_r2, ys)
     #   return (r2_curves_df)

    @output
    @render.data_frame
    def wspol_z_krzywej_CL_paid_interaktywna_i_cl():
        II_dataframe = predykcja_krzywej_factor_interaktywne_i_cl()
        return render.DataGrid(
            II_dataframe,
            width="100%",
            height="100%",
            row_selection_mode='single'
        )

    @output
    @render.data_frame
    def r2_cl_paid_i_cl():
        r2_freq_lc = change_ind_to_col(r2_df_freq_lc_interaktywny_i_cl(), 'Wartość')
        return render.DataGrid(
            r2_freq_lc,
            width="100%",
            height="100%",
            summary=False,
        )

    @output
    @render.plot()
    def plot_wspolczynniki_dopasowane_interaktywny_i_cl():
        factor_paid = wspolczynniki_multiplikatywna_interaktywna_i_cl().iloc[1, 1:]
        xs_input = np.arange(1, len(factor_paid) + 1)
        f_curves_tail_graph = predykcja_krzywej_factor_interaktywne_i_cl().iloc[:, 1:]
        fig = plt.figure()
        for index_loop, row in f_curves_tail_graph.iterrows():
            plt.plot(row, '-', label=str(index_loop))
        plt.plot(factor_paid, 'o-', linewidth=1, label='Initial Selection')
        plt.legend()
        fig.autofmt_xdate()
        return fig

    @output
    @render.data_frame
    def Factors_curve_and_initial_i_cl():
        f_curves_tail_graph_cop = predykcja_krzywej_factor_interaktywne_i_cl()
        f_curves_tail_graph_cop.loc[len(f_curves_tail_graph_cop)] = [
                                                                        'Initial'] + wspolczynniki_multiplikatywna_interaktywna_i_cl().iloc[
                                                                                     1, 1:].tolist()
        return render.DataGrid(
            f_curves_tail_graph_cop,
            width="100%",
            height="100%",
            summary=False,
        )

    def predykcja_krzywej_factor_interaktywne_tail_i_cl():
        parameters_curve = dopasowanie_krzywej_factor_interaktywne_i_cl()
        list_curve = ['Exponential', "Weibull", "Power", "Inverse Power"]
        xs_pred = np.array(
            np.arange(1, len(wspolczynniki_multiplikatywna_interaktywna_i_cl().iloc[1, 1:]) + 1 + int(input.num_tail_i_cl())))
        f_curves_graph_real = yh.sim_data_curve_rezerwy(xs_pred, list_curve, parameters_curve)
        f_curves_graph_real.insert(0, "Krzywa", list_curve)
        return (f_curves_graph_real)

    @reactive.Calc
    @reactive.event(input.accept_final_factor_i_cl, ignore_none=False)
    def calc_Final_Factor_i_cl():
        df_all_trin_factor = predykcja_krzywej_factor_interaktywne_tail_i_cl()
        fact_real = wspolczynniki_multiplikatywna_interaktywna_i_cl().iloc[1, 1:].tolist()
        Final_factors = yh.chose_factor_indywidualny(df_all_trin_factor, fact_real, input.num_of_first_factor_i_cl())
        return (Final_factors)




    @output
    @render.data_frame
    def Final_Factor_i_cl():
        Final_factors = calc_Final_Factor_i_cl()
        return render.DataGrid(
            Final_factors,
            width="100%",
            height="100%",
            summary=False,
        )

    @output
    @render.data_frame
    def Choose_Factor_i_cl():
        Final_factors = calc_Final_Factor_i_cl()
        Choose_factors = pd.DataFrame(columns=['Choose'] + ['dp: ' + str(x) for x in
                                                               range(1, Final_factors.shape[1])],
                                      index=["Wybrana krzywa"])
        Choose_factors.iloc[0, :] = Final_factors.loc[input.wyb_krzywa_ogona_i_cl()].to_list()
        return render.DataGrid(
            Choose_factors,
            width="100%",
            height="100%",
            summary=False,
        )

    @reactive.Calc
    def calc_chainladder_interaktywne_i_cl():
        Final_factors = calc_Final_Factor_i_cl()
        trian_list = every_triangle()[2]
        triangle = trian_list[index_lini_CL_Paid()].iloc[:, 1:]
        CL_fit = Final_factors.loc[input.wyb_krzywa_ogona_i_cl()].to_list()
        Dev_j_base = wspolczynniki_multiplikatywna_interaktywna_i_cl().iloc[0, 1:].to_list()
        Dev_j_z_wagami = wspolczynniki_multiplikatywna_interaktywna_i_cl().iloc[1, 1:].to_list()
        data_output = pd.DataFrame(0, index=np.arange(0, triangle.shape[0] + 1),
                                   columns=['Rok/Suma', 'Ult_base', 'IBNR_base', 'Ult z wagami', 'IBNR z wagami',
                                            'Ult z krzywą', 'IBNR z krzywą'])
        data_output.iloc[:, 0] = trian_list[index_lini_CL_Paid()].iloc[:, 0].tolist() + ["Suma"]
        k = 1

        for wspolczynniki in [Dev_j_base, Dev_j_z_wagami, CL_fit[1:]]:
            proj_triangle = yh.triangle_forward(triangle, wspolczynniki, 0)
            diag = yh.reverse_list(yh.trian_diag(triangle))
            Ultimate_Param_ReservingRisk = proj_triangle.iloc[:, int(proj_triangle.shape[1]) - 1].to_list()
            data_output.iloc[:, k] = Ultimate_Param_ReservingRisk + [np.sum(Ultimate_Param_ReservingRisk)]
            k = k + 1
            BE_Param_ReservingRisk = [x - y for x, y in zip(Ultimate_Param_ReservingRisk, diag)]
            data_output.iloc[:, k] = BE_Param_ReservingRisk + [np.sum(BE_Param_ReservingRisk)]
            k = k + 1
        return (data_output)



    @output
    @render.data_frame
    def Ult_BE_data_interaktywne_i_cl():
        df = calc_chainladder_interaktywne_i_cl()
        df.iloc[:,1:] = df.iloc[:,1:].map(lambda x: f"{x:,.2f}".replace(",", " ") if isinstance(x, (int, float)) else x)
        return render.DataGrid(
            df,
            width="100%",
            height="150%",
        )

            ##########################################   KONIEC Incurred CL

    ###################################### START PAID LR

    @output
    @render.ui
    def triangle_table_p_lr():
        trian_list = every_triangle()[0]
        trian_for_line = trian_list[index_lini_LR_Paid()]
        index_linii = index_lini_LR_Paid()
        update_global_variables_p_lr(index_linii)
        df_copy = trian_for_line.copy()
        df_copy.iloc[:, 1:] = df_copy.iloc[:, 1:].map(
            lambda x: f"{x:,.2f}".replace(",", " ") if isinstance(x, (int, float)) else x)
        return ui.HTML(
            df_copy.to_html(classes='table table-striped table-hover'))

    def update_global_variables_p_lr(ind_linii):
        global ratio_df_p_lr, binary_df_p_lr
        trian_list = every_triangle()[0]
        exponsure = every_triangle()[3][0]
        trian_for_line = trian_list[ind_linii]
       # exponsure = yh.reverse_list(exponsure)
        ratio_df_p_lr = calculate_ratios_p_lr(trian_for_line,exponsure)
        binary_df_p_lr = create_binary_df(ratio_df_p_lr)



    @output
    @render.ui
    def ratios_table_ui_p_lr():
        update_global_variables_p_lr(index_lini_LR_Paid())
        return ui.HTML(
            ratio_df_p_lr.to_html(classes='table table-striped table-hover', table_id="ratios-table-4")
        ), ui.tags.script("highlight_default_cell_p_lr(" + str(input.offset_distance_p_lr()) + ");")



    @reactive.Effect
    @reactive.event(lambda: input.clicked_cell_ratios_table_4())
    def update_clicked_cell_p_lr():
        cell = input.clicked_cell_ratios_table_4()
        if cell:
            row, col, highlighted = cell['row'], cell['col'] - 1, cell['highlighted']
            if row < binary_df_p_lr.shape[0] and col < binary_df_p_lr.shape[1]:  # Check bounds
                if highlighted:
                    binary_df_p_lr.iat[row, col] = 0
                else:
                    binary_df_p_lr.iat[row, col] = 1
                update_trigger_p_lr.set(update_trigger_p_lr.get() + 1)

    @reactive.Effect
    @reactive.event(input.offset_distance_p_lr)
    async def update_highlight_default_cell_p_lr():
        offset = input.offset_distance_p_lr()
        if offset == 0:
            await session.send_custom_message('highlight_cells_p_lr', 0)
        else:
            await session.send_custom_message('highlight_cells_p_lr', offset)

    @reactive.Effect
    @reactive.event(lambda: input.all_generated_cells_ratios_table_4())
    def update_generated_cells_p_lr():
        cells = input.all_generated_cells_ratios_table_4()
        if cells:
            for cell in cells:
                row, col, highlighted = cell['row'], cell['col'], cell['highlighted']
                if row < binary_df_p_lr.shape[0] and col < binary_df_p_lr.shape[1]:  # Sprawdzenie zakresów
                    if highlighted:
                        binary_df_p_lr.iat[row, col] = 0
                    else:
                        binary_df_p_lr.iat[row, col] = 1
            update_trigger_p_lr.set(update_trigger_p_lr.get() + 1)

    @output
    @render.ui
    def binary_ratios_table_ui_p_lr():
        update_trigger_p_lr.get()
        df = binary_df_p_lr.copy()
        return ui.HTML(
            df.to_html(classes='table table-striped table-hover', table_id="binary-ratios-table-4", na_rep='NaN',
                       float_format='{:.0f}'.format))


    @reactive.event(input.clicked_cell_ratios_table_4)
    def wspolczynniki_multiplikatywna_interaktywna_p_lr():
        trian_list = every_triangle()[0]
        trian_for_line = trian_list[index_lini_CL_Paid()]
        triagnle = trian_for_line.iloc[:,1:]
        binary_df_deterministic = create_binary_df(triagnle)
        ind_all, m_i, m_first = yh.index_all(triagnle)
        expons = every_triangle()[3][0]
        macierz_wsp_lr = yh.wspolczynniki_LR_i_j(triagnle, expons)
        LR_j = yh.wspolczynnik_LR(macierz_wsp_lr, binary_df_deterministic, expons)
        LR_j_wagi = yh.wspolczynnik_LR(macierz_wsp_lr, binary_df_p_lr, expons)
        sigma_j_LRMPC = yh.sigma_LRMPC(expons, binary_df_p_lr, macierz_wsp_lr, LR_j_wagi)
        sd_LRMPC = yh.wspolczynnik_sd_LRMPC(sigma_j_LRMPC, binary_df_p_lr, expons)
        #Dev_j_deterministic = yh.Dev(triagnle, binary_df_deterministic, macierz_wsp_l, ind_all)
       # Dev_j = yh.Dev(triagnle, binary_df_p, macierz_wsp_l, ind_all)
       # sigma_j = yh.sigma(triagnle, binary_df_p, macierz_wsp_l, Dev_j, ind_all)
       # sd_j = yh.wspolczynnik_sd(triagnle, binary_df_p, sigma_j, ind_all)
        sd_pd = [x**2 for x in sd_LRMPC]
        I_dataframe = pd.DataFrame(index=['LR_base','LR_j_wagi'],
                                   columns=["Wybór"]+['dp: ' + str(x) for x in range(1, len(LR_j) + 1)])
        I_dataframe.iloc[0, :] =  ["LR_base"]+LR_j
        I_dataframe.iloc[1, :] =  ["LR_j_wagi"]+LR_j_wagi

        return(I_dataframe)

    @output
    @render.data_frame
    def macierz_wspol_CL_interaktywna_p_lr():
        df = wspolczynniki_multiplikatywna_interaktywna_p_lr()
        return render.DataGrid(
            df,
            width="100%",
            height="150%",
        )

    @reactive.Calc
    @reactive.event(input.accept_p_lr, ignore_none=False)
    def wsp_mult_paid_p_lr():
        ys = []
        xs = []
        y = wspolczynniki_multiplikatywna_interaktywna_p_lr().iloc[1, 1:].to_list()
        if (input.dop_all_factor_p_lr() == True):
            ii = 0
            for k in range(1, len(y) + 1):
                ii = ii + 1
                if (y[k - 1] > 0):
                    ys.append(y[k - 1])
                    xs.append(ii)
        elif (input.dop_all_factor_p_lr() == False):
            for chose_factor in input.chose_CL_p_lr():
                ys.append(y[int(chose_factor) - 1])
                xs.append(int(chose_factor))
        return ([np.array(xs, dtype=np.float64), np.array(ys, dtype=np.float64)])

    @reactive.Calc
    @reactive.event(input.accept_p_lr, ignore_none=False)
    def dopasowanie_krzywej_factor_interaktywne_p_lr():
        ys = wsp_mult_paid_p_lr()[1]
        xs = wsp_mult_paid_p_lr()[0]
        list_curve = ['Exponential', "Weibull", "Power", "Inverse Power"]
        parameters_curve = yh.parameters_curve_rezerwy_lr(xs, ys, list_curve, "Paid")
        return (parameters_curve)

    @reactive.Calc
    @reactive.event(input.accept_p_lr, ignore_none=False)
    def predykcja_krzywej_factor_interaktywne_p_lr():
        parameters_curve = dopasowanie_krzywej_factor_interaktywne_p_lr()
        #"Power",
        list_curve = ['Exponential', "Power","Weibull",  "Inverse Power"]
        xs_pred = np.array(np.arange(1, len(wspolczynniki_multiplikatywna_interaktywna_p_lr().iloc[1, 1:]) + 1))
        f_curves_graph_real = yh.sim_data_curve_rezerwy_lr(xs_pred, list_curve, parameters_curve)
        f_curves_graph_real.insert(0, "Krzywa", list_curve)
        return (f_curves_graph_real)

    def change_ind_to_col(df_data, name_col):
        if (name_col in df_data.columns.to_list()):
            df_data = df_data.copy()
        else:
            df_data.insert(0, name_col, df_data.index)
        return (df_data)

    @output
    @render.data_frame
    def wspol_z_krzywej_CL_paid_interaktywna_p_lr():
        II_dataframe = predykcja_krzywej_factor_interaktywne_p_lr()
        return render.DataGrid(
            II_dataframe,
            width="100%",
            height="100%",
            row_selection_mode='single'
        )

    @output
    @render.plot()
    def plot_wspolczynniki_dopasowane_interaktywny_p_lr():
        factor_paid = wspolczynniki_multiplikatywna_interaktywna_p_lr().iloc[1, 1:]
        f_curves_tail_graph = predykcja_krzywej_factor_interaktywne_p_lr().iloc[:, 1:]
        fig = plt.figure()
        for index_loop, row in f_curves_tail_graph.iterrows():
            plt.plot(row, '-', label=str(index_loop))
        plt.plot(factor_paid, 'o-', linewidth=1, label='Initial Selection')
        plt.legend()
        fig.autofmt_xdate()
        return fig

    @reactive.Calc
    # @reactive.event(input.accept_CL, ignore_none=False)
    def r2_df_freq_lc_interaktywny_p_lr():
        ys = wsp_mult_paid_p_lr()[1]
        xs = wsp_mult_paid_p_lr()[0]
        list_curve = ['Exponential', "Weibull", "Power", "Inverse Power"]
        parameters_curve = dopasowanie_krzywej_factor_interaktywne_p_lr()
        f_curves_graph_real_choose = yh.sim_data_curve_rezerwy_lr(xs, list_curve, parameters_curve)
        r2_curves_df = yh.r2_curves_lr(f_curves_graph_real_choose, ys)
        return (r2_curves_df)

    @output
    @render.data_frame
    def r2_cl_paid_p_lr():
        r2_freq_lc = change_ind_to_col(r2_df_freq_lc_interaktywny_p_lr(), 'Wartość')
        return render.DataGrid(
            r2_freq_lc,
            width="100%",
            height="100%",
            summary=False,
        )
    @output
    @render.data_frame
    def Factors_curve_and_initial_p_lr():
        f_curves_tail_graph_cop = predykcja_krzywej_factor_interaktywne_p_lr()
        f_curves_tail_graph_cop.loc[len(f_curves_tail_graph_cop)] = ['Initial'] + wspolczynniki_multiplikatywna_interaktywna_p_lr().iloc[
                                                                                     1, 1:].tolist()
        return render.DataGrid(
            f_curves_tail_graph_cop,
            width="100%",
            height="100%",
            summary=False,
        )

    def predykcja_krzywej_factor_interaktywne_tail_p_lr():
        parameters_curve = dopasowanie_krzywej_factor_interaktywne_p_lr()
        list_curve = ['Exponential', "Weibull", "Power","Inverse Power"]
        xs_pred = np.array(
            np.arange(1, len(wspolczynniki_multiplikatywna_interaktywna_p_lr().iloc[1, 1:]) + 1 + int(input.num_tail_p_lr())))
        f_curves_graph_real = yh.sim_data_curve_rezerwy_lr(xs_pred, list_curve, parameters_curve)
        f_curves_graph_real.insert(0, "Krzywa", list_curve)
        return (f_curves_graph_real)

    @reactive.Calc
    @reactive.event(input.accept_final_factor_p_lr, ignore_none=False)
    def calc_Final_Factor_p_lr():
        df_all_trin_factor = predykcja_krzywej_factor_interaktywne_tail_p_lr()
        fact_real = wspolczynniki_multiplikatywna_interaktywna_p_lr().iloc[1, 1:].tolist()
        Final_factors = yh.chose_factor_indywidualny(df_all_trin_factor, fact_real, input.num_of_first_factor_p_lr())
        return (Final_factors)



    @output
    @render.data_frame
    def Final_Factor_p_lr_out():
        Final_factors = calc_Final_Factor_p_lr()
        return render.DataGrid(
            Final_factors,
            width="100%",
            height="100%",
            summary=False,
        )

    @output
    @render.data_frame
    def Factors_curve_and_initial_p_lr():
        f_curves_tail_graph_cop = predykcja_krzywej_factor_interaktywne_p_lr()
        f_curves_tail_graph_cop.loc[len(f_curves_tail_graph_cop)] = [
                                                                        'Initial'] + wspolczynniki_multiplikatywna_interaktywna_p_lr().iloc[
                                                                                     1, 1:].tolist()
        return render.DataGrid(
            f_curves_tail_graph_cop,
            width="100%",
            height="100%",
            summary=False,
        )

    @output
    @render.data_frame
    def Choose_Factor_p_lr():
        Final_factors = calc_Final_Factor_p_lr()
        Choose_factors = pd.DataFrame( columns=['Choose'] + ['dp: ' + str(x) for x in
                                                               range(1, Final_factors.shape[1])],
                                      index=["Wybrana krzywa"])
        Choose_factors.iloc[0, :] = Final_factors.loc[input.wyb_krzywa_ogona_p_lr()].to_list()
        return render.DataGrid(
            Choose_factors,
            width="100%",
            height="100%",
            summary=False,
        )

    @reactive.Calc
    def calc_chainladder_LR_P_interaktywna():
        exponsure = every_triangle()[3][0]
        trian_list = every_triangle()[0]
        trian_for_line = trian_list[index_lini_LR_paid()]
        triagnle = trian_for_line.iloc[:,1:]
        LR_factor = wspolczynniki_multiplikatywna_interaktywna_p_lr().iloc[1,1:]
        LR_factor_waga = wspolczynniki_multiplikatywna_interaktywna_p_lr().iloc[1, 1:]
        Final_factors = calc_Final_Factor_p_lr()
        LR_fit =Final_factors.loc[input.wyb_krzywa_ogona_p_lr()].to_list()[1:]
        data_output = pd.DataFrame(0, index=trian_for_line.iloc[:,0].tolist()+["Suma"],
                                   columns=['Rok/Suma', 'Ult_base', 'IBNR_base', 'Ult z wagami', 'IBNR z wagami','Ult z krzywą', 'IBNR z krzywą'])
        data_output.iloc[:, 0] = trian_for_line.iloc[:,0].tolist()+["Suma"]
        ind_licz = 1
        for wspolczynniki in [[LR_factor, LR_factor], [LR_factor_waga, LR_factor_waga], [LR_fit, LR_fit]]:
            proj_triangle = yh.triangle_forward_LR_CL(triagnle, wspolczynniki[0], wspolczynniki[1], exponsure,
                                                      0)
            diag = yh.reverse_list(yh.trian_diag(triagnle))
            Ultimate_Param_ReservingRisk = proj_triangle.iloc[:, int(proj_triangle.columns[-1]) - 1].to_list()
            data_output.iloc[:, ind_licz] = Ultimate_Param_ReservingRisk + [np.sum(Ultimate_Param_ReservingRisk)]
            ind_licz = ind_licz + 1
            BE_Param_ReservingRisk = [x - y for x, y in zip(Ultimate_Param_ReservingRisk, diag)]
            data_output.iloc[:, ind_licz] = BE_Param_ReservingRisk + [np.sum(BE_Param_ReservingRisk)]
            ind_licz = ind_licz + 1
        return (data_output)




    @output
    @render.data_frame
    def Ult_BE_data_interaktywne_p_lr():
        df = calc_chainladder_LR_P_interaktywna()
        df.iloc[:,1:] = df.iloc[:,1:].map(lambda x: f"{x:,.2f}".replace(",", " ") if isinstance(x, (int, float)) else x)
        return render.DataGrid(
            df,
            width="100%",
            height="150%",
        )


    ###################################### KONIEC PAID LR

    ###################################### START INCURRED LR

    @output
    @render.ui
    def triangle_table_i_lr():
        trian_list = every_triangle()[2]
        trian_for_line = trian_list[index_lini_LR_incurred()]
        index_linii = index_lini_LR_incurred()
        update_global_variables_i_lr(index_linii)
        # global  ratio_df_p,binary_df_p
        df_copy = trian_for_line.copy()
        df_copy.iloc[:, 1:] = df_copy.iloc[:, 1:].map(
            lambda x: f"{x:,.2f}".replace(",", " ") if isinstance(x, (int, float)) else x)
        return ui.HTML(
            df_copy.to_html(classes='table table-striped table-hover'))


    def update_global_variables_i_lr(ind_linii):
        global ratio_df_i_lr, binary_df_i_lr
        trian_list = every_triangle()[2]
        exponsure = every_triangle()[3][0]
        trian_for_line = trian_list[ind_linii]
       # exponsure = yh.reverse_list(exponsure)
      #  exponsure = yh.inflation_to_eksponsure_interaktywna(exponsure)
        ratio_df_i_lr = calculate_ratios_i_lr(trian_for_line,exponsure)
        binary_df_i_lr = create_binary_df(ratio_df_i_lr)

    @output
    @render.ui
    def ratios_table_ui_i_lr():
        update_global_variables_i_lr(index_lini_LR_Paid())
        return ui.HTML(
            ratio_df_i_lr.to_html(classes='table table-striped table-hover', table_id="ratios-table-5")
        ), ui.tags.script("highlight_default_cell_i_lr(" + str(input.offset_distance_i_lr()) + ");")

    @output
    @render.ui
    def binary_ratios_table_ui_i_lr():
        update_trigger_i_lr.get()
        df = binary_df_i_lr.copy()
        return ui.HTML(
            df.to_html(classes='table table-striped table-hover', table_id="binary-ratios-table-5", na_rep='NaN',
                       float_format='{:.0f}'.format))

    @reactive.Effect
    @reactive.event(lambda: input.clicked_cell_ratios_table_5())
    def update_clicked_cell_i_lr():
        cell = input.clicked_cell_ratios_table_5()
        if cell:
            row, col, highlighted = cell['row'], cell['col'] - 1, cell['highlighted']
            if row < binary_df_i_lr.shape[0] and col < binary_df_i_lr.shape[1]:  # Check bounds
                if highlighted:
                    binary_df_i_lr.iat[row, col] = 0
                else:
                    binary_df_i_lr.iat[row, col] = 1
                update_trigger_i_lr.set(update_trigger_i_lr.get() + 1)

    @reactive.Effect
    @reactive.event(input.offset_distance_i_lr)
    async def update_highlight_default_cell_i_lr():
        offset = input.offset_distance_i_lr()
        if offset == 0:
            await session.send_custom_message('highlight_cells_i_lr', 0)
        else:
            await session.send_custom_message('highlight_cells_i_lr', offset)

    @reactive.Effect
    @reactive.event(lambda: input.all_generated_cells_ratios_table_5())
    def update_generated_cells_i_lr():
        cells = input.all_generated_cells_ratios_table_5()
        if cells:
            for cell in cells:
                row, col, highlighted = cell['row'], cell['col'], cell['highlighted']
                if row < binary_df_i_lr.shape[0] and col < binary_df_i_lr.shape[1]:  # Sprawdzenie zakresów
                    if highlighted:
                        binary_df_i_lr.iat[row, col] = 0
                    else:
                        binary_df_i_lr.iat[row, col] = 1
            update_trigger_i_lr.set(update_trigger_i_lr.get() + 1)

    @reactive.event(input.clicked_cell_ratios_table_5)
    def wspolczynniki_multiplikatywna_interaktywna_i_lr():
        trian_list = every_triangle()[2]
        trian_for_line = trian_list[index_lini_LR_incurred()]
        triagnle = trian_for_line.iloc[:,1:]
        binary_df_deterministic = create_binary_df(triagnle)
        expons = every_triangle()[3][0]
        macierz_wsp_lr = yh.wspolczynniki_LR_i_j(triagnle, expons)
        LR_j = yh.wspolczynnik_LR(macierz_wsp_lr, binary_df_deterministic, expons)
        LR_j_wagi = yh.wspolczynnik_LR(macierz_wsp_lr, binary_df_i_lr, expons)
        sigma_j_LRMPC = yh.sigma_LRMPC(expons, binary_df_i_lr, macierz_wsp_lr, LR_j_wagi)
        sd_LRMPC = yh.wspolczynnik_sd_LRMPC(sigma_j_LRMPC, binary_df_i_lr, expons)
        #Dev_j_deterministic = yh.Dev(triagnle, binary_df_deterministic, macierz_wsp_l, ind_all)
       # Dev_j = yh.Dev(triagnle, binary_df_p, macierz_wsp_l, ind_all)
       # sigma_j = yh.sigma(triagnle, binary_df_p, macierz_wsp_l, Dev_j, ind_all)
       # sd_j = yh.wspolczynnik_sd(triagnle, binary_df_p, sigma_j, ind_all)
        sd_pd = [x**2 for x in sd_LRMPC]
        I_dataframe = pd.DataFrame(index=['LR_base','LR_j_wagi'],
                                   columns=["Wybór"]+['dp: ' + str(x) for x in range(1, len(LR_j) + 1)])
        I_dataframe.iloc[0, :] =  ["LR_base"]+LR_j
        I_dataframe.iloc[1, :] =  ["LR_j_wagi"]+LR_j_wagi

        return(I_dataframe)

    @output
    @render.data_frame
    def macierz_wspol_CL_interaktywna_i_lr():
        df = wspolczynniki_multiplikatywna_interaktywna_i_lr()
        return render.DataGrid(
            df,
            width="100%",
            height="150%",
        )

    @reactive.Calc
    @reactive.event(input.accept_i_lr, ignore_none=False)
    def wsp_mult_paid_i_lr():
        ys = []
        xs = []
        y = wspolczynniki_multiplikatywna_interaktywna_i_lr().iloc[1, 1:].tolist()
        if (input.dop_all_factor_i_lr() == True):
            ii = 0
            for k in range(1, len(y) + 1):
                ii = ii + 1
                if (y[k - 1] > 0):
                    ys.append(y[k - 1])
                    xs.append(ii)
        elif (input.dop_all_factor_i_lr() == False):
            for chose_factor in input.chose_CL_i_lr():
                ys.append(y[int(chose_factor) - 1])
                xs.append(int(chose_factor))
        return ([np.array(xs, dtype=np.float64), np.array(ys, dtype=np.float64)])

    @reactive.Calc
    @reactive.event(input.accept_i_lr, ignore_none=False)
    def dopasowanie_krzywej_factor_interaktywne_i_lr():
        ys = wsp_mult_paid_i_lr()[1]
        xs = wsp_mult_paid_i_lr()[0]
        list_curve = ['Exponential', "Weibull", "Power", "Inverse Power"]
        parameters_curve = yh.parameters_curve_rezerwy_lr(xs, ys, list_curve, "Paid")
        return (parameters_curve)

    @reactive.Calc
    @reactive.event(input.accept_i_lr, ignore_none=False)
    def predykcja_krzywej_factor_interaktywne_i_lr():
        parameters_curve = dopasowanie_krzywej_factor_interaktywne_i_lr()
        #"Power",
        list_curve = ['Exponential', "Weibull",  "Inverse Power"]
        xs_pred = np.array(np.arange(1, len(wspolczynniki_multiplikatywna_interaktywna_i_lr().iloc[1, 1:]) + 1))
        f_curves_graph_real = yh.sim_data_curve_rezerwy_lr(xs_pred, list_curve, parameters_curve)
        f_curves_graph_real.insert(0, "Krzywa", list_curve)
        return (f_curves_graph_real)

    def change_ind_to_col(df_data, name_col):
        if (name_col in df_data.columns.to_list()):
            df_data = df_data.copy()
        else:
            df_data.insert(0, name_col, df_data.index)
        return (df_data)

    @output
    @render.data_frame
    def wspol_z_krzywej_CL_paid_interaktywna_i_lr():
        II_dataframe = predykcja_krzywej_factor_interaktywne_i_lr()
        return render.DataGrid(
            II_dataframe,
            width="100%",
            height="100%",
            row_selection_mode='single'
        )

    @output
    @render.plot()
    def plot_wspolczynniki_dopasowane_interaktywny_i_lr():
        factor_paid = wspolczynniki_multiplikatywna_interaktywna_i_lr().iloc[1, 1:]
        f_curves_tail_graph = predykcja_krzywej_factor_interaktywne_i_lr().iloc[:, 1:]
        fig = plt.figure()
        for index_loop, row in f_curves_tail_graph.iterrows():
            plt.plot(row, '-', label=str(index_loop))
        plt.plot(factor_paid, 'o-', linewidth=1, label='Initial Selection')
        plt.legend()
        fig.autofmt_xdate()
        return fig

    @reactive.Calc
    # @reactive.event(input.accept_CL, ignore_none=False)
    def r2_df_freq_lc_interaktywny_i_lr():
        ys = wsp_mult_paid_i_lr()[1]
        xs = wsp_mult_paid_i_lr()[0]
        list_curve = ['Exponential', "Weibull", "Power", "Inverse Power"]
        parameters_curve = dopasowanie_krzywej_factor_interaktywne_i_lr()
        f_curves_graph_real_choose = yh.sim_data_curve_rezerwy_lr(xs, list_curve, parameters_curve)
        r2_curves_df = yh.r2_curves_lr(f_curves_graph_real_choose, ys)
        return (r2_curves_df)

    @output
    @render.data_frame
    def r2_cl_paid_i_lr():
        r2_freq_lc = change_ind_to_col(r2_df_freq_lc_interaktywny_i_lr(), 'Wartość')
        return render.DataGrid(
            r2_freq_lc,
            width="100%",
            height="100%",
            summary=False,
        )
    @output
    @render.data_frame
    def Factors_curve_and_initial_i_lr():
        f_curves_tail_graph_cop = predykcja_krzywej_factor_interaktywne_i_lr()
        f_curves_tail_graph_cop.loc[len(f_curves_tail_graph_cop)] = ['Initial'] + wspolczynniki_multiplikatywna_interaktywna_i_lr().iloc[
                                                                                     1, 1:].tolist()
        return render.DataGrid(
            f_curves_tail_graph_cop,
            width="100%",
            height="100%",
            summary=False,
        )

    def predykcja_krzywej_factor_interaktywne_tail_i_lr():
        parameters_curve = dopasowanie_krzywej_factor_interaktywne_i_lr()
        list_curve = ['Exponential', "Weibull","Power", "Inverse Power"]
        xs_pred = np.array(
            np.arange(1, len(wspolczynniki_multiplikatywna_interaktywna_i_lr().iloc[1, 1:]) + 1 + int(input.num_tail_i_lr())))
        f_curves_graph_real = yh.sim_data_curve_rezerwy_lr(xs_pred, list_curve, parameters_curve)
        f_curves_graph_real.insert(0, "Krzywa", list_curve)
        return (f_curves_graph_real)

    @reactive.Calc
    @reactive.event(input.accept_final_factor_i_lr, ignore_none=False)
    def calc_Final_Factor_i_lr():
        df_all_trin_factor = predykcja_krzywej_factor_interaktywne_tail_i_lr()
        fact_real = wspolczynniki_multiplikatywna_interaktywna_i_lr().iloc[1, 1:].tolist()
        Final_factors = yh.chose_factor_indywidualny(df_all_trin_factor, fact_real, input.num_of_first_factor_i_lr())
        return (Final_factors)

    @output
    @render.data_frame
    def Final_Factor_i_lr_out():
        Final_factors = calc_Final_Factor_i_lr()
        return render.DataGrid(
            Final_factors,
            width="100%",
            height="100%",
            summary=False,
        )

    @output
    @render.data_frame
    def Factors_curve_and_initial_i_lr():
        f_curves_tail_graph_cop = predykcja_krzywej_factor_interaktywne_i_lr()
        f_curves_tail_graph_cop.loc[len(f_curves_tail_graph_cop)] = ['Initial'] + wspolczynniki_multiplikatywna_interaktywna_i_lr().iloc[1, 1:].tolist()
        return render.DataGrid(
            f_curves_tail_graph_cop,
            width="100%",
            height="100%",
            summary=False,
        )

    @output
    @render.data_frame
    def Choose_Factor_i_lr():
        Final_factors = calc_Final_Factor_i_lr()
        Choose_factors = pd.DataFrame( columns=['Choose'] + ['dp: ' + str(x) for x in range(1, Final_factors.shape[1])],
                                      index=["Wybrana krzywa"])
        Choose_factors.iloc[0, :] = Final_factors.loc[input.wyb_krzywa_ogona_i_lr()].to_list()
        return render.DataGrid(
            Choose_factors,
            width="100%",
            height="100%",
            summary=False,
        )

    @reactive.Calc
    def calc_chainladder_LR_I_interaktywna():
        exponsure = every_triangle()[3][0]
        trian_list = every_triangle()[2]
        trian_for_line = trian_list[index_lini_LR_incurred()]
        triagnle = trian_for_line.iloc[:,1:]
        LR_factor = wspolczynniki_multiplikatywna_interaktywna_i_lr().iloc[1,1:]
        LR_factor_waga = wspolczynniki_multiplikatywna_interaktywna_i_lr().iloc[1, 1:]
        Final_factors = calc_Final_Factor_i_lr()
        LR_fit =Final_factors.loc[input.wyb_krzywa_ogona_i_lr()].to_list()[1:]
        data_output = pd.DataFrame(0, index=trian_for_line.iloc[:,0].tolist()+["Suma"],
                                   columns=['Rok/Suma', 'Ult_base', 'IBNR_base', 'Ult z wagami', 'IBNR z wagami','Ult z krzywą', 'IBNR z krzywą'])
        data_output.iloc[:, 0] = trian_for_line.iloc[:,0].tolist()+["Suma"]
        ind_licz = 1
        for wspolczynniki in [[LR_factor, LR_factor], [LR_factor_waga, LR_factor_waga], [LR_fit, LR_fit]]:
            proj_triangle = yh.triangle_forward_LR_CL(triagnle, wspolczynniki[0], wspolczynniki[1], exponsure,
                                                      0)
            diag = yh.reverse_list(yh.trian_diag(triagnle))
            Ultimate_Param_ReservingRisk = proj_triangle.iloc[:, int(proj_triangle.shape[1]) - 1].to_list()
            data_output.iloc[:, ind_licz] = Ultimate_Param_ReservingRisk + [np.sum(Ultimate_Param_ReservingRisk)]
            ind_licz = ind_licz + 1
            BE_Param_ReservingRisk = [x - y for x, y in zip(Ultimate_Param_ReservingRisk, diag)]
            data_output.iloc[:, ind_licz] = BE_Param_ReservingRisk + [np.sum(BE_Param_ReservingRisk)]
            ind_licz = ind_licz + 1
        return (data_output)


    @output
    @render.data_frame
    def Ult_BE_data_interaktywne_i_lr():
        df = calc_chainladder_LR_I_interaktywna()
        df.iloc[:,1:] = df.iloc[:,1:].map(lambda x: f"{x:,.2f}".replace(",", " ") if isinstance(x, (int, float)) else x)
        return render.DataGrid(
            df,
            width="100%",
            height="150%",
        )

    @reactive.Effect
    # @reactive.event(lambda: input.clicked_cell_ratios_table_2())
    def sava_table_p_cl():
        if input.save_button_save_p_cl():
            save_path = os.path.join(input.folder_path_save_p_cl(), input.file_name_save_p_cl())
            df = calc_chainladder_interaktywne()
            trian_list = every_triangle()[0]
            triangle = trian_list[index_lini_CL_Paid()].iloc[:, 0].tolist()
            df.iloc[:, 0] = triangle + ["Suma"]
            df.to_excel(save_path)
        if input.save_button_save_i_cl():
            save_path = os.path.join(input.folder_path_save_i_cl(), input.file_name_save_i_cl())
            df = calc_chainladder_interaktywne_i_cl()
            trian_list = every_triangle()[0]
            triangle = trian_list[index_lini_CL_Paid()].iloc[:, 0].tolist()
            df.iloc[:, 0] = triangle + ["Suma"]
            df.to_excel(save_path)
        if input.save_button_save_p_lr():
            save_path = os.path.join(input.folder_path_save_p_lr(), input.file_name_save_p_lr())
            df = calc_chainladder_LR_P_interaktywna()
            trian_list = every_triangle()[0]
            triangle = trian_list[index_lini_CL_Paid()].iloc[:, 0].tolist()
            df.iloc[:, 0] = triangle + ["Suma"]
            df.to_excel(save_path)
        if input.save_button_save_i_lr():
            save_path = os.path.join(input.folder_path_save_i_lr(), input.file_name_save_i_lr())
            df = calc_chainladder_LR_I_interaktywna()
            trian_list = every_triangle()[0]
            triangle = trian_list[index_lini_CL_Paid()].iloc[:, 0].tolist()
            df.iloc[:, 0] = triangle + ["Suma"]
            df.to_excel(save_path)


    #####  INCURRED LR STOP

    #####  PAID LR START

#    @output
#    @render.data_frame
#    def macierz_wspol_CL_interaktywna_p_lr():
#        wspolczynniki_matrix = wspolczynniki_multiplikatywna_interaktywna_p_lr()
#        return render.DataGrid(
#            wspolczynniki_matrix,
#            width="100%",
#            height="100%",
#            row_selection_mode = 'single')




    #### KONIEC PAID LR



            ### dla paid/inccured



    @output
    @render.table
    def triangle_table_pi_p():
        trian_list = every_triangle()[0]
        trian_for_line = trian_list[index_lini_CL_Incurred()]
        index_linii = index_lini_CL_Incurred()
        update_global_variables_pi_pi(index_linii)
        # global  ratio_df_p,binary_df_p
        return trian_for_line

    def update_global_variables_pi_pi(ind_linii):
        global ratio_df_pi_pi, binary_df_pi_pi
        trian_list_paid = every_triangle()[0]
        trian_for_line_paid = trian_list_paid[ind_linii]
        trian_list_inccured = every_triangle()[2]
        trian_for_line_incurred = trian_list_inccured[ind_linii]
        ratio_df_pi_pi = calculate_ratios_pi(trian_for_line_paid,trian_for_line_incurred).iloc[:,1:]
        binary_df_pi_pi = create_binary_df(ratio_df_pi_pi)
        #trian_for_line_paid.to_excel("triangle_paid.xlsx", index=False, engine='openpyxl')
        #trian_for_line_incurred.to_excel("triangle_inccured.xlsx", index=False, engine='openpyxl')
 #   @output
 #   @render.ui
 ##   def ratios_table_ui_pi_pi():
  #      update_global_variables_pi_pi(index_lini_CL_Incurred())
  #      return ui.HTML(ratio_df_pi_pi.to_html(classes='table table-striped table-hover', table_id="ratios-table-3"))

    #@output
    #@render.ui
    #def binary_ratios_table_ui_pi_pi():
    #    update_trigger_pi_pi.get()
    #    df = binary_df_pi_pi.copy()
    #    return ui.HTML(
    #        df.to_html(classes='table table-striped table-hover', table_id="binary-ratios-table-3",
    #                    na_rep='NaN',
    #                    float_format='{:.0f}'.format))

    #@reactive.Effect
    #@reactive.event(input.clicked_cell_ratios_table_3)
    #def update_clicked_cell_pi_i():
    #    cell = input.clicked_cell_ratios_table_3()
    #    # print(f"Cell clicked in ratios table for P: {cell}")  # Debug print
    #    if cell:
    #        row, col, highlighted = cell['row'], cell['col'], cell['highlighted']
    #        if highlighted:
    #            binary_df_pi_pi.iat[row, col] = 0
    #        else:
    #            binary_df_pi_pi.iat[row, col] = 1
    #        update_trigger_pi_pi.set(update_trigger_pi_pi.get() + 1)




    @reactive.event(input.clicked_cell_ratios_table_3)
    def wspolczynniki_multiplikatywna_incurred_interaktywna():
        trian_list = every_triangle()[2]
        trian_for_line = trian_list[index_lini_CL_Incurred()]
        triagnle = trian_for_line.iloc[:,1:]
        binary_df_deterministic = create_binary_df(triagnle)
        ind_all, m_i, m_first = yh.index_all(triagnle)
        macierz_wsp_l = yh.l_i_j(triagnle, ind_all)
        Dev_j_deterministic = yh.Dev(triagnle, binary_df_deterministic, macierz_wsp_l, ind_all)
        Dev_j = yh.Dev(triagnle, binary_df_pi_i, macierz_wsp_l, ind_all)
        sigma_j = yh.sigma(triagnle, binary_df_pi_i, macierz_wsp_l, Dev_j, ind_all)
        sd_j = yh.wspolczynnik_sd(triagnle, binary_df_pi_i, sigma_j, ind_all)
        sd_pd = [x ** 2 for x in sd_j]
        trian_list_paid = every_triangle()[0]
        trian_for_line_paid = trian_list_paid[index_lini_CL_Incurred()]
        triagnle_paid = trian_for_line_paid.iloc[:,1:]
        binary_df_deterministic = create_binary_df(triagnle_paid)
        r_j_base = yh.wspolczynnik_r(triagnle_paid,triagnle,binary_df_deterministic)
        r_j_waga = yh.wspolczynnik_r(triagnle_paid,triagnle,binary_df_pi_pi)

        r_i_j = yh.wspolczynnik_r_i_j(triagnle_paid, triagnle)
        varj = yh.wspolczynnik_var_j(triagnle, r_i_j, r_j_waga, binary_df_pi_pi)
#binary_df_pi_pi
        I_dataframe = pd.DataFrame(index=['CL_base','CL wagi','r_j_base','r_j_waga','sigma','sd'],
                                   columns=[str(x) for x in range(1, len(Dev_j) + 3)])
        I_dataframe.iloc[0, :] =  ["CL_base"]+Dev_j_deterministic+ [np.nan]
        I_dataframe.iloc[1, :] =  ["CL"]+Dev_j+ [np.nan]
        I_dataframe.iloc[2, :] =  ["r_j_base"]+r_j_base
        I_dataframe.iloc[3, :] =  ["r_j_waga"]+r_j_waga
        I_dataframe.iloc[4, :] =  ["sigma"]+varj
        I_dataframe.iloc[5, :] =  ["sd"]+sd_pd+ [np.nan]
        return(I_dataframe)


    @output
    @render.data_frame
    def macierz_wspol_CL_interaktywna_pi_i():
        wspolczynniki_matrix = wspolczynniki_multiplikatywna_incurred_interaktywna()
        return render.DataGrid(
            wspolczynniki_matrix,
            width="100%",
            height="100%",
            row_selection_mode = 'single')



    @reactive.Calc
    @reactive.event(input.accept_CL_incurred, ignore_none=False)
    def dopasowanie_krzywej_factor_CL_incurred_interaktywne():
        Dev_pd_Incurred = wspolczynniki_multiplikatywna_incurred_interaktywna().iloc[1,1:]
        sd_pd = wspolczynniki_multiplikatywna_incurred_interaktywna().iloc[5,1:]
        ilosc_dop_wsp_CL_incurred = [int(x) for x in input.chose_CL_incurred()]
        vector_value_incurred, x_k_ind_incurred = yh.check_value(
            Dev_pd_Incurred,
            ilosc_dop_wsp_CL_incurred, input.Min_CL_incurred(), input.Max_CL_incurred())
        n_CL_incurred = len(Dev_pd_Incurred) - (len(ilosc_dop_wsp_CL_incurred) - len(x_k_ind_incurred))
        sd_chose = [sd_pd[x-1] for x in x_k_ind_incurred]
        a, b = yh.fit_curve(vector_value_incurred, sd_chose,
                            x_k_ind_incurred, 'factor_CL', n_CL_incurred)
        dev_pozostawione = [Dev_pd_Incurred[x] for x in range(0, (input.Poz_CL_incurred()))]
        vec_output = ['CL'] + dev_pozostawione + yh.wspolczynnik_reg(a, b,input.Poz_CL_incurred() + 1,len(Dev_pd_Incurred.tolist()) + input.ilosc_okresow_incurred()-1,
                                                                                               'factor_CL')
        return (vec_output)

    @reactive.Calc
    @reactive.event(input.accept_p_i, ignore_none=False)
    def dopasowanie_krzywej_P_to_I_interaktywne():
        rj_pd_Incurred = wspolczynniki_multiplikatywna_incurred_interaktywna().iloc[3,1:]
        #wro policzyc sd dla tego
        sd_pd_Incurred = wspolczynniki_multiplikatywna_incurred_interaktywna().iloc[4,1:]
        ilosc_dop_wsp_P_to_I = [int(x) for x in input.chose_p_i()]
        vector_value_p_i, x_k_ind_p_i = yh.check_value(rj_pd_Incurred,ilosc_dop_wsp_P_to_I, input.Min_p_i(), input.Max_p_i())
        n_CL_p_i = len(rj_pd_Incurred) - (len(ilosc_dop_wsp_P_to_I) - len(x_k_ind_p_i))
        a, b = yh.fit_curve(vector_value_p_i, sd_pd_Incurred,
                            x_k_ind_p_i,
                            'factor P_to_I', n_CL_p_i)
        rj_pozostawione = [rj_pd_Incurred[x] for x in range(0, (input.Poz_p_i()))]
        vec_output_p_i = (['rj'] + rj_pozostawione
                          + yh.wspolczynnik_reg(a, b, input.Poz_p_i() + 1,len(rj_pd_Incurred) + input.ilosc_okresow_incurred() ,
                                                                                           'factor P_to_I'))
        return (vec_output_p_i)

    #@reactive.Calc
  #  @reactive.event(input.accept_p_i_var, ignore_none=False)
  #  def dopasowanie_krzywej_variance_P_to_I():
   #     varj_pd_Incurred = wspolczynniki_r()[1]
  #      ilosc_dop_wsp_var_p_i = [int(x) for x in input.chose_var_p_i()]
    #    vector_value_p_i, x_k_ind_p_i = yh.check_value(varj_pd_Incurred.iloc[index_lini_CL_Incurred(), ilosc_dop_wsp_var_p_i].to_list(),
    #                                                   ilosc_dop_wsp_var_p_i, input.Min_var_p_i(), input.Max_var_p_i())
   #     n_CL_var_p_i = len(varj_pd_Incurred.iloc[index_lini_CL_Incurred(),].to_list()) - (len(ilosc_dop_wsp_var_p_i) - len(x_k_ind_p_i))
     #   a, b = yh.fit_curve(vector_value_p_i, varj_pd_Incurred.iloc[index_lini_CL_Incurred(), x_k_ind_p_i].to_list(), x_k_ind_p_i,
     #                       'variance_P_to_I', n_CL_var_p_i)
     #   vec_output_var_p_i = ['varj'] + varj_pd_Incurred.iloc[index_lini_CL_Incurred(),
      #                                  1:(input.Poz_p_i_var() + 1)].to_list() + yh.wspolczynnik_reg(a, b,
        #                                                                                             input.Poz_p_i_var() + 1,
       #                                                                                              varj_pd_Incurred.shape[
       #                                                                                                  1] + input.ilosc_okresow_incurred() - 1,
       #                                                                                              'variance_P_to_I')
        #return (vec_output_var_p_i)


    @render.data_frame
    def wsol_z_krzywej_CL_paid_interaktywnap_pi_i():
        Dev_pd = wspolczynniki_multiplikatywna_incurred_interaktywna().iloc[1,1:]
        II_dataframe_incurred = pd.DataFrame(0,index = [0,1,2,3,4], columns = [str(x) for x in range(1,len(Dev_pd)+2+input.ilosc_okresow())])
        II_dataframe_incurred.iloc[0, :] = dopasowanie_krzywej_factor_CL_incurred_interaktywne()+[np.nan]
        II_dataframe_incurred.iloc[1, :] = dopasowanie_krzywej_P_to_I_interaktywne()
       # II_dataframe_incurred.iloc[2, :] = dopasowanie_krzywej_P_to_I()[1:]
      #  II_dataframe_incurred.iloc[3, :] = dopasowanie_krzywej_variance_P_to_I()
        return render.DataGrid(
            II_dataframe_incurred,
            width="100%",
            height="100%",
            row_selection_mode='single'
        )

    @output
    @render.plot()
    def plot_wspolczynniki_dopasowane_interaktywny_pi_i():
        fig = plt.figure()
        Dev_pd_Incurred = wspolczynniki_multiplikatywna_incurred_interaktywna().iloc[1,1:]
        rj_pd_Incurred = wspolczynniki_multiplikatywna_incurred_interaktywna().iloc[3,1:]
        if (input.id_panel_CL_inc() == None):
            plt.xticks(np.arange(1, len(Dev_pd_Incurred)+input.ilosc_okresow()))
            fig.autofmt_xdate()
        elif (input.id_panel_CL_inc()[0]=="Dopasowanie CL Incurred"):
            CL_fit = dopasowanie_krzywej_factor_CL_incurred_interaktywne()
            plt.plot(np.arange(1,len(Dev_pd_Incurred)+1), Dev_pd_Incurred, 'b',label='CL Incurred')
            plt.plot(np.arange(input.Poz_CL_incurred(),len(CL_fit)), CL_fit[input.Poz_CL_incurred():], 'r',label='Dopasowane CL Incurred')
            plt.xticks(np.arange(1, len(Dev_pd_Incurred)+input.ilosc_okresow()))
            fig.autofmt_xdate()
            fig.legend()
        elif (input.id_panel_CL_inc()[0]=="Dopasowanie Paid/Incurred"):
            rj_fit = dopasowanie_krzywej_P_to_I_interaktywne()
            plt.plot(np.arange(1,len(rj_pd_Incurred)+1), rj_pd_Incurred, 'b',label='varj')
            plt.plot(np.arange(input.Poz_p_i(),len(rj_fit)), rj_fit[input.Poz_p_i():], 'r',label='Dopasowane P/I')
            plt.xticks(np.arange(1, len(rj_pd_Incurred)+input.ilosc_okresow()))
            fig.autofmt_xdate()
            fig.legend()
        return fig


    @reactive.Calc
    def calc_chainladder_incurred_interaktywne():
        trian_list = every_triangle()[2]
        triangle = trian_list[index_lini_CL_Paid()].iloc[:,1:]
        CL_fit = dopasowanie_krzywej_factor_CL_incurred_interaktywne()
        n_dim = triangle.shape[1]
        Dev_j_base = wspolczynniki_multiplikatywna_incurred_interaktywna().iloc[0,1:n_dim].to_list()
        Dev_j_z_wagami = wspolczynniki_multiplikatywna_incurred_interaktywna().iloc[1,1:n_dim].to_list()
        data_output = pd.DataFrame(0, index=np.arange(0, triangle.shape[0] + 1),
                                   columns=['Rok/Suma', 'Ult_base', 'IBNR_base', 'Ult z wagami', 'IBNR z wagami','Ult z krzywą', 'IBNR z krzywą'])
        data_output.iloc[:, 0] = np.arange(0, triangle.shape[0] + 1)
        k = 1
        rj_base = wspolczynniki_multiplikatywna_incurred_interaktywna().iloc[2,2:].to_list()
        rj_waga = wspolczynniki_multiplikatywna_incurred_interaktywna().iloc[3, 2:].to_list()
        rj_fit = dopasowanie_krzywej_P_to_I_interaktywne()[2:]
        dev_rj = [x*y for x,y in zip(Dev_j_base,rj_base)]
        dev_rj_waga = [x*y for x,y in zip(Dev_j_base,rj_waga)]
        dev_rj_fit = [x*y for x,y in zip(CL_fit[1:],rj_fit)]

        for wspolczynniki in [dev_rj, dev_rj_waga,dev_rj_fit ]:
            proj_triangle = yh.triangle_forward(triangle, wspolczynniki, 0)
            diag = yh.reverse_list(yh.trian_diag(triangle))
            Ultimate_Param_ReservingRisk = proj_triangle.iloc[:, int(proj_triangle.columns[-1]) - 1].to_list()
            data_output.iloc[:, k] = Ultimate_Param_ReservingRisk + [np.sum(Ultimate_Param_ReservingRisk)]
            k = k + 1
            BE_Param_ReservingRisk = [x - y for x, y in zip(Ultimate_Param_ReservingRisk, diag)]
            data_output.iloc[:, k] = BE_Param_ReservingRisk + [np.sum(BE_Param_ReservingRisk)]
            k = k + 1
        return (data_output)

    @output
    @render.data_frame
    def Ult_BE_data_interaktywne_pi_i():
        df = calc_chainladder_incurred_interaktywne()
        return render.DataGrid(
            df,
            width="100%",
            height="150%",
        )
######
            ##koniec paid/inccured

    @output
    @render.image
    def image():
        from pathlib import Path

        dir = Path(__file__).resolve().parent
        img: ImgData = {"src": str(dir / "Model Ryzyka Rezerw.png"), "width": "1600px", "height": "900px"}
        return img

  #WEJSCIE
    @reactive.Calc
    def Upload_data():
        if input.file1() is None:
            return "Wprowadź dane"
        f: list[FileInfo] = input.file1()
        xl = pd.ExcelFile(f[0]["datapath"])
        sheet_names = xl.sheet_names
        #xl.close()
        return [xl, sheet_names]
#WRÓĆ !!!!!
    @reactive.Calc
    def Upload_wagi_P_I():
        if input.wagi_p_i_input() is not None:
            f: list[FileInfo] = input.wagi_p_i_input()
            l_bis_LR = every_triangle()[1]
            every_wagi_LR = []
            for linia in ["FIRE_I"]:
                df_wagi_LR = pd.read_excel(open(f[0]["datapath"], 'rb'), index_col=0,
                                        sheet_name=linia)
                col_list = [str(x) for x in df_wagi_LR.columns.to_list()]
                df_copy = pd.DataFrame(0, columns=col_list,
                                       index=np.arange(0, df_wagi_LR.shape[0]))
                for i in range(0, df_wagi_LR.shape[1]): df_copy.iloc[:, i] = df_wagi_LR.iloc[:, i].to_list()
                df_copy.insert(0, "Rok", df_wagi_LR.index.to_list(), True)
                every_wagi_LR.append(df_copy)
        return (every_wagi_LR)

    @reactive.Calc
    def Upload_wagi():
        if input.wagi_input() is not None:
            f: list[FileInfo] = input.wagi_input()
            l_bis = every_triangle()[1]
            every_wagi = []
            for linia in l_bis:
                df_wagi = pd.read_excel(open(f[0]["datapath"], 'rb'), index_col=0,
                                        sheet_name=linia)
                col_list = [str(x) for x in df_wagi.columns.to_list()]
                df_copy = pd.DataFrame(0, columns=col_list,
                                       index=np.arange(0, df_wagi.shape[0]))
                for i in range(0, df_wagi.shape[1]): df_copy.iloc[:, i] = df_wagi.iloc[:, i].to_list()
                df_copy.insert(0, "Rok", df_wagi.index.to_list(), True)
                every_wagi.append(df_copy)
        return (every_wagi)

    @reactive.Calc
    def Upload_wagi_LR():
        if input.wagi_input_LR() is not None:
            f: list[FileInfo] = input.wagi_input_LR()
            l_bis_LR = every_triangle()[1]
            every_wagi_LR = []
            for linia in l_bis_LR:
                df_wagi_LR = pd.read_excel(open(f[0]["datapath"], 'rb'), index_col=0,
                                        sheet_name=linia)
                col_list = [str(x) for x in df_wagi_LR.columns.to_list()]
                df_copy = pd.DataFrame(0, columns=col_list,
                                       index=np.arange(0, df_wagi_LR.shape[0]))
                for i in range(0, df_wagi_LR.shape[1]): df_copy.iloc[:, i] = df_wagi_LR.iloc[:, i].to_list()
                df_copy.insert(0, "Rok", df_wagi_LR.index.to_list(), True)
                every_wagi_LR.append(df_copy)
        return (every_wagi_LR)

    @reactive.Calc
    def Upload_ekspozycje():
        if input.ekspozycja_input() is None:
            return "Wprowadź dane"
        f_ekspozycje: list[FileInfo] = input.ekspozycja_input()
        xl_ekspozycje = pd.read_excel(f_ekspozycje[0]["datapath"], index_col=0)
        col_list = [str(x) for x in xl_ekspozycje.columns.to_list()]
        df_copy = pd.DataFrame(0, columns=col_list,
                               index=np.arange(0, xl_ekspozycje.shape[0]))
        for i in range(0, xl_ekspozycje.shape[1]): df_copy.iloc[:, i] = xl_ekspozycje.iloc[:, i].to_list()
        df_copy.insert(0, "LoB", xl_ekspozycje.index.to_list(), True)
        return df_copy

    @reactive.Calc
    def Upload_inflacje():
        if input.inflacja_input() is None:
            return "Wprowadź dane"
        f_inflacja: list[FileInfo] = input.inflacja_input()
        xl_inflacja = pd.read_excel(f_inflacja[0]["datapath"], index_col=0)
        col_list = [str(x) for x in xl_inflacja.columns.to_list()]
        df_copy = pd.DataFrame(0, columns=col_list,
                               index=np.arange(0, xl_inflacja.shape[0]))
        for i in range(0, xl_inflacja.shape[1]): df_copy.iloc[:, i] = xl_inflacja.iloc[:, i].to_list()
        df_copy.insert(0, "LoB", xl_inflacja.index.to_list(), True)
        return df_copy

    #Aktuaizacja zakladek
    @reactive.Effect
    def _():
        if input.file1() is None:
            ui.update_selectize(
                "zakladka",
                choices=['-'],
                server=True,
            )
        else:
            x = Upload_data()[1]
            ui.update_selectize(
                "zakladka",
                choices=x,
                server=False,
            )

    @reactive.Effect
    def _():
        l_bis = data_analisis()[1]
        if (len(l_bis) == 0):
            ui.update_selectize(
                "linie_biznesowe",
                choices=['-'],
                server=True,
            )
        else:
            ui.update_selectize(
                "linie_biznesowe",
                choices=l_bis,
                server=False,
            )
            ui.update_selectize(
                "linie_biznesowe_CL_Paid",
                choices=l_bis,
                server=False,
            )
            ui.update_selectize(
                "linie_biznesowe_CL_Incurred",
                choices=l_bis,
                server=False,)
            ui.update_selectize(
                "linie_biznesowe_LR_Paid",
                choices=l_bis,
                server=False,
            )
            ui.update_selectize(
                "linie_biznesowe_LR_Incurred",
                choices=l_bis,
                server=False,
            )
            ui.update_selectize(
                "linie_biznesowe_CL_Paid_stoch",
                choices=l_bis,
                server=False,
            )

    @reactive.Calc
    def data_analisis():
        if input.file1() is None:
            l_bis = []
            df_paid = pd.DataFrame({'a': [],
                               'b': []})
            df_reserve = pd.DataFrame({'a': [],
                               'b': []})
        else:
            if (input.zakladka() == '-'):
                l_bis = []
                df_paid = pd.DataFrame({'a':  [],
                                   'b':  []})
                df_reserve = pd.DataFrame({'a':  [],
                                   'b':  []})
            else:
                xl = Upload_data()[0]
                df_paid = xl.parse('Triangle_paid_gross')
                xl = Upload_data()[0]
                df_reserve = xl.parse('Triangle_case_reserves_gross')
                l_bis = list(set(df_paid.iloc[:, 0].to_list()))
        return [df_paid, l_bis,df_reserve]

    @reactive.Calc
#    def every_triangle():
#        lista_trojkatow_paid = []
#        lista_trojkatow_incurred = []
#        df_paid = data_analisis()[0]
#        df_reserve = data_analisis()[2]
##        l_bis = data_analisis()[1]
#        inflacja_dataframe = Upload_inflacje()
#        for linia in l_bis:
#            triangle_paid = yh.show_triangle_for_libes_app(df_paid, linia)
#            triangle_paid = yh.change_value_less_diagonal(triangle_paid,np.nan)
#            triangle_reserve = yh.show_triangle_for_libes(df_reserve, linia)
           # print(triangle_reserve.to_string())
 #           if (input.potwierdz_inflacje()==True):
#                #triangle_paid = yh.inflation_to_triangle(inflacja_dataframe, triangle_paid, linia)
#                triangle_reserve= triangle_reserve.cumsum(axis=1)
#                trian_resrve_add = yh.change_value_less_diagonal(triangle_reserve, np.nan)

            #    print(trian_resrve_add.to_string())

#                res_infl = yh.inflation_to_triangle(inflacja_dataframe, trian_resrve_add, linia)
#                incremental_reserv = yh.incremental_triangle(res_infl)
#                paid_infl = yh.inflation_to_triangle(inflacja_dataframe, triangle_paid, linia)
#                triangle_incurred_2 = paid_infl.add(incremental_reserv)
#                triangle_incurred_3 = triangle_incurred_2.copy()
#            else:
#                paid_infl = df_paid.copy()
#                triangle_incurred_3 = paid_infl.add(triangle_reserve)
            #uwzglednienie inflacji
            #dla paidow
#            col_list = [str(x) for x in np.arange(0, triangle_paid.shape[1] + 1)]
#            df_copy = pd.DataFrame(0, columns=col_list,
#                                   index=np.arange(0, triangle_paid.shape[0]))
#            df_copy.iloc[:, 0] = triangle_paid.index.to_list()
#            for i in range(1, triangle_paid.shape[1] + 1): df_copy.iloc[:, i] = paid_infl.iloc[:, i - 1].to_list()
#            df_output = yh.change_value_less_diagonal(df_copy, np.nan)
#            lista_trojkatow_paid.append(df_output)
#            # dla incurred
#            col_list = [str(x) for x in np.arange(0, triangle_paid.shape[1] + 1)]
#            df_copy_incurred = pd.DataFrame(0, columns=col_list,
#                                   index=np.arange(0, triangle_paid.shape[0]))
#            df_copy_incurred.iloc[:, 0] = triangle_incurred_3.index.to_list()
#            for i in range(1, triangle_paid.shape[1] + 1): df_copy_incurred.iloc[:, i] = triangle_incurred_3.iloc[:, i - 1].to_list()
#            df_output_incurred = yh.change_value_less_diagonal(df_copy_incurred, np.nan)
#            lista_trojkatow_incurred.append(df_output_incurred)
#            #([lista_trojkatow_paid,l_bis,lista_trojkatow_incurred])
#        return ([lista_trojkatow_paid,l_bis,lista_trojkatow_incurred])

    #  return ([lista_trojkatow_paid,l_bis,lista_trojkatow_incurred])



    @reactive.Effect
    def load_data():
        # Funkcja uruchamiana po kliknięciu przycisku
        if input.load_button():
            # Pobieramy dane od użytkownika
            folder_path = input.folder_path()
            folder_path = folder_path.replace("\\","/")
            file_name = input.file_name()
            sheet_name = input.sheet_name()
            start_row = input.start_row()
            start_col = input.start_col()
            end_row = input.end_row()
            end_col = input.end_col()

            #folder_path = "I:/Ubezpieczeniowe/2025/STU Ergo Hestia SA/3_Od_zakładu/Pismo DIU STU ERGO Hestia 2025 - odp 14.01.2025 pkt 8/CP ResQ"  # Wskaż folder, gdzie jest plik
            #file_name = "CP_MTPL_I_202409.xlsx"  # Nazwa pliku
            #sheet_name = "DFM Ratios_4"  # Nazwa arkusza
            #start_row = 9  # Wiersz początkowy
            #start_col = 1  # Kolumna początkowa
            #end_row = 97  # Wiersz końcowy
            #end_col = 89  # Kolumna końcowa

            #folder_path = "M:/Hestia kontrola/Test_Aplikacji/"  # Wskaż folder, gdzie jest plik
            #file_name = "test_trojkaty.xlsx"  # Nazwa pliku
            #sheet_name = "paid"  # Nazwa arkusza
            #start_row = 2  # Wiersz początkowy
            #start_col = 1  # Kolumna początkowa
            #end_row = 19  # Wiersz końcowy
            #end_col = 18

            #folder_path = "I:/Ubezpieczeniowe/2025/STU Ergo Hestia SA/3_Od_zakładu/wezwanie z 2025-01-22/pkt 2/"  # Wskaż folder, gdzie jest plik
            #file_name = "ACCIDENT_BA.xlsx"  # Nazwa pliku
            #sheet_name = "G_dir-ALAE_PAID"  # Nazwa arkusza
            #start_row = 5  # Wiersz początkowy
            #start_col = 1  # Kolumna początkowa
            #end_row = 93  # Wiersz końcowy
            #end_col = 89

            data = load_excel_data(folder_path, file_name, sheet_name, start_row+1, start_col, end_row, end_col)
            if(input.radio_button_paid()=="Inkrementalne"):
                data.iloc[:,1:] = data.iloc[:,1:].cumsum(axis=1)
            # Ustawiamy dane do reactive
            data = yh.change_value_less_diagonal(data,np.nan)
            reactive_data.set(data)
        if input.load_button_inc():
            #folder_path = "I:/Ubezpieczeniowe/2025/STU Ergo Hestia SA/3_Od_zakładu/Pismo DIU STU ERGO Hestia 2025 - odp 14.01.2025 pkt 8/CP ResQ"  # Wskaż folder, gdzie jest plik
            #file_name = "CP_MTPL_I_202409.xlsx"  # Nazwa pliku
            #sheet_name = "Add_4"  # Nazwa arkusza
            #start_row = 6  # Wiersz początkowy
            #start_col = 1  # Kolumna początkowa
            #end_row = 93  # Wiersz końcowy
            #end_col = 89  # Kolumna końcowa
            # Wczytanie danych
            folder_path_inc = input.folder_path_inc()
            folder_path_inc = folder_path_inc.replace("\\","/")
            file_name_inc = input.file_name_inc()
            sheet_name_inc = input.sheet_name_inc()
            start_row_inc = input.start_row_inc()
            start_col_inc = input.start_col_inc()
            end_row_inc = input.end_row_inc()
            end_col_inc = input.end_col_inc()

            #folder_path = "M:/Hestia kontrola/Test_Aplikacji/"  # Wskaż folder, gdzie jest plik
            #file_name = "test_trojkaty.xlsx"  # Nazwa pliku
            #sheet_name = "inc"  # Nazwa arkusza
            #start_row = 5 # Wiersz początkowy
            #start_col = 3  # Kolumna początkowa
            #end_row = 22  # Wiersz końcowy
            #end_col = 20

            #folder_path = "I:/Ubezpieczeniowe/2025/STU Ergo Hestia SA/3_Od_zakładu/Pismo DIU STU ERGO Hestia 2025 - odp 14.01.2025 pkt 8/CP ResQ/"  # Wskaż folder, gdzie jest plik
            #file_name = "CP_MAT_05_202409.xlsx"  # Nazwa pliku
            #sheet_name = "add1"  # Nazwa arkusza
            ##start_row = 7  # Wiersz początkowy
            #start_col = 1  # Kolumna początkowa
            #end_row = 95  # Wiersz końcowy
            #end_col = 89

            #folder_path = "I:/Ubezpieczeniowe/2025/STU Ergo Hestia SA/3_Od_zakładu/wezwanie z 2025-01-22/pkt 2/"  # Wskaż folder, gdzie jest plik
            #file_name = "ACCIDENT_BA.xlsx"  # Nazwa pliku
            #sheet_name = "G_dir-ALAE_INCURRED"  # Nazwa arkusza
            #start_row = 5  # Wiersz początkowy
            #start_col = 1  # Kolumna początkowa
            #end_row = 93  # Wiersz końcowy
            #end_col = 89


            data_inc = load_excel_data(folder_path_inc, file_name_inc, sheet_name_inc, start_row_inc+1, start_col_inc, end_row_inc, end_col_inc)
            if(input.radio_button_inc()=="Inkrementalne"):
                data_inc.iloc[:,1:] = data_inc.iloc[:,1:].cumsum(axis=1)
            # Ustawiamy dane do reactive
            data_inc = yh.change_value_less_diagonal(data_inc,np.nan)
            reactive_data_inc.set(data_inc)
        if input.load_button_eksp():
                #folder_path = "I:/Ubezpieczeniowe/2025/STU Ergo Hestia SA/3_Od_zakładu/Pismo DIU STU ERGO Hestia 2025 - odp 14.01.2025 pkt 8/CP ResQ"  # Wskaż folder, gdzie jest plik
                #file_name = "CP_MTPL_I_202409.xlsx"  # Nazwa pliku
                #sheet_name = "Add_4"  # Nazwa arkusza
                #start_row = 6  # Wiersz początkowy
                #start_col = 90  # Kolumna początkowa
                #end_row = 93  # Wiersz końcowy
                #end_col = 90  # Kolumna końcowa
                # Wczytanie danych
                folder_path = input.folder_path_eksp()
                folder_path = folder_path.replace("\\", "/")
                file_name = input.file_name_eksp()
                sheet_name = input.sheet_name_eksp()
                start_row = input.start_row_eksp()
                start_col = input.start_col_eksp()
                end_row = input.end_row_eksp()
                end_col = input.end_col_eksp()
                #data_eksp = load_excel_data(folder_path, file_name, sheet_name, start_row, start_col, end_row, end_col)

                #folder_path = "M:/Hestia kontrola/Test_Aplikacji/"  # Wskaż folder, gdzie jest plik
                #file_name = "test_trojkaty.xlsx"  # Nazwa pliku
                #sheet_name = "inc"  # Nazwa arkusza
                ##start_row = 5  # Wiersz początkowy
                #start_col = 21  # Kolumna początkowa
                #end_row = 22  # Wiersz końcowy
                #end_col = 21


                #folder_path = "I:/Ubezpieczeniowe/2025/STU Ergo Hestia SA/3_Od_zakładu/Pismo DIU STU ERGO Hestia 2025 - odp 14.01.2025 pkt 8/CP ResQ/"  # Wskaż folder, gdzie jest plik
                #file_name = "CP_MAT_05_202409.xlsx"  # Nazwa pliku
                #sheet_name = "add1"  # Nazwa arkusza
                #start_row = 7  # Wiersz początkowy
                #start_col = 90  # Kolumna początkowa
                #end_row = 95  # Wiersz końcowy
                #end_col = 90

                data_eksp = load_excel_data(folder_path, file_name, sheet_name, start_row+1, start_col, end_row, end_col)

                reactive_data_eksp.set(data_eksp)

    #OBLICZEWNIA
    # Wprowadx dane i wstepna analiza
   # @reactive.Calc
    def every_triangle():
        lista_trojkatow_paid = []
        lista_trojkatow_incurred = []
        lista_expo_list = []
        df_output = reactive_data.get()
        df_output_incurred = reactive_data_inc.get()
       # df_output = pd.read_excel("trian_for_line_paid.xlsx",engine='openpyxl')
        #df_output_incurred = pd.read_excel("trian_for_line_incurred.xlsx",engine='openpyxl')
        #df_output_incurred = pd.DataFrame()
        lista_trojkatow_incurred.append(df_output_incurred)
        lista_trojkatow_paid.append(df_output)
        lista_expo = reactive_data_eksp.get()
        if input.load_button_eksp():
            lista_expo_list.append(lista_expo.iloc[:,0].tolist())
        #lista_expo.append([35315055.72838893, 54364804.30372669, 55999463.22609352, 96755495.69907188, 175628551.0412365, 253792055.82037112, 351956996.66924846, 457757405.8755202, 525655194.93950087, 593008506.3120264, 723545894.6129045, 773685678.0308353, 766873248.3055058, 764346574.2868818, 900983078.4543248, 1289517470.3297994, 1549566745.7093534, 1613031024.196641, 1609072857.6647491, 1615898049.86,542982799.25	,396775237.01	,282862629.95	,162919290.86,	97519794.37,	96588223.4,	65726952.87])
        l_bis = ["MTPL_I"]

        return ([lista_trojkatow_paid,l_bis,lista_trojkatow_incurred,lista_expo_list])

    @reactive.Calc
    def index_lini():
        l_bis = every_triangle()[1]
        ind, = np.where(np.array(l_bis) == str(input.linie_biznesowe()))
        return(ind[0])

    @reactive.Calc
    def index_lini_CL_Paid():
        l_bis = every_triangle()[1]
        ind, = np.where(np.array(l_bis) == str(input.linie_biznesowe_CL_Paid()))
        return(ind[0])


    @reactive.Calc
    def index_lini_LR_Paid():
        l_bis = every_triangle()[1]
        ind, = np.where(np.array(l_bis) == str(input.linie_biznesowe_LR_Paid()))
        return(ind[0])


    @reactive.Calc
    def index_lini_CL_Incurred():
        l_bis = every_triangle()[1]
        ind, = np.where(np.array(l_bis) == str(input.linie_biznesowe_CL_Incurred()))
        return(ind[0])

    @reactive.Calc
    def index_lini_LR_paid():
        l_bis = every_triangle()[1]
        ind, = np.where(np.array(l_bis) == str(input.linie_biznesowe_LR_Paid()))
        return (ind[0])

    @reactive.Calc
    def index_lini_LR_incurred():
        l_bis = every_triangle()[1]
        ind, = np.where(np.array(l_bis) == str(input.linie_biznesowe_LR_Incurred()))
        return (ind[0])
########################################################################################################################

app = App(app_ui, server)
run_app(app,port =8003)
