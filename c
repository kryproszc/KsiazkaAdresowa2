""" Symulation module """

import math
import os
import warnings

import numpy as np
import pandas as pd
import scipy.optimize as so
import scipy.stats as st
from lmoments3 import distr
from scipy.special import factorial
from scipy.special import gammaln
from scipy.stats import nbinom
from scipy.stats import poisson

# Pomijamy ostrzeżenia związane ze zbyt długim obliczaniem przy poszukiwaniu MLE
warnings.filterwarnings("ignore", category=RuntimeWarning)


class Symulacja(object):
    """ Symulation class """

    def __init__(self, data_import=None, sym_num=0, par_fit_met="", ex_num=0, m_num=0, op_risk_perc=0.0,
                 thres_choose="", cut_perc=0.0, par_p=0.0, rep_num=0, order_stats_dict=None, sro_use=''):
        self.data_import = data_import
        self.order_stats_dict = order_stats_dict
        self.sym_num = sym_num
        self.par_fit_met = par_fit_met
        self.m_num = m_num
        self.ex_num = ex_num
        self.op_risk_perc = op_risk_perc
        self.thres_choose = thres_choose
        self.thres = []
        self.cut_perc = cut_perc
        self.par_p = par_p
        self.res_dict1 = {}
        self.spl_keys = []
        self.rep_num = rep_num
        self.sro_use = sro_use
        self.symuluj()

    def symuluj(self):
        """ Symulation function """
        np.random.seed(0)
        data_import = self.data_import
        sro_use = self.sro_use
        in_path = data_import.in_path
        op_risk_d = data_import.op_risk_d

        if sro_use == 'ZRO_included':
            sro_dict = data_import.sro_summary_dict
        else:
            sro_dict = data_import.SRO_dict_by_Entity_and_Basel_category

        sro_map = create_sro_map()
        scen_params = data_import.scenarios_params
        sym_num = self.sym_num
        m_num = self.m_num
        ex_num = self.ex_num
        par_fit_met = self.par_fit_met
        op_risk_perc = self.op_risk_perc
        thres_choose = self.thres_choose
        cut_perc = self.cut_perc
        sro_params = pd.DataFrame(columns=['Spolka', 'Kategoria', 'Freq', 'ce_val'])
        self.get_threshold_values()
        thres = self.thres
        par_p = self.par_p
        rep_num = self.rep_num
        lgnm_ksz = 0.0
        lgnm_skal = 0.0
        gpar_ksz = 0.0
        gpar_lok = 0.0
        gpar_skal = 0.0

        # Sprawdzamy czy plik .npy jest już w folderze jeżeli tak to pomijamy procedurę ponownego wczytywania danych
        if not os.path.isfile(in_path + "ryzyka_standalone.npy"):

            res_dict = {}
            prog_licz = 0

            for k1, v1 in op_risk_d.items():
                res_dict[k1] = {}

                for k2, v2 in v1.items():
                    freq_vals = v2['Freq_vals']
                    dane = v2['Values']
                    len_dane = len(dane)
                    monthly_dane = v2['Mnthly_losses']
                    res_dict[k1][k2] = {}

                    # Gdy dla danej kategorii ryzyka dostępnych jest w bazie mniej niż 10 strat to jako rozkład strat
                    # dla danej kategorii przyjmuje się rozkład strat z drugiej spółki
                    if len_dane < 10:
                        spolki2 = list(op_risk_d.keys())
                        idx = spolki2.index(k1)
                        spolki2.pop(idx)
                        dane = op_risk_d[spolki2[0]][k2]['Values']
                        freq_vals = op_risk_d[spolki2[0]][k2]['Freq_vals']
                        len_dane = len(dane)
                        monthly_dane = op_risk_d[spolki2[0]][k2]['Mnthly_losses']

                        # Jeżeli dla danej kategorii ryzyka, w bazie dla drugiej spółki, dostępnych jest również mniej
                        # niż 10 obserwacji to przyjmuje się, że straty z danej kategorii nie będą modelowane
                        if len_dane < 10:
                            res_dict[k1][k2]['Risk 1:200'] = 0.0
                            res_dict[k1][k2]['Losses'] = np.array([])
                            prog_licz += 1
                            continue

                    # 3.3.1 W przypadku, gdy dostępnych jest co najmniej 30 obserwacji wyznacza się próg odcięcia
                    # dużych strat d_k w oparciu o analizę wykresów Hill plot oraz ME plot
                    if thres_choose == "Percentile":
                        curr_prog = np.percentile(dane, cut_perc)
                    else:
                        curr_prog = thres[prog_licz]

                    # Liczymy parametry pogrubionych ogonów tylko w sytuacji gdy mamy więcej niż 30 obserwacji w bazie
                    if len_dane > 30:
                        # 3.3.2 Dopasowujemy uogólniony rozkład Pareto do obserwacji powyżej progu d_k. Przez Y
                        # oznaczamy zmienną o tym rozkładzie
                        ogon_dane = dane[dane >= curr_prog]

                        if par_fit_met == "MLE":
                            # Dopasowujemy Uogólniony rozkład Pareto dla ogona metodą logarytmu największej
                            # wiarygodnosci
                            gpar_ksz, gpar_lok, gpar_skal = st.genpareto.fit(ogon_dane, floc=curr_prog,
                                                                             optimizer=opti_fit)
                        elif par_fit_met == "L-moments":
                            # Dopasowujemy Uogólniony rozkład Pareto dla ogona metodą logarytmowanych momentów
                            paras = distr.gpa.lmom_fit(ogon_dane)
                            gpar_ksz = paras['c']
                            gpar_lok = curr_prog
                            gpar_skal = paras['scale']

                    perc_arr = np.zeros(rep_num)
                    losses_arr = np.zeros((rep_num, sym_num))
                    exp_freq_ests = 0
                    ce_val = 0

                    for rep_n in range(rep_num):
                        # 3.3.3. Z obserwacji zgormadzonych w bazie losujemy M próbek x^((i))={x_1^((i) ),…,x_60^((i))},
                        # i=1, …, M, z których każda zawiera straty dla danej kategorii ryzyka zarejestrowane w losowo
                        # wybranych 60 miesięcznych podokresach.
                        len_monthly_dane = len(monthly_dane)
                        rnd_months = np.random.randint(0, len_monthly_dane, size=(m_num, 60))
                        loss_arr_temp = monthly_dane[rnd_months]
                        xi_arr = [np.hstack(arr) for arr in loss_arr_temp]
                        freq_arr = freq_vals[rnd_months]

                        # 3.3.4.3 Załóżmy, że dysponujemy opiniami ekspertów N_1^e,… N_(k_e)^e o spodziewanej liczbie
                        # strat operacyjnych z danej kategorii ryzyka, którą wyznaczono poprzez sumowanie prospektywnej
                        # oceny ryzyka w zakresie liczby zdarzeń dla poszczególnych typów zdarzeń wskazanych przez
                        # danego eksperta w ramach samooceny. Zakładamy, że opinie ekspertów są niezależne
                        sro_res = sro_dict[k1][sro_map[k2]]
                        sro_estims_per_cat = pd.DataFrame(columns=list(sro_res.columns))
                        exp_freq_ests = 0

                        # Sumujemy czestotliwości podawane przez ekspertów z biur wiodacych
                        for file_name, val_df in sro_res.items():
                            if sro_use != 'ZRO_included':
                                main_offices = get_main_offices(k1, k2)

                                if main_offices[0] == "ALL":
                                    exp_freq_ests += val_df.values[:, 1].sum()
                                else:
                                    for P_off in main_offices:
                                        if P_off in file_name:
                                            exp_freq_ests += val_df.values[:, 1].sum()
                                            break

                                sro_estims_per_cat = sro_estims_per_cat.append(val_df, ignore_index=True)

                        sro_path = os.path.join(in_path, "SRO_aggregated")

                        if not os.path.exists(sro_path):
                            os.makedirs(sro_path)

                        # Zapisujemy parametry SRO do pliku
                        sro_estims_per_cat.to_excel(os.path.join(sro_path, k1 + "_" + k2 + "_sro_expert_op.xlsx"))

                        # Zaokrąglamy czestotliwości w górę, żeby wszystkie częstotliwości były reprezentowane przez
                        # liczby całkowite
                        exp_freq_ests = math.ceil(exp_freq_ests)

                        # Listy parametrów
                        freq_prms_list = []
                        probs_arr = np.zeros(m_num)
                        sev_prms_list = []

                        for i, curr_freqs in enumerate(freq_arr):

                            # 3.3.4 Estymacja częstości:
                            freq_mean = curr_freqs.mean()
                            freq_var = curr_freqs.var()

                            # 3.3.4.1 Dla każdego i=1, …, M: estymujemy parametry θ^((i)) rozkładu częstości (Poissona
                            # lub ujemnego dwumianowego w przypadku, gdy wariancja liczby strat w miesiącu przekracza
                            # średnią o co najmniej 10%) o gęstości f(n│θ=θ^((i)))
                            if freq_var <= freq_mean * 1.10:

                                # Wyrażamy średnią częstotliwość zgodnie ze skalą do której przeskalowaliśmy
                                # czestotliwości podane przez ekspertów i uraczniamy je mnozac przez 12
                                freq_mean_scld = freq_mean * 12

                                # 3.3.4.4 Wyznaczamy rozkład a posteriori na zbiorze Θ zgodnie ze wzorem
                                probs_arr[i] = poisson.pmf(exp_freq_ests, freq_mean_scld)
                                freq_prms_list.append(("Poisson", freq_mean_scld))
                            else:
                                nb_params = fit_nbinom(curr_freqs)

                                # Wyrażamy liczbę sukcesów zgodnie ze skalą do której przeskalowaliśmy
                                # czestotliwości podawane przez ekspertów i uraczniamy je mnozac przez 12
                                n_val = nb_params['size'] * 12
                                p_val = nb_params['prob']
                                freq_prms_list.append(("Negative Binomial", n_val, p_val))

                                # 3.3.4.4 Wyznaczamy rozkład a posteriori na zbiorze Θ zgodnie ze wzorem
                                probs_arr[i] = nbinom.pmf(exp_freq_ests, n_val, p_val)

                            # 3.3.5.1 Estymujemy rozkład lognormalny LN(μ_i, σ_i^2) w oparciu o ograniczoną do
                            # obserwacji poniżej progu d_k próbkę x^((i)) metodą L-momentów (korekta - dopasowujemy LN
                            # do całej próbki, bo bez tego za rzadko trafiały się obserwacje powyżej progu)
                            curr_xi = xi_arr[i]

                            if par_fit_met == "MLE":
                                lgnm_ksz, _, lgnm_skal = st.lognorm.fit(curr_xi, optimizer=opti_fit)
                            elif par_fit_met == "L-moments":
                                paras0 = distr.nor.lmom_fit(np.log(curr_xi))
                                lgnm_skal = np.exp(paras0['loc'])
                                lgnm_ksz = paras0['scale']

                            sev_prms_list.append((lgnm_ksz, lgnm_skal))

                        # Macierz wag dopasowanych rozkładów
                        probs_arr[probs_arr == 0] = np.finfo(probs_arr.dtype).tiny
                        params_wghts = probs_arr / probs_arr.sum()

                        # 3.3.4.5. Definiujemy rozkład liczby strat operacyjnych z danej kategorii ryzyka jako
                        # predykcyjny rozkład aposteriori
                        freq_prms_inds = np.random.choice(m_num, m_num, p=params_wghts, replace=True)

                        # 3.3.5.3.2. Załóżmy, że dla każdej jednostki j=1,…,t_SRO w ramach samooceny pozyskano
                        # oszacowania częstotliwości oraz dotkliwości dotyczące l_j typów incydentów. Dla i=1,…,l_j
                        # niech N_(j,i) będzie zmienną o rozkładzie Poissona reprezentującą liczbę strat dopasowaną do
                        # oszacowania częstości podanej przez jednostkę j dla incydentu typu i. Niech
                        # 〖〖(X〗_(j,i,l))〗_(l=1)^∞ oznacza ciąg niezależnych zmiennych losowych o jednakowym
                        # rozkładzie dopasowanym do oszacowania dotkliwości podanego przez jednostkę j dla typu
                        # incydentu i.
                        jnd_exp_ls = np.zeros(ex_num)

                        for sro_arr in sro_res.values:

                            sro_arr = np.expand_dims(sro_arr, axis=0)

                            for row in sro_arr:
                                l_zdarz = row[1]
                                przec_wart = row[2]
                                najw_strat = row[3]

                                if isinstance(l_zdarz, str):
                                    continue

                                liczba_szkod0 = np.random.poisson(l_zdarz, ex_num)

                                if isinstance(przec_wart, str) or np.isnan(przec_wart) or przec_wart == 0:
                                    continue

                                elif not isinstance(najw_strat, str) and not np.isnan(najw_strat) and \
                                        najw_strat > 0 and najw_strat > przec_wart:
                                    skal_par1 = np.log(przec_wart)
                                    lambd_val = math.ceil(5 * l_zdarz)
                                    alpha = 0.5
                                    lgn_val = st.lognorm.ppf(1 + (np.log(alpha) / lambd_val), s=1)
                                    ksz_par1 = (np.log(najw_strat) - skal_par1) / lgn_val

                                    jnd_exp_ls += np.array([st.lognorm.rvs(s=ksz_par1, scale=np.exp(skal_par1),
                                                                           size=l_szkod).sum()
                                                            for l_szkod in liczba_szkod0])
                                else:
                                    jnd_exp_ls += liczba_szkod0 * przec_wart

                        # 3.3.5.3.4. Wyznaczamy oszadowanie kapitału w danej kategorii ryzyka c^e, jako kwantyl rzędu
                        # α rozkładu zmiennej S^e.
                        ce_val = np.percentile(jnd_exp_ls, op_risk_perc)

                        # 3.3.5. Estymacja dotkliwości strat poniżej progu:
                        ci_b = np.zeros(m_num)

                        for j, ind in enumerate(freq_prms_inds):
                            dist_name = freq_prms_list[ind][0]

                            if dist_name == "Poisson":
                                curr_fq_mean = freq_prms_list[ind][1]
                                liczba_szkod = np.random.poisson(curr_fq_mean, m_num)
                            else:
                                curr_n_val = freq_prms_list[ind][1]
                                curr_p_val = freq_prms_list[ind][2]
                                liczba_szkod = np.random.negative_binomial(curr_n_val, curr_p_val, m_num)

                            # Wybieramy z listy odpowiednie parametry rozkładu log-normalnego
                            lgnm_ksz0 = sev_prms_list[ind][0]
                            lgnm_skal0 = sev_prms_list[ind][1]

                            # Tworzymy macierz zagregowanych rocznych strat
                            agg_loss = np.array([st.lognorm.rvs(lgnm_ksz0, scale=lgnm_skal0, size=l_szkod).sum()
                                                 for l_szkod in liczba_szkod])

                            # 3.3.5.2 Wyznaczamy oszacowanie kapitału c_i^b na ryzyko danej kategorii, jako kwantyl
                            # rzędu α rozkładu złożonego, przy czym jako rozkład częstości przyjmujemy predykcyjny
                            # rozkład aposteriori, wskazany w punkcie 4.5.
                            ci_b[j] = np.percentile(agg_loss, op_risk_perc)

                        # 3.3.5.4.1.Wyznaczamy zbiór odległości
                        dists_set = np.abs(ce_val - ci_b)
                        dists_set.sort()
                        d_p = dists_set[:np.ceil(par_p * m_num).astype(int)].max()

                        # 3.3.5.5 Niech U_p będzie zmienną o rozkładzie jednostajnym na zbiorze zbiorze U_p. Przez X
                        # oznaczamy zmienną o rozkładzie dotkliwości
                        up_set_inds = np.where(dists_set <= d_p)
                        up_set_params = np.array(sev_prms_list)[up_set_inds]
                        up_set_len = len(up_set_params)

                        # 3.3.6. Definiujemy zmienną o rozkładzie dotkliwości z pogrubionym ogonem
                        lk_loss = np.zeros(sym_num)

                        for k in range(sym_num):
                            rnd_num0 = np.random.randint(0, m_num)
                            curr_freq = freq_prms_list[freq_prms_inds[rnd_num0]]
                            dist_name2 = curr_freq[0]

                            if dist_name2 == "Poisson":
                                curr_fq_mean2 = curr_freq[1]
                                rocz_l_szkod = np.random.poisson(curr_fq_mean2)
                                # Dwanaście, bo zależy nam na rocznych stratach, równie dobrze moglibyśmy losować 12
                                # razy z tego rozkładu, ale to byłoby bardziej złożone obliczeniowo
                            else:
                                curr_n_val2 = curr_freq[1]
                                curr_p_val2 = curr_freq[2]
                                rocz_l_szkod = np.random.negative_binomial(curr_n_val2, curr_p_val2)
                                # Dwanaście, bo zależy nam na rocznych stratach, równie dobrze moglibyśmy losować 12
                                # razy z tego rozkładu, ale to byłoby bardziej złożone obliczeniowo

                            # Losujemy indeks z macierzy up_set_params
                            rnd_num = np.random.randint(0, up_set_len)

                            # Dla wylosowanego indeksu wybieramy parametry kształtu i skali
                            x_kszt = up_set_params[rnd_num, 0]
                            x_skal = up_set_params[rnd_num, 1]

                            zk_losses = st.lognorm.rvs(x_kszt, scale=x_skal, size=rocz_l_szkod)

                            # Pogrubiamy ogony tylko jak mamy więcej niż 30 obserwacji w bazie
                            if len_dane > 30:
                                ot_losses_mask = zk_losses > curr_prog
                                ot_l_sz = ot_losses_mask.sum()
                                gen_par_losses = st.genpareto.rvs(gpar_ksz, loc=gpar_lok, scale=gpar_skal, size=ot_l_sz)
                                zk_losses[ot_losses_mask] = gen_par_losses

                            zk_loss = zk_losses.sum()

                            # 3.3.7 Załóżmy, że zdefiniowano m^e  zdarzeń dotkliwych (binary events), z których straty
                            # reprezentowane są przez zmienne losowe X_k^e, k=1, …, m^e, które są niezależne od siebie
                            # oraz od pozostałych zmiennych występujących w modelu
                            be_scens = scen_params.values
                            xk_loss = 0
                            curr_be_scens = be_scens[be_scens[:, 2] == k2]

                            for row1 in curr_be_scens:
                                if row1[3] == k1 or row1[3] == 'Unia':
                                    rozkl_name = row1[4]
                                    curr_czest = row1[5]
                                    avg_sev = row1[6]
                                    max_sev = row1[7]

                                    if rozkl_name == "Poissona":
                                        l_zdarz = np.random.poisson(curr_czest, 1)[0]

                                        if max_sev == 0:
                                            xk_loss += l_zdarz * avg_sev
                                        else:
                                            skal_par5 = np.log(avg_sev)
                                            lgn_val = st.norm.ppf(0.5 ** 0.05)
                                            ksz_par5 = (np.log(max_sev) - skal_par5) / lgn_val
                                            xk_loss += st.lognorm.rvs(s=ksz_par5, scale=np.exp(skal_par5),
                                                                      size=l_zdarz).sum()

                                    elif rozkl_name == "Bernoulliego" and np.random.binomial(1, curr_czest):
                                        if max_sev == 0:
                                            xk_loss += avg_sev
                                        else:
                                            skal_par6 = np.log(avg_sev)
                                            lgn_val = st.norm.ppf(0.5 ** 0.05)
                                            ksz_par6 = (np.log(max_sev) - skal_par6) / lgn_val
                                            xk_loss += st.lognorm.rvs(s=ksz_par6, scale=np.exp(skal_par6), size=1)[0]

                            # 3.3.8. Rozkład strat w danej kategorii ryzyka jest rozkładem zmiennej...
                            lk_loss[k] = zk_loss + xk_loss

                        # Wyznaczamy finalne wartości
                        perc_arr[rep_n] = np.percentile(lk_loss, op_risk_perc)
                        losses_arr[rep_n] = lk_loss

                    loss_mode = np.percentile(perc_arr, 50.0)
                    diff = np.abs(perc_arr - loss_mode)
                    small_ind = np.argmin(diff)
                    res_dict[k1][k2]['Risk 1:200'] = perc_arr[small_ind]
                    res_dict[k1][k2]['Losses'] = losses_arr[small_ind]
                    prog_licz += 1
                    sro_series = pd.Series([k1, k2, exp_freq_ests, ce_val], index=sro_params.columns)
                    sro_params = sro_params.append(sro_series, ignore_index=True)

            sro_params.to_excel(in_path + "sro_estims_df.xlsx")
            np.save(in_path + "ryzyka_standalone.npy", res_dict)

        # noinspection PyTypeChecker
        self.res_dict1 = np.load(in_path + "ryzyka_standalone.npy").item()  # type: dict
        self.spl_keys = list(self.res_dict1.keys())

    def get_threshold_values(self):

        thres = []
        op_risk_d = self.data_import.op_risk_d
        order_stats_dict = self.order_stats_dict

        for k1, v1 in op_risk_d.items():
            for k2, v2 in v1.items():
                order_stat_number = order_stats_dict[k1][k2]
                dane = v2['Values']
                dane_sorted_desc = -np.sort(-dane)
                thres.append(dane_sorted_desc[order_stat_number])

        self.thres = thres


def create_sro_map():
    """ Funkcja tworząca mapping między SRO a nazwami używanymi w biezacej implementacji """

    return {"Oszustwo wewnętrzne": '1. oszustwa wewnętrzne', "Oszustwo zewnętrzne": '2. oszustwa zewnętrzne ',
            "Praktyka kadrowa i bezpieczeństwo pracy": '3. praktyka kadrowa ',
            "Klienci, produkty i praktyka biznesowa": '4. klienci, produkty ',
            "Uszkodzenia aktywów": '5. uszkodzenie aktywów ',
            "Zakłócenia działalności i błędy systemów": '6. zakłócenia działalności ',
            "Dokonywanie transakcji, dostawa oraz zarządzanie procesami": '7. dokonywanie transakcji '}


def get_main_offices(splk, op_risk_kat):
    """ Funkcja  wskazująca na wiodące biura przy określaniu strat w SRO """

    if splk == "P SA":
        return get_main_offices_Pm(op_risk_kat)
    else:
        return get_main_offices_Pz(op_risk_kat)


def get_main_offices_Pm(op_risk_kat):
    """ Funkcja  wskazująca na wiodące biura przy określaniu strat w SRO """

    if op_risk_kat == "Oszustwo wewnętrzne" or op_risk_kat == "Oszustwo zewnętrzne":
        return ["BBE", "BZU", "BNI"]
    elif op_risk_kat == "Oszustwo wewnętrzne" or op_risk_kat == "Oszustwo zewnętrzne":
        return ["BBE", "BZU", "BNI"]
    elif op_risk_kat == "Praktyka kadrowa i bezpieczeństwo pracy":
        return ["PHR", "ZBHP"]
    elif op_risk_kat == "Zakłócenia działalności i błędy systemów":
        return ["PTI", "BNI"]
    else:
        return ["ALL"]


def get_main_offices_Pz(op_risk_kat):
    """ Funkcja  wskazująca na wiodące biura przy określaniu strat w SRO """

    if op_risk_kat == "Oszustwo wewnętrzne" or op_risk_kat == "Oszustwo zewnętrzne":
        return ["BBE", "BZU", "BNI"]
    elif op_risk_kat == "Oszustwo wewnętrzne" or op_risk_kat == "Oszustwo zewnętrzne":
        return ["BBE", "BZU", "BNI"]
    elif op_risk_kat == "Praktyka kadrowa i bezpieczeństwo pracy":
        return ["PHR", "ZBHP"]
    elif op_risk_kat == "Zakłócenia działalności i błędy systemów":
        return ["PTI", "BNI"]
    else:
        return ["ALL"]


def opti_fit(fun, x0, args):
    """ Optimizer for fiting log-normal distribution """

    return so.minimize(fun, x0, args=args, method='SLSQP', tol=1e-16, options={'maxiter': 100000}).x


# X is a numpy array representing the data initial params is a numpy array representing the initial values of size and
# prob parameters
def fit_nbinom(x, initial_params=None):
    """ Funkcja dopasowująca parametry rozkładu ujemnie dwumianowego do próbki """

    infinitesimal = np.finfo(np.float).eps

    def log_likelihood(params1, *args):
        """ log_likelihood calculation """

        r, p = params1
        x1 = args[0]
        n = x1.size
        sum0 = np.sum(gammaln(x1 + r))
        sum1 = np.sum(np.log(factorial(x1)))
        sum2 = np.sum(x1 * np.log(1 - (p if p < 1 else 1 - infinitesimal)))
        result = sum0 - sum1 - n * (gammaln(r)) + n * r * np.log(p) + sum2
        return -result

    if initial_params is None:
        # Reasonable initial values (from fitdistr function in R)
        m = np.mean(x)
        v = np.var(x)
        size = (m ** 2) / (v - m) if v > m else 10

        # Convert mu/size parameterization to prob/size
        p0 = size / ((size + m) if size + m != 0 else 1)
        r0 = size
        initial_params = np.array([r0, p0])

    bounds = [(infinitesimal, None), (infinitesimal, 1)]
    optimres = so.fmin_l_bfgs_b(log_likelihood, x0=initial_params, args=(x,), approx_grad=True, bounds=bounds)
    params = optimres[0]

    return {'size': params[0], 'prob': params[1]}
