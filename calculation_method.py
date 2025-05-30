import pandas as pd
import numpy as np
from numpy.random import gamma
from numba import njit, prange


from numba import njit
import numpy as np
from numba import njit
import numpy as np

@njit
def run_simulations_numpy_numba(tri_base, residuals, mask, phi, a2a, nbr_samples=1000):
    n_rows, n_cols = tri_base.shape
    results = np.zeros(nbr_samples)
    sqrt_tri = np.sqrt(np.abs(tri_base))

    for i in range(nbr_samples):
        sampled_resid = np.empty_like(tri_base)
        for r in range(n_rows):
            for c in range(n_cols):
                if mask[r, c]:
                    idx = np.random.randint(0, residuals.shape[0])
                    sampled_resid[r, c] = residuals[idx]
                else:
                    sampled_resid[r, c] = 0.0

        tri_sim = tri_base + sampled_resid * sqrt_tri

        # cumsum axis=1
        ctri_sim = np.empty_like(tri_sim)
        for r in range(n_rows):
            acc = 0.0
            for c in range(n_cols):
                acc += tri_sim[r, c]
                ctri_sim[r, c] = acc

        # square triangle
        for r in range(1, n_rows):
            for c in range(n_cols - r, n_cols):
                ctri_sim[r, c] = ctri_sim[r, c - 1] * a2a[c - 1]

        # to_incr_np (manual diff axis=1 with first column)
        tri_sqrd = np.empty_like(ctri_sim)
        for r in range(n_rows):
            tri_sqrd[r, 0] = ctri_sim[r, 0]
            for c in range(1, n_cols):
                tri_sqrd[r, c] = ctri_sim[r, c] - ctri_sim[r, c - 1]

        # gamma sampling
        for r in range(1, n_rows):
            for c in range(n_cols):
                if c < n_cols - r:
                    continue
                m = np.abs(tri_sqrd[r, c])
                if m == 0:
                    continue
                v = m * phi
                shape = m * m / v if v != 0 else 1.0
                scale = v / m if m != 0 else 1.0
                tri_sqrd[r, c] = np.random.gamma(shape, scale)

        # cumsum axis=1 again
        ctri_sqrd2 = np.empty_like(tri_sqrd)
        for r in range(n_rows):
            acc = 0.0
            for c in range(n_cols):
                acc += tri_sqrd[r, c]
                ctri_sqrd2[r, c] = acc

        results[i] = np.sum(ctri_sqrd2[:, -1])

    return results


@njit
def stochastic_triangle_forward_numba_batched(data_paid, sigma_j, dev, sd,
                                              sim_total=1000, batch_sim=1000,
                                              ultimate_param_resrisk=0,
                                              main_seed=202260011):
    mm, n_cols_orig = data_paid.shape
    n_dev = len(dev)
    results = np.zeros(sim_total)
    num_batches = sim_total // batch_sim

    for batch in range(num_batches):
        seed = main_seed + batch
        mu_part = np.empty((batch_sim, n_dev))
        sigma_part = np.empty((batch_sim, n_dev))

        for j in range(n_dev):
            np.random.seed(seed + j)
            mu_part[:, j] = np.random.normal(loc=dev[j], scale=sd[j], size=batch_sim)
            df = max(1, mm - j - 1)
            chi_list = np.random.chisquare(df, size=batch_sim)
            for s in range(batch_sim):
                sigma_part[s, j] = (np.floor(chi_list[s]) * sigma_j[j]) / df

        for i in range(batch_sim):
            np.random.seed(seed + i)
            m_i = mu_part[i, :]
            sigma_i = sigma_part[i, :]
            data_paid_copy = np.copy(data_paid).astype(np.float64)
            n_cols_current = data_paid_copy.shape[1]

            if n_cols_current < n_dev + 1:
                extra_cols = (n_dev + 1) - n_cols_current
                extended = np.empty((mm, n_cols_current + extra_cols), dtype=np.float64)
                extended[:, :n_cols_current] = data_paid_copy
                extended[:, n_cols_current:] = 0.0
                data_paid_copy = extended
                n_cols_current = data_paid_copy.shape[1]

            for j in range(n_dev):
                max_ind_row = max(1, mm - j)
                for r in range(max_ind_row - 1, mm):
                    if data_paid_copy[r, j] == 0:
                        continue
                    var_ij = sigma_i[j] / data_paid_copy[r, j]
                    m_sq = m_i[j] * m_i[j]
                    denom = np.sqrt(m_sq + var_ij)
                    if denom == 0.0 or m_sq == 0.0:
                        continue
                    lmean = np.log(m_sq / denom)
                    lstdev = np.sqrt(np.log(1 + (var_ij / m_sq)))
                    cl_ij = np.random.lognormal(lmean, lstdev)
                    data_paid_copy[r, j + 1] = data_paid_copy[r, j] * cl_ij

            u_i = data_paid_copy[:, n_cols_current - 1]
            results[batch * batch_sim + i] = np.sum(u_i) - ultimate_param_resrisk

    return results

import numpy as np
from numba import njit

@njit
def divide(x, y):
    result = np.zeros_like(x)
    for i in range(len(x)):
        result[i] = 0 if y[i] == 0 else x[i] / y[i]
    return result

@njit
def run_bootstrap_monte_carlo(triangle, weight, number_of_simulations=1000, is_sigma_reestimated=True, sigma_tail=1.65):
    triangle = np.nan_to_num(triangle)
    weight = np.nan_to_num(weight)
    a = triangle.shape[0]

    indiv_factors = np.zeros((a-1, a-1))
    for k in range(a-1):
        indiv_factors[:, k] = divide(triangle[:a-1, k+1], triangle[:a-1, k])

    indiv_factors_weighted = indiv_factors * weight[:a-1, :a-1]

    ldf = np.zeros(a-1)
    for k in range(a-1):
        num = 0.0
        den = 0.0
        for i in range(a-1):
            num += triangle[i, k] * indiv_factors_weighted[i, k]
            den += triangle[i, k] * weight[i, k]
        ldf[k] = 1.0 if den == 0 else num / den

    sigma = np.zeros(a-1)
    for k in range(a-1):
        num = 0.0
        den = 0.0
        for i in range(a-1):
            diff = indiv_factors_weighted[i, k] - ldf[k]
            w = weight[i, k]
            num += triangle[i, k] * w * diff * diff
            den += w
        sigma[k] = 0.0 if den <= 1 else np.sqrt(num / (den - 1))

    if a >= 4:
        sigma[a-2] = np.sqrt(min((sigma[a-3]**2 / sigma[a-4])**2, min(sigma[a-3]**2, sigma[a-4]**2)))

    for k in range(a-1):
        wsum = 0.0
        for i in range(a-1):
            wsum += weight[i, k]
        if wsum == 1.0:
            sigma[k] = sigma[max(0, k-1)]
        elif wsum == 0.0:
            sigma[k] = sigma_tail

    ln_mu_bootstrap = np.zeros((a, a-1))
    ln_sd_bootstrap = np.zeros((a, a-1))
    for k in range(a):
        for i in range(a-1):
            x = triangle[k, i]
            if x == 0:
                ln_mu_bootstrap[k, i] = -1e10  # zamiast -np.inf
                ln_sd_bootstrap[k, i] = 0.0
            else:
                mean_val = ldf[i] * x
                variance_ratio = (x * sigma[i]**2) / (mean_val**2)
                ln_mu_bootstrap[k, i] = np.log(mean_val) - 0.5 * np.log(variance_ratio + 1.0)
                ln_sd_bootstrap[k, i] = np.sqrt(np.log(variance_ratio + 1.0))

    ult = np.zeros(number_of_simulations)

    for n in range(number_of_simulations):
        triangle_boot = np.zeros((a, a))
        for i in range(a):
            triangle_boot[i, 0] = triangle[i, 0]

        for k in range(a):
            for i in range(1, a):
                if triangle_boot[k, i-1] != 0:
                    mu = ln_mu_bootstrap[k, i-1]
                    sigma_ln = ln_sd_bootstrap[k, i-1]
                    triangle_boot[k, i] = np.random.lognormal(mu, sigma_ln)

        indiv_f_boot = np.zeros((a-1, a-1))
        for k in range(a-1):
            indiv_f_boot[:, k] = divide(triangle_boot[:a-1, k+1], triangle[:a-1, k])
        indiv_f_boot_weighted = indiv_f_boot * weight[:a-1, :a-1]

        ldf_boot = np.zeros(a-1)
        for k in range(a-1):
            num = 0.0
            den = 0.0
            for i in range(a-1):
                num += triangle[i, k] * indiv_f_boot_weighted[i, k]
                den += triangle[i, k] * weight[i, k]
            ldf_boot[k] = 1.0 if den == 0 else num / den

        sigma_boot = sigma.copy()
        if is_sigma_reestimated:
            for k in range(a-1):
                num = 0.0
                den = 0.0
                for i in range(a-1):
                    diff = indiv_f_boot_weighted[i, k] - ldf_boot[k]
                    w = weight[i, k]
                    num += triangle[i, k] * w * diff * diff
                    den += w
                sigma_boot[k] = 0.0 if den <= 1 else np.sqrt(num / (den - 1))
                if k == a-2 and a >= 3:
                    sigma_boot[k] = np.sqrt(min((sigma_boot[k-1]**2 / sigma_boot[k-2])**2,
                                                min(sigma_boot[k-1]**2, sigma_boot[k-2]**2)))
                wsum = 0.0
                for i in range(a-1):
                    wsum += weight[i, k]
                if wsum == 1:
                    sigma_boot[k] = sigma_boot[max(0, k-1)]
                elif wsum == 0:
                    sigma_boot[k] = sigma_tail

        triangle_simu = np.zeros((a, a))
        for i in range(a):
            for j in range(a):
                triangle_simu[i, j] = triangle[i, j]

        for k in range(a):
            for i in range(1, a):
                if k + i >= a and triangle_simu[k, i-1] != 0:
                    prev = triangle_simu[k, i-1]
                    mu = np.log(ldf_boot[i-1] * prev) - 0.5 * np.log((prev * sigma_boot[i-1]**2) / (ldf_boot[i-1]**2 * prev**2) + 1)
                    sigma_ln = np.sqrt(np.log((prev * sigma_boot[i-1]**2) / (ldf_boot[i-1]**2 * prev**2) + 1))
                    triangle_simu[k, i] = np.random.lognormal(mu, sigma_ln)

        ult[n] = np.sum(triangle_simu[:, a-1])

    return ult

class TriangleCalculator:
    # === PANDAS SECTION ===

    @staticmethod
    def to_cum(tri: pd.DataFrame) -> pd.DataFrame:
        return tri.cumsum(axis=1)

    @staticmethod
    def to_incr(tri: pd.DataFrame) -> pd.DataFrame:
        tri_incr = tri.diff(axis=1)
        tri_incr.iloc[:, 0] = tri.iloc[:, 0]
        return tri_incr

    @staticmethod
    def get_a2a_factors(tri: pd.DataFrame) -> pd.Series:
        all_devps = tri.columns.tolist()
        min_origin, max_origin = tri.index.min(), tri.index.max()
        dps0, dps1 = all_devps[:-1], all_devps[1:]
        a2a_headers = [f"{ii}-{jj}" for ii, jj in zip(dps0, dps1)]
        a2a = []

        for dp1, dp0 in zip(dps1[::-1], dps0[::-1]):
            vals1 = tri.loc[min_origin:(max_origin - dp0), dp1].sum()
            vals0 = tri.loc[min_origin:(max_origin - dp0), dp0].sum()
            a2a.append((vals1 / vals0).item())

        return pd.Series(data=a2a[::-1], index=a2a_headers)

    @staticmethod
    def get_a2u_factors(a2a: pd.Series) -> pd.Series:
        return pd.Series(np.cumprod(a2a[::-1])[::-1], index=a2a.index, name="A2U")

    @staticmethod
    def fit_triangle_from_latest(ctri0: pd.DataFrame, a2a: pd.Series) -> pd.DataFrame:
        nbr_devps = ctri0.shape[1]
        ctri = pd.DataFrame(columns=ctri0.columns, index=ctri0.index, dtype=float)

        for idx, origin in enumerate(ctri0.index):
            latest_devp = nbr_devps - idx
            if latest_devp <= 0 or latest_devp > nbr_devps:
                continue
            ctri.at[origin, latest_devp] = ctri0.at[origin, latest_devp]
            for devp in range(latest_devp - 1, 0, -1):
                ctri.at[origin, devp] = float(ctri.at[origin, devp + 1]) / float(a2a.iloc[devp - 1])

        return ctri

    @staticmethod
    def full_ci_calculation(df: pd.DataFrame) -> tuple[pd.DataFrame, float]:
        a2a = TriangleCalculator.get_a2a_factors(df)
        ctri = TriangleCalculator.fit_triangle_from_latest(df, a2a)
        tri0 = TriangleCalculator.to_incr(df)
        tri = TriangleCalculator.to_incr(ctri)

        r_us = (tri0 - tri) / tri.abs().map(np.sqrt)
        n = tri0.count().sum().item()
        p = tri0.index.shape[0] + tri0.columns.shape[0] - 1
        DF = n - p
        phi = ((r_us ** 2).sum().sum() / DF).item()
        r_adj = np.sqrt(n / DF) * r_us

        return r_adj, phi

    @staticmethod
    def square_tri(tri: pd.DataFrame, a2a: pd.Series) -> pd.DataFrame:
        nbr_devps = tri.shape[1]
        for r_idx in range(1, nbr_devps):
            for c_idx in range(nbr_devps - r_idx, nbr_devps):
                tri.iat[r_idx, c_idx] = tri.iat[r_idx, c_idx - 1] * a2a.iat[c_idx - 1]
        return tri

    @staticmethod
    def sample_with_process_variance(ctri_ii_sqrd: pd.DataFrame, phi: float, rng=None) -> pd.DataFrame:
        if rng is None:
            rng = np.random.default_rng(seed=516)
        nbr_devps = ctri_ii_sqrd.shape[0]
        tri_ii_sqrd = ctri_ii_sqrd.copy()
        for r_idx in range(1, nbr_devps):
            for c_idx in range(nbr_devps - r_idx, nbr_devps):
                m = np.abs(tri_ii_sqrd.iat[r_idx, c_idx])
                v = m * phi
                shape = m ** 2 / v if v != 0 else 1.0
                scale = v / m if m != 0 else 1.0
                tri_ii_sqrd.iat[r_idx, c_idx] = rng.gamma(shape=shape, scale=scale)
        return tri_ii_sqrd

    # === NUMPY SECTION ===

    @staticmethod
    def to_cum_np(tri: np.ndarray) -> np.ndarray:
        return np.cumsum(tri, axis=1)

    @staticmethod
    def to_incr_np(tri: np.ndarray) -> np.ndarray:
        result = np.diff(tri, axis=1, prepend=0)
        result[:, 0] = tri[:, 0]
        return result

    @staticmethod
    def get_a2a_factors_np(tri: np.ndarray, mask: np.ndarray) -> np.ndarray:
        n_rows, n_cols = tri.shape
        a2a = []
        for j in range(n_cols - 1):
            vals0 = []
            vals1 = []
            for i in range(n_rows):
                if i + j + 1 < n_cols and mask[i, j] and mask[i, j + 1]:
                    vals0.append(tri[i, j])
                    vals1.append(tri[i, j + 1])
            num = np.nansum(vals1)
            denom = np.nansum(vals0)
            a2a.append(num / denom if denom != 0 else 1.0)
        return np.array(a2a)

    @staticmethod
    def square_tri_np(tri: np.ndarray, a2a: np.ndarray) -> np.ndarray:
        tri = tri.copy()
        n_rows, n_cols = tri.shape
        for i in range(1, n_rows):
            for j in range(n_cols - i, n_cols):
                tri[i, j] = tri[i, j - 1] * a2a[j - 1]
        return tri

    @staticmethod
    def to_mask_np(tri: np.ndarray) -> np.ndarray:
        return ~np.isnan(tri)

    @staticmethod
    def full_ci_calculation_np(df: np.ndarray, mask: np.ndarray) -> tuple[np.ndarray, float]:
        ctri = TriangleCalculator.fit_triangle_from_latest_np(df, mask)
        tri0 = TriangleCalculator.to_incr_np(df)
        tri = TriangleCalculator.to_incr_np(ctri)
        with np.errstate(divide='ignore', invalid='ignore'):
            r_us = (tri0 - tri) / np.sqrt(np.abs(tri))
        n = np.count_nonzero(mask)
        p = df.shape[0] + df.shape[1] - 1
        DF = n - p
        phi = np.nansum(r_us ** 2) / DF
        r_adj = np.sqrt(n / DF) * r_us
       # print(pd.DataFrame(r_adj))
        return r_adj, phi

    @staticmethod
    def fit_triangle_from_latest_np(ctri0: np.ndarray, mask: np.ndarray) -> np.ndarray:
        n_rows, n_cols = ctri0.shape
        ctri = np.full((n_rows, n_cols), np.nan)
        a2a = TriangleCalculator.get_a2a_factors_np(ctri0, mask)
        for i in range(n_rows):
            latest = n_cols - i - 1
            if 0 <= latest < n_cols:
                ctri[i, latest] = ctri0[i, latest]
                for j in range(latest - 1, -1, -1):
                    ctri[i, j] = ctri[i, j + 1] / a2a[j]
        return ctri

    @staticmethod
    def run_simulations_numpy(tri_base: np.ndarray, residuals: np.ndarray, mask: np.ndarray, phi: float, a2a: np.ndarray, nbr_samples=1000, seed=516) -> np.ndarray:
       
        rng = np.random.default_rng(seed)
        results = np.zeros(nbr_samples)
        sqrt_tri = np.sqrt(np.abs(tri_base))
        for i in range(nbr_samples):
            sampled_resid = rng.choice(residuals, size=mask.shape, replace=True)
            sampled_resid = np.where(mask, sampled_resid, np.nan)
            tri_sim = tri_base + sampled_resid * sqrt_tri
            ctri_sim = TriangleCalculator.to_cum_np(tri_sim)
            a2a_sim = TriangleCalculator.get_a2a_factors_np(ctri_sim, TriangleCalculator.to_mask_np(ctri_sim))
            ctri_sqrd = TriangleCalculator.square_tri_np(ctri_sim, a2a_sim)
            tri_sqrd = TriangleCalculator.to_incr_np(ctri_sqrd)
            for r in range(1, tri_sqrd.shape[0]):
                for c in range(tri_sqrd.shape[1]):
                    if c < tri_sqrd.shape[1] - r:
                        continue
                    m = np.abs(tri_sqrd[r, c])
                    if m == 0 or np.isnan(m):
                        continue
                    v = m * phi
                    shape = m ** 2 / v if v != 0 else 1.0
                    scale = v / m if m != 0 else 1.0
                    tri_sqrd[r, c] = rng.gamma(shape=shape, scale=scale)
            ctri_sqrd2 = TriangleCalculator.to_cum_np(tri_sqrd)
            results[i] = np.nansum(ctri_sqrd2[:, -1])
        return results

    def get_latest(tri: pd.DataFrame) -> pd.Series:

        nbr_devps = tri.shape[1]
        latest = [tri.iat[ii, nbr_devps - ii - 1].item() for ii in range(nbr_devps)]
        return pd.Series(data=latest, index=tri.index, name="latest")

    @staticmethod
    def run_simulations_numpy_nb(tri_base, residuals, mask, phi, a2a, nbr_samples=1000, seed=516):
        np.random.seed(seed)  # ustal seed ręcznie
        return run_simulations_numpy_numba(tri_base, residuals, mask, phi, a2a, nbr_samples)

    @staticmethod
    def run_batched_simulation(data_paid, sigma_j, dev, sd,
                               sim_total=1000, batch_sim=1000,
                               ultimate_param_resrisk=0,
                               main_seed=202260011):
        return stochastic_triangle_forward_numba_batched(
            data_paid, sigma_j, dev, sd,
            sim_total, batch_sim,
            ultimate_param_resrisk,
            main_seed
        )
    
    @staticmethod
    def run_bootstrap_monte_carlo_init(triangle, weight, number_of_simulations=1000, is_sigma_reestimated=True, sigma_tail=1.65):
        return run_bootstrap_monte_carlo(triangle, weight, number_of_simulations, is_sigma_reestimated, sigma_tail)

    
    @staticmethod
    def elementwise_division(data_dict):
        # Konwersja do macierzy
        columns = sorted(data_dict.keys())
        data_matrix = np.array([
            [val for val in data_dict[col]]
            for col in columns
        ]).T  # shape: (n_rows, n_cols)

        n_rows, n_cols = data_matrix.shape
        result = [[None for _ in range(n_cols - 1)] for _ in range(n_rows)]

        for i in range(n_rows):
            for j in range(1, n_cols):
                prev = data_matrix[i][j - 1]
                curr = data_matrix[i][j]
                if prev is None or curr is None:
                    result[i][j - 1] = None
                elif prev == 0:
                    result[i][j - 1] = 1
                else:
                    result[i][j - 1] = curr / prev

        return result  # shape: (n_rows, n_cols - 1)
   
    @staticmethod
    def Dev_prem(data_paid, w):
        nn = data_paid.shape[1]
        Dev_j = []
        for j in range(nn - 1):
            mianownik = np.sum([data_paid.iloc[i, j] * w.iloc[i, j] for i in range(nn-j-1)])
            licznik = np.sum([data_paid.iloc[i, j+1] * w.iloc[i, j] for i in range(nn-j-1)])
            if(mianownik==0):
                Dev_j.append(1)
            else:
                Dev_j.append(licznik / mianownik)
        return (Dev_j)
    
    @staticmethod
    def calculate_sigma(p_ij: pd.DataFrame, l_ij: pd.DataFrame, w_ij: pd.DataFrame, dev_j: list[float]) -> list[float]:
        max_col = l_ij.shape[1]
        sigmas = []
        sd = []

        for j in range(0, max_col):  # zaczynamy od j=1, bo potrzebujemy j-1
            numerator = 0.0
            denominator = 0.0
            denominator_sd = 0.0

            for i in range(len(l_ij)):
                try:
                    w = w_ij.iloc[i, j]
                    p = p_ij.iloc[i, j]
                    l = l_ij.iloc[i, j]
                    dev = dev_j[j]

                    if not np.isnan(w) and not np.isnan(p) and not np.isnan(l):
                        numerator += w * p * (l - dev) ** 2
                        denominator += w
                        denominator_sd+=w * p 
                except IndexError:
                    continue
            if denominator > 1:
                sigma = np.sqrt(numerator / (denominator-1))
                sd.append(sigma/np.sqrt(denominator_sd))
            elif j ==(max_col-1) and sigmas[j - 2]!=0 and denominator_sd!=0:
                sigma = min(sigmas[j - 1]**4/sigmas[j - 2]**2, min(sigmas[j - 2]**2, sigmas[j - 1]**2) )
                sd.append(sigma/np.sqrt(denominator_sd))
            else:
                sigma = 0
                sd.append(0)
            sigmas.append(sigma)

        return [sigmas,sd]
