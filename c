def server(input, output, session):
    user_data = UserSessionData()

    @reactive.Effect
    @reactive.event(input.load_button)
    def load_data_paid():
        lista = user_data.reactive_data_paid_list.get()
        lista.append(df)
        user_data.reactive_data_paid_list.set(lista)
        show_info_modal("Dane Paid zostały wczytane.")
