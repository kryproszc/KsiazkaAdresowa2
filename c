# === triangle_utils.py ===

import pandas as pd
import numpy as np
import plotly.graph_objects as go

class TriangleCalculator:
    def __init__(self):
        pass  # nie trzymamy danych na starcie, tylko czekamy na dane wejściowe

    def calculate_ratios_vectorized(self, df):
        data = df.iloc[:, 1:].to_numpy(dtype=float)  # pomijamy kolumnę 'AY'
        denominator = data[:, :-1]
        numerator = data[:, 1:]
        mask = denominator != 0
        ratios = np.full_like(denominator, fill_value=np.nan)
        ratios[mask] = numerator[mask] / denominator[mask]
        ratio_df = pd.DataFrame(
            np.hstack([ratios, np.full((ratios.shape[0], 1), np.nan)]),
            columns=df.columns[1:],
            index=df.index
        )
        return ratio_df

    def create_binary_df(self, ratio_df):
        return ratio_df.map(lambda x: 1 if pd.notna(x) else np.nan)

    def calculate_dev(self, triangle_df, weights_df, ratio_df):
        data_paid = triangle_df.iloc[:, 1:].to_numpy(dtype=float)
        weights = weights_df.to_numpy(dtype=float)
        l_matrix = ratio_df.to_numpy(dtype=float)

        mask = ~np.isnan(data_paid) & ~np.isnan(l_matrix)

        numerator = np.nansum(data_paid * weights * l_matrix * mask, axis=0)
        denominator = np.nansum(data_paid * weights * mask, axis=0)

        with np.errstate(divide='ignore', invalid='ignore'):
            dev_j = np.where(denominator == 0, 1, numerator / denominator)

        return dev_j

    def get_r2_numpy_manual(self, x, y):
        zx = (x - np.mean(x)) / np.std(x, ddof=1)
        zy = (y - np.mean(y)) / np.std(y, ddof=1)
        r = np.sum(zx * zy) / (len(x) - 1)
        return r ** 2

    def r2_curves(self, f_curves, y_input):
        r2_value = pd.DataFrame(np.nan, index=['Wartość'], columns=f_curves.index)

        k = 0
        for row in f_curves.index:

            if (row == "Exponential"):
                r2 = self.get_r2_numpy_manual(np.log(y_input - 1), np.log(f_curves.loc[row] - 1))
                r2_value.iloc[0, k] = r2

            elif (row == "Weibull"):
                r2 = self.get_r2_numpy_manual(
                    np.log(np.log(y_input / (y_input - 1))),
                    np.log(np.log(f_curves.loc[row] / (f_curves.loc[row] - 1)))
                )
                r2_value.iloc[0, k] = r2

            elif (row == "Power"):
                r2 = self.get_r2_numpy_manual(
                    np.log(np.log(y_input)),
                    np.log(np.log(f_curves.loc[row]))
                )
                r2_value.iloc[0, k] = r2

            elif (row == "Inverse Power"):
                r2 = self.get_r2_numpy_manual(
                    np.log(y_input - 1),
                    np.log(f_curves.loc[row] - 1)
                )
                r2_value.iloc[0, k] = r2

            k = k + 1

        return (r2_value)

    def curve_predict(self, x, a, b, c, curve):
        if curve == 'Exponential':
            y_fit = 1 + a * np.exp(b * x)
        elif curve == 'Weibull':
            y_fit = 1 / (1 - np.exp(-a * x ** b))
            # y_fit = 1 + a * x ** (a - 1) * np.exp(-(x ** a))  # alternatywna wersja zakomentowana
        elif curve == 'Power':
            y_fit = a * (b * x)
        elif curve == 'Inverse Power':
            y_fit = 1 + a * (c + x) ** b
        return y_fit

    def parameters_curve_reservoir(self, xs, ys, lista_krzywych):
        # Inicjalizacja DataFrame na wyniki
        parametry = pd.DataFrame(np.nan, index=lista_krzywych, columns=['a', 'b', 'c'])

        for krzywa in lista_krzywych:
            if krzywa == 'Exponential':
                param = np.polyfit(xs, np.log(ys - 1), deg=1)
                parametry.loc['Exponential', ['a', 'b']] = param

            elif krzywa == 'Weibull':
                param = np.polyfit(np.log(xs), np.log(np.log(ys / (ys - 1))), deg=1)
                parametry.loc['Weibull', ['a', 'b']] = param

            elif krzywa == 'Power':
                param = np.polyfit(xs, np.log(np.log(ys)), deg=1)
                parametry.loc['Power', ['a', 'b']] = param

            elif krzywa == 'Inverse Power':
                best_r2 = -np.inf
                best_param = None

                for c in [-0.5, 1, 3, 5]:
                    param = np.polyfit(np.log(xs + c), np.log(ys - 1), deg=1)
                    y_pred = self.curve_predict(xs, np.exp(param[1]), param[0], c, curve='Inverse Power')
                    r2 = self.get_r2_numpy_manual(np.log(ys - 1), np.log(y_pred))

                    if r2 > best_r2:
                        best_r2 = r2
                        best_param = list(param) + [c]

                parametry.loc['Inverse Power'] = best_param

        return parametry

    def sim_data_curve_rezerwy(self, x, lista_krzywych, parameters_curve_num):
        factor_value = pd.DataFrame(np.nan, index=lista_krzywych,
                                    columns=['dp: ' + str(j) for j in range(1, len(x) + 1)])

        for l_curve in lista_krzywych:
            if l_curve == 'Exponential':
                param = parameters_curve_num.loc['Exponential'][0:2]
                y = self.curve_predict(x, np.exp(param.iloc[1]), param.iloc[0], c=0, curve='Exponential')
                factor_value.loc['Exponential'] = y

            elif l_curve == 'Weibull':
                param = parameters_curve_num.loc['Weibull'][0:2]
                y = self.curve_predict(x, np.exp(param.iloc[1]), param.iloc[0], c=0, curve='Weibull')
                factor_value.loc['Weibull'] = y

            elif l_curve == 'Power':
                param = parameters_curve_num.loc['Power'][0:2]
                y = self.curve_predict(x, np.exp(np.exp(param.iloc[1])), np.exp(param.iloc[0]), c=0, curve='Power')
                factor_value.loc['Power'] = y

            elif l_curve == 'Inverse Power':
                param = parameters_curve_num.loc['Inverse Power']
                y = self.curve_predict(x, np.exp(param.iloc[1]), param.iloc[0], param.iloc[2], curve='Inverse Power')
                factor_value.loc['Inverse Power'] = y

        return factor_value


    def triangle_forward(self, df_data, f, k_forward_start):
        df_t = df_data.copy()
        data = df_t.values  # numpy array for speed
        m, n = data.shape

        max_len = len(f) + 1
        if max_len > n:
            # dodaj nowe kolumny jeśli trzeba
            extra_cols = max_len - n
            data = np.hstack((data, np.full((m, extra_cols), np.nan)))

        for j in range(k_forward_start - 1, len(f)):
            col_from = data[:, j]
            col_to = data[:, j + 1]
            start_row = max(0, m - j - 1)
            data[start_row:, j + 1] = col_from[start_row:] * f[j]

        col_names = list(df_data.columns) + [f'proj_{i}' for i in range(n, data.shape[1])]
        return pd.DataFrame(data, columns=col_names)
