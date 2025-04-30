""" Copula generation and results export module """

import math
import os
import sys

import numpy as np
import pandas as pd
import scipy.stats as st
from scipy.stats.mstats import rankdata
import cvxpy as cp


class CopulaJoin(object):
    """ Copula generation and results export class """

    def __init__(self, sym_res=None, calc_date="", cop_type="", cop_corr=0.0, df_num=0.0, cop_alpha=0.0,
                 sf_oprisk=None, out_path="", import_main=None, rep_num=0, m_num=0, group_calc=False):
        self.sym_res = sym_res
        self.calc_date = calc_date
        self.cop_type = cop_type
        self.cop_corr = cop_corr
        self.df_num = df_num
        self.rep_num = rep_num
        self.m_num = m_num
        self.cop_alpha = cop_alpha
        self.sf_oprisk = sf_oprisk
        self.out_path = out_path
        self.import_main = import_main
        self.group_calc = group_calc
        self.copula_and_export()

    # Eksport wynikow do plikow Excel
    def copula_and_export(self):
        """ Copula generation and results export function """

        calc_date = self.calc_date
        cop_type = self.cop_type
        cop_corr = self.cop_corr
        cop_alpha = self.cop_alpha
        sym_res = self.sym_res
        sf_oprisk = self.sf_oprisk
        out_path = self.out_path
        rep_num = self.rep_num
        m_num = self.m_num
        group_calc = self.group_calc
        spl_keys = sym_res.spl_keys
        res_dict1 = sym_res.res_dict1
        sym_num = sym_res.sym_num
        op_risk_perc = sym_res.op_risk_perc
        import_main = self.import_main
        losses_by_months_df = group_losses_by_months(import_main, spl_keys, group_calc)
        spearman_rank_corr_matrix = dict()
        rank_arachnitude = dict()
        deg_freedom = dict()
        df_range = [3, 10]
        df_step = 0.5

        for splk in spl_keys:
            spearman_rank_corr_matrix[splk] = losses_by_months_df[splk].corr(method='spearman')
            corr_np = spearman_rank_corr_matrix[splk].to_numpy().astype(float)
            corr_optimized = nearest_nonnegative_psd(corr_np)
            spearman_rank_corr_matrix[splk] = pd.DataFrame(corr_optimized,
                                                           index=spearman_rank_corr_matrix[splk].index,
                                                           columns=spearman_rank_corr_matrix[splk].columns)

            rank_arachnitude[splk] = calculate_rank_arachnitude(losses_by_months_df[splk])
            rank_arachnitude_matrices = calculate_arachnitude_matrices(spearman_rank_corr_matrix[splk], df_range,
                                                                       df_step)
            odleglosci_macierzy = dict()

            for df in rank_arachnitude_matrices.keys():
                odleglosci_macierzy[df] = np.sum((rank_arachnitude[splk] - rank_arachnitude_matrices[df]) ** 2)

            lista_kluczy_df = np.array(list(odleglosci_macierzy.keys()))
            deg_freedom[splk] = lista_kluczy_df[list(odleglosci_macierzy.values()) ==
                                                np.min(list(odleglosci_macierzy.values()))]

        date = calc_date.replace("-", "_")

        res_path = os.path.join(out_path, "Wyniki")
        sym_cnt = str(int(sym_num / 1000)) + "K"

        if not os.path.exists(res_path):
            os.makedirs(res_path)

        cop_name = cop_type.upper().replace("-", "_")
        xlsx_path1 = os.path.join(res_path, 'Ryzyko operacyjne - wymogi_' + date + '_M_' + str(m_num) + '_S_' +
                                  sym_cnt + '_REP_' + str(rep_num) + "_COP_" + cop_name + '.xlsx')
        writer1 = pd.ExcelWriter(xlsx_path1, engine="xlsxwriter")
        workbook1 = writer1.book
        worksheet1 = workbook1.add_worksheet("Wymogi " + calc_date)
        worksheet1.set_column("A:A", 30)
        worksheet1.set_column("B:E", 17)
        worksheet1.set_row(0, 60)
        worksheet1.set_tab_color('#0083ff')
        bold = workbook1.add_format({'bold': True, 'align': 'center', 'valign': 'vcenter', 'border': 1,
                                     'font_size': 10, 'text_wrap': True, 'num_format': '# ### ### ##0.00'})
        bold_col = workbook1.add_format({'bold': True, 'align': 'center', 'valign': 'vcenter', 'border': 1,
                                         'font_size': 11, 'text_wrap': True, 'bg_color': '#03a544'})
        bold_col1 = workbook1.add_format({'bold': True, 'align': 'center', 'valign': 'vcenter', 'border': 1,
                                          'font_size': 11, 'text_wrap': True, 'bg_color': '#0070c0',
                                          'font_color': 'white'})
        bold_col2 = workbook1.add_format({'bold': True, 'align': 'center', 'valign': 'vcenter', 'border': 1,
                                          'font_size': 11, 'text_wrap': True, 'bg_color': '#7030a0',
                                          'font_color': 'white'})
        bold_col3 = workbook1.add_format({'bold': True, 'align': 'center', 'valign': 'vcenter', 'border': 1,
                                          'font_size': 11, 'text_wrap': True, 'bg_color': '#C0504D',
                                          'font_color': 'white'})
        bold_col4 = workbook1.add_format({'bold': True, 'align': 'center', 'valign': 'vcenter', 'border': 1,
                                          'font_size': 11, 'text_wrap': True, 'bg_color': '#002060',
                                          'font_color': 'white'})
        base0 = workbook1.add_format({'num_format': '# ### ### ##0.00', 'align': 'center', 'border': 1,
                                      'valign': 'vcenter'})
        perc = workbook1.add_format({'num_format': '0.00%', 'align': 'center', 'valign': 'vcenter', 'border': 1,
                                     'font_size': 10})

        # Dla każdej spółki przeliczamy wyniki przy pomocy kopuły i zapisujemy je do pliku Excel
        for splk in spl_keys:

            # Jeżeli chcemy przeprowadzić obliczenia dla Grupy to musimy zdefiniować liczbę kategorii ryzyka jako 14
            if group_calc and splk == 'Grupa PZU KOPUŁA':
                # Ustalama kategorię ryzyk
                kat_keys = [k2 for k2 in res_dict1['PZU SA'].keys()]
                len_kat = 14

                # Pobieramy straty wygenerowane dla poszczególnych kategorii ryzyka operacyjnego
                risk_data = np.asarray([c_dict['Losses'] for c_dict in res_dict1['PZU SA'].values()] +
                                       [c_dict['Losses'] for c_dict in res_dict1['PZU Życie SA'].values()]).transpose()

            else:
                # Ustalama kategorię ryzyk
                kat_keys = [k2 for k2 in res_dict1[splk].keys()]
                len_kat = len(kat_keys)

                # Pobieramy straty wygenerowane dla poszczególnych kategorii ryzyka operacyjnego
                risk_data = np.array([c_dict['Losses'] for c_dict in res_dict1[splk].values()]).transpose()

            # Definiujemy macierz korelacji w wielowymiarowym rozkładzie Gaussa z wczeniej zdefiniowanym parametrem
            corr_mat = np.ones((len_kat, len_kat)) * cop_corr
            np.fill_diagonal(corr_mat, 1.0)

            if not is_psd(corr_mat):
                raise ValueError('Macierz kowariancji nie jest dodatnio półokreslona dla współczynnika kowariancji' +
                                 ' równego ' + str(cop_corr))

            curr_means = [0] * len_kat
            copula_unif = np.array([])

            if cop_type == "Gaussa":
                # Generujemy liczby losowe z wielowymiarowego rozkładu normalnego i "ujednostajniamy" je przy pomocy
                # dystrybuanty standardowego rozkładu normalnego (CDF)
                copula_unif = st.norm.cdf(np.random.multivariate_normal(curr_means, corr_mat, sym_num))

            elif cop_type == "T-Studenta":
                # Generujemy liczby losowe z wielowymiarowego rozkładu T-Studenta i "ujednostajniamy" je przy pomocy
                # dystrybuanty rozkładu t studenta
                corr_mat = spearman_rank_corr_matrix[splk]
                df_num = deg_freedom[splk]
                copula_unif = t_copula_unif(corr_mat, df_num, sym_num)

            elif cop_type == "Claytona":
                copula_unif = clayton_copula_unif(len_kat, cop_alpha, sym_num)

            elif cop_type == "Gumbela":
                copula_unif = gumbel_copula_unif(len_kat, cop_alpha, sym_num)

            elif cop_type == "Franka":
                copula_unif = frank_copula_unif(len_kat, cop_alpha, sym_num)

            # Dla każdego segmentu ryzyka operacyjnego tworzymy rangi z jego rozkładu i sumujemy otrzymane wartosci
            # otrzymując  łączny rozkład strat z kopuły
            risk_data.sort(axis=0)  # TT 2020.11.: przed polaczeniem kopula symulacje na brzegach trzeba posortowac
            joint_distribution = risk_data[rankdata(copula_unif, axis=0).astype(int) - 1, np.arange(len_kat)]
            np.savetxt(os.path.join(out_path, 'Wyniki//joint_distribution_multivariate_' + splk + '.csv'),
                       joint_distribution, delimiter=",")
            joined_dist = np.sum(joint_distribution, axis=1)

            # Wybieramy "op_risk_perc" percentyl z łącznego rozkładu strat
            fin_risk = np.percentile(joined_dist, op_risk_perc)
            curr_row0 = len(kat_keys)

            if splk == "PZU SA":
                curr_col = 1
                worksheet1.write(0, curr_col, splk, bold_col1)
            elif splk == 'PZU Życie SA':
                curr_col = 2
                worksheet1.write(0, curr_col, splk, bold_col2)
            elif splk == 'Grupa PZU':
                curr_col = 3
                worksheet1.write(0, curr_col, splk + "\n(Złączone dane wejściowe)", bold_col3)
            else:
                curr_col = 4
                worksheet1.write(0, curr_col, "Grupa PZU\n(Ryzyka złączone kopułą)", bold_col4)

            # Wypisujemy tylko dla pierwszej symulacji
            if curr_col == 1:
                worksheet1.write(0, 0, "Obliczenia na datę: " + calc_date, bold_col)
                [worksheet1.write(i + 1, 0, kat, bold) for i, kat in enumerate(kat_keys)]
                worksheet1.write(curr_row0 + 2, 0, "Ryzyko operacyjne " + "niezdywersyfikowane", bold)
                worksheet1.write(curr_row0 + 3, 0, "Ryzyko operacyjne " + "zdywersyfikwane", bold)
                worksheet1.write(curr_row0 + 4, 0, "Wskaźnik dywersyfikacji", bold)
                worksheet1.write(curr_row0 + 6, 0, "Formuła Standardowa", bold)
                worksheet1.write(curr_row0 + 7, 0, "Model wewnętrzny / Formuła Standardowa", bold)

            # Wypisujemy wartości ryzyk dla poszczególnych kategorii
            [worksheet1.write(i + 1, curr_col, np.percentile(risk_data[:, i], op_risk_perc) if
            splk != 'Grupa PZU KOPUŁA' else "-", base0) for i, kat in enumerate(kat_keys)]

            sum_risk = np.sum(np.percentile(risk_data, op_risk_perc, axis=0))
            div_ind = 1 - fin_risk / sum_risk
            worksheet1.write(curr_row0 + 2, curr_col, sum_risk, base0)
            worksheet1.write(curr_row0 + 3, curr_col, fin_risk, bold)
            worksheet1.write(curr_row0 + 4, curr_col, div_ind, perc)

            sf_risk = sf_oprisk[splk] if splk != 'Grupa PZU KOPUŁA' else sf_oprisk['Grupa PZU']
            worksheet1.write(curr_row0 + 6, curr_col, sf_risk, base0)
            worksheet1.write(curr_row0 + 7, curr_col, fin_risk / sf_risk, perc)

        writer1.save()


def nearest_nonnegative_psd(corr_matrix):
    n = corr_matrix.shape[0]
    X = cp.Variable((n, n), symmetric=True)
    constraints = [
        X >> 0,
        X >= 0,
        cp.diag(X) == 1
    ]
    problem = cp.Problem(
        cp.Minimize(cp.norm(X - corr_matrix, 'fro')),
        constraints
    )
    problem.solve(solver=cp.SCS, eps=1e-10, verbose=False)
    corr_optimized = np.maximum(X.value, 0)
    np.fill_diagonal(corr_optimized, 1)
    return corr_optimized


def is_psd(a, tol=1e-8):
    """ Funkcja sprawdzająca czy macierz jest dodatnio półokreslona """

    e, _ = np.linalg.eigh(a)
    return np.all(e > -tol)


def group_losses_by_months(import_main, spl_keys, group_calc):
    dict_losses = import_main.op_risk_d
    daty_all = dict()
    losses_aggregated_by_months = dict()
    losses_counts = dict()
    losses_by_mths_df = dict()

    for splk in spl_keys:
        kat_keys = [k2 for k2 in dict_losses[splk].keys()]
        losses_aggregated_by_months[splk] = dict()
        losses_counts[splk] = dict()
        lista_dat = []

        for kat in kat_keys:
            lista_dat = lista_dat + (dict_losses[splk][kat]['Dates']).tolist()
            daty_all[splk] = dict()
            daty_all[splk]['wszystkie_daty'] = np.array(lista_dat)

        daty_all[splk]['min'] = np.min(daty_all[splk]['wszystkie_daty'])
        daty_all[splk]['max'] = np.max(daty_all[splk]['wszystkie_daty'])
        daty_all[splk]['miesiace'] = pd.date_range(daty_all[splk]['min'], daty_all[splk]['max'], freq='MS')

        for kat in kat_keys:
            daty_pom = dict_losses[splk][kat]['Dates']
            losses_pom = dict_losses[splk][kat]['Values']
            losses_aggregated_by_months[splk][kat] = np.array([np.sum(losses_pom[daty_pom == data])
                                                               for data in daty_all[splk]['miesiace']])
            losses_counts[splk][kat] = np.sum(losses_aggregated_by_months[splk][kat] > 0.0)

        losses_by_mths_df[splk] = pd.DataFrame.from_dict(losses_aggregated_by_months[splk])

    # Jeżeli chcemy przeprowadzić również obliczenia dla Grupy to musimy dodać klucz do listy spółek i połączyć
    # horyzontalnie dataframe'y miesięcznych strat z PZU SA i PZU Życie SA w jeden łączny dataframe
    if group_calc:
        # Dodajemy nową spółkę do listy spółek
        pzu_group_cop = 'Grupa PZU KOPUŁA'
        spl_keys += [pzu_group_cop]

        # Pobieramy dataframe'y miesięcznych strat dla PZU SA
        pzum_losses = losses_by_mths_df['PZU SA']

        # Tworzymy słownik zawierający stare i nowe nazwy kolumn
        pzum_new_names = {c_col: 'PZU SA: ' + c_col for c_col in list(pzum_losses)}

        # Zmieniamy nazwy kolumn
        pzum_losses.rename(columns=pzum_new_names, inplace=True)

        # Pobieramy dataframe'y miesięcznych strat dla PZU Życie SA
        pzuz_losses = losses_by_mths_df['PZU Życie SA']

        # Tworzymy słownik zawierający stare i nowe nazwy kolumn
        pzuz_new_names = {c_col: 'PZU Życie SA: ' + c_col for c_col in list(pzuz_losses)}

        # Zmieniamy nazwy kolumn
        pzuz_losses.rename(columns=pzuz_new_names, inplace=True)

        # Łączymy dataframe'y w jeden wspólny dataframe
        losses_by_mths_df[pzu_group_cop] = pd.concat([pzum_losses, pzuz_losses], axis=1)

    return losses_by_mths_df


def calculate_rank_arachnitude(d_frame):
    n_row = len(d_frame.index)
    n_kol = len(d_frame.columns)
    arachnitude_matrix = np.zeros((n_kol, n_kol))

    for i in range(n_kol):
        for j in range(n_kol):
            x = d_frame.iloc[:, i].rank() / n_row
            y = d_frame.iloc[:, j].rank() / n_row
            arachnitude_matrix[i, j] = np.corrcoef((2 * x - 1) ** 2, (2 * y - 1) ** 2)[0, 1]

    return arachnitude_matrix


def calculate_arachnitude_curves(correlation_range, corr_step, df_range, df_step):
    corr_min = correlation_range[0]
    corr_max = correlation_range[1]
    df_min = df_range[0]
    df_max = df_range[1]
    sample_size = 10000

    corr_steps_n = math.ceil((corr_max - corr_min) / corr_step)
    df_steps_n = math.ceil((df_max - df_min) / df_step)
    corr_steps = np.array([corr_min + i * corr_step for i in range(corr_steps_n)] + [corr_max])
    df_steps = np.array([df_min + i * df_step for i in range(df_steps_n)] + [df_max])
    arachnitude_matrix = np.zeros((corr_steps_n, df_steps_n))

    for i in range(corr_steps_n):
        for j in range(df_steps_n):
            mac_kor = np.array([[1, corr_steps[i]], [corr_steps[i], 1]])
            probka = t_copula_unif(mac_kor, df_steps[j], sample_size)
            arachnitude_matrix[i, j] = calculate_rank_arachnitude(pd.DataFrame(probka))[0, 1]

    return arachnitude_matrix


def calculate_arachnitude_matrices(correlation_matrix, df_range, df_step):
    df_min = df_range[0]
    df_max = df_range[1]
    sample_size = 10000
    df_steps_n = math.ceil((df_max - df_min) / df_step)
    df_steps = np.array([df_min + i * df_step for i in range(df_steps_n)] + [df_max])
    arachnitude_matrices = dict()

    for df in df_steps:
        probka = t_copula_unif(correlation_matrix, df, sample_size)
        arachnitude_matrices[df] = calculate_rank_arachnitude(pd.DataFrame(probka))

    return arachnitude_matrices


def t_copula_unif(sigma, df, n_size):
    """ Funkcja zwracająca wektor liczb losowych z wielowymiarowego rozkładu t-studenta ujednostajnionych dystrubuantą
    rozkładu t-studenta """

    sample1 = st.multivariate_normal.rvs(cov=sigma, size=n_size)
    sample2 = np.tile(st.chi2.rvs(df=df, size=n_size), (sigma.shape[0], 1)).T
    sample2_ = np.sqrt(df / sample2)
    sample_t = sample2_ * sample1

    return st.t.cdf(sample_t, df=df)


def clayton_copula_unif(n, alpha, m):
    """ Funkcja zwracająca wektor ujednostajnionych liczb losowych z kopuły Claytona """

    if alpha < 0:
        raise ValueError('Alpha must be >=0 for Clayton Copula Family')
    if n < 2:
        raise ValueError('Dimensionality Argument [N] must be an integer >= 2')
    elif n == 2:
        u1 = st.uniform.rvs(size=m)
        p = st.uniform.rvs(size=m)
        if alpha < np.spacing(1):
            u2 = p
        else:
            u2 = u1 * np.power((np.power(p, (-alpha / (1.0 + alpha))) - 1 + np.power(u1, alpha)), (-1.0 / alpha))

        u = np.column_stack((u1, u2))
    else:
        # Algorithm 1 described in both the SAS Copula Procedure, as well as the
        # paper: "High Dimensional Archimedean Copula Generation Algorithm"
        u = np.empty((m, n))
        for ii in range(0, m):
            shape = 1.0 / alpha
            v = st.gamma.rvs(shape)

            # sample N independent uniform random variables
            x_i = st.uniform.rvs(size=n)
            t = -1 * np.log(x_i) / v
            if alpha < 0:
                tmp = np.maximum(0, 1.0 - t)
            else:
                tmp = 1.0 + t

            u[ii, :] = np.power(tmp, -1.0 / alpha)

    return u


def gumbel_copula_unif(n, alpha, m):
    """ Funkcja zwracająca wektor ujednostajnionych liczb losowych z kopuły Gumbela """

    if alpha < 1:
        raise ValueError('Alpha must be >=1 for Gumbel Copula Family!')
    if n < 2:
        raise ValueError('Dimensionality Argument [N] must be an integer >= 2')
    elif n == 2:
        if alpha < (1 + math.sqrt(np.spacing(1))):
            u1 = st.uniform.rvs(size=m)
            u2 = st.uniform.rvs(size=m)
        else:
            # Use the Marshal-Olkin method
            # Generate gamma as Stable(1/alpha,1), c.f. Devroye, Thm. IV.6.7
            # Generate M uniformly distributed RV's between -pi/2 and pi/2
            u = (st.uniform.rvs(size=m) - 0.5) * math.pi
            u2 = u + math.pi / 2
            e = -1 * np.log(st.uniform.rvs(size=m))
            t = np.cos(u - u2 / alpha) / e
            gamma = np.power(np.sin(u2 / alpha) / t, (1.0 / alpha)) * t / np.cos(u)

            # Frees&Valdez, eqn 3.5
            u1 = np.exp(-1 * (np.power(-1 * np.log(st.uniform.rvs(size=m)), 1.0 / alpha) / gamma))
            u2 = np.exp(-1 * (np.power(-1 * np.log(st.uniform.rvs(size=m)), 1.0 / alpha) / gamma))

        u = np.column_stack((u1, u2))
    else:
        # Algorithm 1 described in both the SAS Copula Procedure, as well as the
        # paper: "High Dimensional Archimedean Copula Generation Algorithm"
        u = np.empty((m, n))

        for ii in range(0, m):
            a = 1.0 / alpha
            b = 1
            g = np.power(np.cos(math.pi / (2.0 * alpha)), alpha)
            d = 0
            pm = 1
            v = rstable1(1, a, b, g, d, pm)

            # Sample N independent uniform random variables
            x_i = st.uniform.rvs(size=n)
            t = -1 * np.log(x_i) / v

            u[ii, :] = np.exp(-1 * np.power(t, 1.0 / alpha))

    return u


def frank_copula_unif(n, alpha, m):
    """ Funkcja zwracająca wektor ujednostajnionych liczb losowych z kopuły Franka """

    if n < 2:
        raise ValueError('Dimensionality Argument [N] must be an integer >= 2')
    elif n == 2:
        u1 = st.uniform.rvs(size=m)
        p = st.uniform.rvs(size=m)
        if abs(alpha) > math.log(sys.float_info.max):
            u2 = (u1 < 0).astype(int) + np.sign(alpha) * u1  # u1 or 1-u1
        elif abs(alpha) > math.sqrt(np.spacing(1)):
            u2 = -1 * np.log((np.exp(-alpha * u1) * (1 - p) / p + np.exp(-alpha)) / (1 + np.exp(-alpha * u1) *
                                                                                     (1 - p) / p)) / alpha
        else:
            u2 = p

        u = np.column_stack((u1, u2))
    else:
        # Algorithm 1 described in both the SAS Copula Procedure, as well as the
        # paper: "High Dimensional Archimedean Copula Generation Algorithm"
        if alpha <= 0:
            raise ValueError('For N>=3, alpha > 0 in Frank Copula')

        u = np.empty((m, n))

        for ii in range(0, m):
            p = -1.0 * np.expm1(-1 * alpha)
            if p == 1:
                # Boundary case protection
                p = 1 - np.spacing(1)
            v = st.logser.rvs(p, size=1)

            # Sample N independent uniform random variables
            x_i = st.uniform.rvs(size=n)
            t = -1 * np.log(x_i) / v
            u[ii, :] = -1.0 * np.log1p(np.exp(-t) * np.expm1(-1.0 * alpha)) / alpha

    return u


# Dodatkowe funkcje pomocnicze
def rstable1(n, alpha, beta, gamma=1, delta=0, pm=1):
    """ Funkcja pomocnicza do wyliczania kopuły Gumbela """
    _beta = beta
    _pm = pm
    return _rstable_c(n, alpha) * gamma + delta


def _rstable_c(n, alpha):
    """ Funkcja pomocnicza do wyliczania kopuły Gumbela """
    _n = n
    return np.power(np.cos(math.pi / 2.0 * alpha), -1.0 / alpha) * _rstable0(alpha)


def _rstable0(alpha):
    """ Funkcja pomocnicza do wyliczania kopuły Gumbela """
    u = st.uniform.rvs(size=1)
    while True:
        # generate non-zero exponential random variable
        w = st.expon.rvs(size=1)
        if w != 0:
            break
    return np.power(_a(math.pi * u, alpha) / np.power(w, 1.0 - alpha), 1.0 / alpha)


def _a(x, alpha):
    """ Funkcja pomocnicza do wyliczania kopuły Gumbela """

    i_alpha = 1.0 - alpha
    return _a_3(x, alpha, i_alpha)


def _a_3(x, alpha, i_alpha):
    """ Funkcja pomocnicza do wyliczania kopuły Gumbela """

    return np.power(i_alpha * np.sinc(i_alpha * x / math.pi), i_alpha) * np.power(alpha * np.sinc(alpha * x / math.pi),
                                                                                  alpha) / np.sinc(x / math.pi)
