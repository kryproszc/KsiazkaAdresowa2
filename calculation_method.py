import pandas as pd
import numpy as np
from numpy.random import gamma


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
        print(pd.DataFrame(r_adj))
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
