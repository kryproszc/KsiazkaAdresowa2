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

    # 💡 Dodajemy CSS zapobiegający łamaniu liczb
    return ui.HTML(f"""
        <style>
            #triangle-table-1 td {{
                white-space: nowrap;
            }}
        </style>
        {html_table}
    """)
