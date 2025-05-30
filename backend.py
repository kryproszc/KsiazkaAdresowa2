from fastapi import FastAPI, File, UploadFile, WebSocket, HTTPException
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel
from typing import List, Union, Optional
import uuid
import pandas as pd
import numpy as np
import asyncio
from io import BytesIO
from openpyxl import load_workbook
from calculation_method import TriangleCalculator
import time
from typing import Dict


print("backin.py")
app = FastAPI()


app.add_middleware(
    CORSMiddleware,
    allow_origins=["http://localhost:3000"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

CACHE = {}

class MatrixRequest(BaseModel):
    user_id: str
    paid_data: List[List[Union[str, int, float, None]]]
    paid_weights: List[List[int]]
    cl_data: List[List[Union[str, int, float, None]]]
    cl_weights: List[List[int]]
    triangle_raw: List[List[Union[str, int, float, None]]]
    cl_weights_raw: List[List[int]]
    wagi_mult: Optional[List[List[int]]] = None
    quantiles: Optional[List[float]] = None
    nbr_samples: Optional[int] = 1000

class QuantileRequest(BaseModel):
    user_id: str
    request_id: str
    quantiles: List[float]



# ----------------------------
# Session Management Classes
# ----------------------------

class UserSession:
    def __init__(self, user_id: str):
        self.user_id = user_id
        self.simulations: Dict[str, Dict] = {}

    def save_simulation(self, request_id: str, sim, diff, latest, quantiles_result=None):
        self.simulations[request_id] = {
            "sim": sim,
            "diff": diff,
            "latest": latest,
            "quantiles": quantiles_result
        }

    def get_simulation(self, request_id: str):
        return self.simulations.get(request_id)


class SessionManager:
    def __init__(self):
        self.sessions: Dict[str, UserSession] = {}

    def get_session(self, user_id: str) -> UserSession:
        if user_id not in self.sessions:
            self.sessions[user_id] = UserSession(user_id)
        return self.sessions[user_id]


SESSIONS = SessionManager()





@app.get("/")
async def root():
    return {"message": "Hello World"}

@app.post("/upload")
async def create_upload_files(file: UploadFile):
    print(file.filename)
    content = await file.read()
    wb = load_workbook(filename=BytesIO(content))
    ws = wb.active
    if ws:
        for row in ws.iter_rows(values_only=True):
            for cell in row:
                if cell:
                    print(cell, end=" ")
            print("")
    return {"filename": file.filename, "size": len(content)}

@app.post("/calc/cl")
async def calc_cl(
    data: List[List[Union[str, int, float, None]]],
    selected: List[List[int]],
):
    print("selected")
    print(selected)
    df = pd.DataFrame(data).iloc[1:, 1:]
    df = df.apply(pd.to_numeric, errors="coerce")
    tri = TriangleCalculator.full_ci_calculation(df)[0]
    safe_tri = tri.replace([np.inf, -np.inf], np.nan).astype(object).where(pd.notnull(tri), None)

    matrix = [[tri.columns.name or "AY"] + list(tri.columns)]
    for idx in tri.index:
        values = list(safe_tri.loc[idx])
        while values and values[-1] is None:
            values.pop()
        row = [idx] + values
        matrix.append(row)
    return {"data": matrix}

# ---------------------
# CORE: FULL + QUANTILES
# ---------------------

@app.post("/calc/full")
async def calc_full(payload: MatrixRequest):
    print("df_triangle_raw")

    start1 = time.time()
    start2 = time.time()

    user_session = SESSIONS.get_session(payload.user_id)

    df_triangle_raw = (
        pd.DataFrame(payload.triangle_raw)
        .iloc[1:, 1:]
        .apply(pd.to_numeric, errors="coerce")
    )
    df_cl_weights_raw = pd.DataFrame(payload.cl_weights_raw).astype(float)
    cl_weights = pd.DataFrame(payload.cl_weights).astype(float)

    df_np = np.array(df_triangle_raw.values, dtype=float)
    df_wagi_np = np.array(df_cl_weights_raw.values, dtype=float)

    combined_mask = (~np.isnan(df_np))
    mask_np = ~np.isnan(df_np)

    tri0_np = TriangleCalculator.to_incr_np(df_np)
    ctri0_np = TriangleCalculator.to_cum_np(tri0_np)
    a2a_np = TriangleCalculator.get_a2a_factors_np(ctri0_np, mask_np)
    ctri_np = TriangleCalculator.fit_triangle_from_latest_np(df_np, mask_np)
    tri_np = TriangleCalculator.to_incr_np(ctri_np)
    r_adj_np, phi_np = TriangleCalculator.full_ci_calculation_np(df_np, mask_np)

    residuals = r_adj_np[(combined_mask) & (r_adj_np != 0) & ~np.isnan(r_adj_np)].flatten()
    end1 = time.time()
    print(f"Czas wykonania: {end1 - start1:.2f} sekund")

    sim_results_np = TriangleCalculator.run_simulations_numpy_nb(
        tri_np, residuals, mask_np, phi_np, a2a_np, nbr_samples=payload.nbr_samples
    )

    latest = np.sum(TriangleCalculator.get_latest(df_triangle_raw))
    sim_diff_np = sim_results_np - latest

    print(f"Czas wykonania symulacje i ibnr: {time.time() - start2:.2f} sekund")

    request_id = str(uuid.uuid4())

    # ZAPIS DO SESJI ZAMIAST CACHE
    user_session.save_simulation(
        request_id=request_id,
        sim=sim_results_np,
        diff=sim_diff_np,
        latest=latest
    )

    counts, bins = np.histogram(sim_results_np, bins=50)

    print(f"Czas wykonania calosc: {time.time() - start2:.2f} sekund")

    return {
        "message": "OK",
        "request_id": request_id,
        "triangle_shape": df_triangle_raw.shape,
        "histogram": {
            "bins": bins.tolist(),
            "counts": counts.tolist()
        }
    }


@app.post("/calc/quantiles")
async def calc_quantiles(payload: QuantileRequest):
    user_session = SESSIONS.get_session(payload.user_id)
    cached = user_session.get_simulation(payload.request_id)

    if cached is None:
        raise HTTPException(status_code=403, detail="Nie znaleziono danych symulacji")

    sim_results_np = cached["sim"]
    sim_diff_np = cached["diff"]

    quantiles = payload.quantiles
    values_sim = np.quantile(sim_results_np, quantiles)
    values_diff = np.quantile(sim_diff_np, quantiles)

    quantile_result = {
        str(round(q, 4)): {
            "value": float(round(v1, 2)) if np.isfinite(v1) else None,
            "value_minus_latest": float(round(v2, 2)) if np.isfinite(v2) else None
        }
        for q, v1, v2 in zip(quantiles, values_sim, values_diff)
    }

    stats = {
        "mean": {
            "value": float(np.mean(sim_results_np)),
            "value_minus_latest": float(np.mean(sim_diff_np)),
        },
        "variance": {
            "value": float(np.var(sim_results_np)),
            "value_minus_latest": float(np.var(sim_diff_np)),
        },
        "std_dev": {
            "value": float(np.std(sim_results_np)),
            "value_minus_latest": float(np.std(sim_diff_np)),
        },
        "min": {
            "value": float(np.min(sim_results_np)),
            "value_minus_latest": float(np.min(sim_diff_np)),
        },
        "max": {
            "value": float(np.max(sim_results_np)),
            "value_minus_latest": float(np.max(sim_diff_np)),
        },
    }

    return {
        "quantiles": quantile_result,
        "stats": stats
    }

@app.websocket("/ws/progress")
async def websocket_endpoint(websocket: WebSocket):
    await websocket.accept()
    for i in range(1, 11):
        await websocket.send_json({"progress": i})
        if i < 10:
            await asyncio.sleep(1)
    await websocket.close()

class PercentileRequest(BaseModel):
    user_id: str
    request_id: str
    value: float
    source: str  # 'sim' lub 'diff'

@app.post("/calc/percentile")
async def calc_percentile(payload: PercentileRequest):
    user_session = SESSIONS.get_session(payload.user_id)
    cached = user_session.get_simulation(payload.request_id)

    if cached is None:
        raise HTTPException(status_code=404, detail="Nie znaleziono danych symulacji")

    value = payload.value
    source = payload.source

    def compute_percentile(data, val):
        sorted_data = np.sort(data)
        percentile = np.searchsorted(sorted_data, val, side='right') / len(sorted_data)
        return float(round(percentile, 6))

    if source == 'sim':
        percentile = compute_percentile(cached["sim"], value)
    elif source == 'diff':
        percentile = compute_percentile(cached["diff"], value)
    else:
        raise HTTPException(status_code=400, detail="Nieprawidłowe źródło: musi być 'sim' lub 'diff'")

    return {
        "source": source,
        "percentile": percentile
    }

### zaklada multiplikatywna stochastyczna

@app.post("/calc/mult_stoch")
async def calc_mult_stoch(payload: MatrixRequest):
    data = {}
    headers = payload.paid_data[0][1:]

    for i, col in enumerate(headers):
        data[col] = []
        for row in payload.paid_data[1:]:  # pomijamy nagłówek
            value = row[i + 1]  # i + 1, bo indeks 0 to rok
            data[col].append(int(value) if str(value).isdigit() else None)

    
    train_devide = TriangleCalculator.elementwise_division(data)
    train_devide_serializable = np.array(train_devide).tolist()  
    return {
        "train_devide": train_devide_serializable
    }


class WspolczynnikiMultRequest(BaseModel):
    user_id: str
    wagi_mult: List[List[int]]
    paid_data: List[List[Union[str, int, float, None]]]


import math

def try_parse_float(value):
    try:
        f = float(value)
        if not math.isfinite(f):
            return None
        return f
    except (ValueError, TypeError):
        return None

@app.post("/calc/wspolczynniki_mult")
async def calc_wspolczynniki_mult(payload: WspolczynnikiMultRequest):
    data = {}
    headers = payload.paid_data[0][1:]
    print(payload.paid_data)
    for i, col in enumerate(headers):
        data[col] = []
        for row in payload.paid_data[1:]:  # pomijamy nagłówek
            value = row[i + 1]  # i + 1, bo indeks 0 to rok
            data[col].append(int(value) if str(value).isdigit() else None)
    print(data)
    print(pd.DataFrame(data))
    l_ij = pd.DataFrame(TriangleCalculator.elementwise_division(data))
    print(l_ij)
    w_ij = pd.DataFrame(payload.wagi_mult)
    print(w_ij)
    p_ij = pd.DataFrame(data)
    print(p_ij)
    dev_j = TriangleCalculator.Dev_prem(p_ij,w_ij)
    print(dev_j)

    sigmas = TriangleCalculator.calculate_sigma(p_ij, l_ij, w_ij, dev_j)

    return {
        "dev": dev_j,
        "sd": sigmas[0],     
        "sigma": sigmas[1]   
    }


class WspolczynnikiMultiplikatywnaStochastycznaRequest(BaseModel):
    user_id: str
    dev: List[float]
    sd: List[float]
    sigma: List[float]
    triangle: List[List[Union[str, int, float, None]]]
    sim_total: Optional[int] = 1000
    batch_sim: Optional[int] = 1000
    main_seed: Optional[int] = 202260011
    ultimate_param_resrisk: Optional[int] = 0

@app.post("/calc/obliczenia_stoch_multiplikatywna")
async def obliczenia_stoch_multiplikatywna(payload: WspolczynnikiMultiplikatywnaStochastycznaRequest):
    data = {}
    headers = payload.triangle[0][1:]
    for i, col in enumerate(headers):
        data[col] = []
        for row in payload.triangle[1:]:
            value = row[i + 1]
            data[col].append(int(value) if str(value).isdigit() else None)
    df_np = np.array(list(data.values()), dtype=float).T
    print("payload.sd")
    print(payload.sd)
    print("payload.dev")
    print( payload.dev)
    print("payload.sigma")
    print( payload.sigma)
    results_cale = TriangleCalculator.run_batched_simulation(df_np, payload.sd, payload.dev, payload.sigma,
                                                             sim_total=payload.sim_total,batch_sim=payload.batch_sim,
                                                             ultimate_param_resrisk=payload.ultimate_param_resrisk,
                                                             main_seed=payload.main_seed)
    latest = np.sum(TriangleCalculator.get_latest(pd.DataFrame(df_np)))
    sim_diff_np = results_cale - latest

    request_id = str(uuid.uuid4())
    user_session = SESSIONS.get_session(payload.user_id)
    user_session.save_simulation(request_id, results_cale, sim_diff_np, latest)
    counts, bins = np.histogram(results_cale, bins=50)

    return {
        "message": "OK",
        "request_id": request_id,
        "triangle_shape": df_np.shape,
        "histogram": {
            "bins": bins.tolist(),
            "counts": counts.tolist()
        }
    }

class QuantileStochRequest(BaseModel):
    user_id: str
    request_id: str
    quantiles: List[float]

@app.post("/calc/quantiles_stoch")
async def calc_quantiles_stoch(payload: QuantileStochRequest):
    user_session = SESSIONS.get_session(payload.user_id)
    cached = user_session.get_simulation(payload.request_id)

    if cached is None:
        raise HTTPException(status_code=403, detail="Nie znaleziono danych symulacji")

    sim_results_np = cached["sim"]
    sim_diff_np = cached["diff"]

    quantiles = payload.quantiles
    values_sim = np.quantile(sim_results_np, quantiles)
    values_diff = np.quantile(sim_diff_np, quantiles)

    quantile_result = {
        str(round(q, 4)): {
            "value": float(round(v1, 2)) if np.isfinite(v1) else None,
            "value_minus_latest": float(round(v2, 2)) if np.isfinite(v2) else None
        }
        for q, v1, v2 in zip(quantiles, values_sim, values_diff)
    }

    stats = {
        "mean": {
            "value": float(np.mean(sim_results_np)),
            "value_minus_latest": float(np.mean(sim_diff_np)),
        },
        "variance": {
            "value": float(np.var(sim_results_np)),
            "value_minus_latest": float(np.var(sim_diff_np)),
        },
        "std_dev": {
            "value": float(np.std(sim_results_np)),
            "value_minus_latest": float(np.std(sim_diff_np)),
        },
        "min": {
            "value": float(np.min(sim_results_np)),
            "value_minus_latest": float(np.min(sim_diff_np)),
        },
        "max": {
            "value": float(np.max(sim_results_np)),
            "value_minus_latest": float(np.max(sim_diff_np)),
        },
    }

    return {
        "quantiles": quantile_result,
        "stats": stats
    }


class PercentileStochRequest(BaseModel):
    user_id: str
    request_id: str
    value: float
    source: str  # 'sim' lub 'diff'

@app.post("/calc/percentile_stoch")
async def calc_percentile_stoch(payload: PercentileStochRequest):
    user_session = SESSIONS.get_session(payload.user_id)
    cached = user_session.get_simulation(payload.request_id)

    if cached is None:
        raise HTTPException(status_code=404, detail="Nie znaleziono danych symulacji")

    value = payload.value
    source = payload.source

    def compute_percentile(data, val):
        sorted_data = np.sort(data)
        percentile = np.searchsorted(sorted_data, val, side='right') / len(sorted_data)
        return float(round(percentile, 6))

    if source == 'sim':
        percentile = compute_percentile(cached["sim"], value)
    elif source == 'diff':
        percentile = compute_percentile(cached["diff"], value)
    else:
        raise HTTPException(status_code=400, detail="Nieprawidłowe źródło: musi być 'sim' lub 'diff'")

    return {
        "source": source,
        "percentile": percentile
    }


###################
@app.get("/debug/endpoints")
def debug_endpoints():
    return [route.path for route in app.routes]
### boot

class WspolczynnikiBootRequest(BaseModel):
    user_id: str
    wagi_boot: List[List[int]]
    paid_data: List[List[Union[str, int, float, None]]]




@app.post("/calc/wspolczynniki_boot")
async def calc_wspolczynniki_boot(payload: WspolczynnikiBootRequest):
    data = {}
    headers = payload.paid_data[0][1:]
    for i, col in enumerate(headers):
        data[col] = []
        for row in payload.paid_data[1:]:  
            value = row[i + 1]  
            data[col].append(int(value) if str(value).isdigit() else None)
    l_ij = pd.DataFrame(TriangleCalculator.elementwise_division(data))
    w_ij = pd.DataFrame(payload.wagi_boot)
    p_ij = pd.DataFrame(data)
    dev_j = TriangleCalculator.Dev_prem(p_ij,w_ij)

    print(dev_j)
    print("----")

    sigmas = TriangleCalculator.calculate_sigma(p_ij, l_ij, w_ij, dev_j)
    print(sigmas[0])
    print("----")
    print(sigmas[1])
    print("----")
    return {
        "dev": dev_j,
        "sd": sigmas[0],
        "sigma": sigmas[1]
    }


class WspolczynnikiBootParamRequest(BaseModel):
    user_id: str
    dev: List[float]
    sd: List[float]
    sigma: List[float]
    triangle: List[List[Union[str, int, float, None]]]
    wagi_boot: List[List[int]]
    sim_total: Optional[int] = 1000
    batch_sim: Optional[int] = 1000
    main_seed: Optional[int] = 202260011
    ultimate_param_resrisk: Optional[int] = 0


@app.post("/calc/obliczenia_boot_multiplikatywna")
async def obliczenia_boot_multiplikatywna(payload: WspolczynnikiBootParamRequest):
    wagi = pd.DataFrame(payload.wagi_boot)
    data = {}
    headers = payload.triangle[0][1:]
    for i, col in enumerate(headers):
        data[col] = [
            int(row[i + 1]) if str(row[i + 1]).isdigit() else 0
            for row in payload.triangle[1:]
        ]
    triangle_np = np.array(list(data.values()), dtype=float).T
    weight_np = wagi.iloc[:(wagi.shape[0]-1), :(wagi.shape[0]-1)].to_numpy(dtype=np.float64)
    print(pd.DataFrame(weight_np))
    results = TriangleCalculator.run_bootstrap_monte_carlo_init(
        triangle_np,
        weight_np,
        number_of_simulations=payload.sim_total,
        is_sigma_reestimated=False
    )

    latest = np.sum(TriangleCalculator.get_latest(pd.DataFrame(triangle_np)))
    sim_diff_np = results - latest

    request_id = str(uuid.uuid4())
    user_session = SESSIONS.get_session(payload.user_id)
    user_session.save_simulation(request_id, results, sim_diff_np, latest)

    counts, bins = np.histogram(results, bins=50)

    return {
        "message": "OK",
        "request_id": request_id,
        "triangle_shape": triangle_np.shape,
        "histogram": {
            "bins": bins.tolist(),
            "counts": counts.tolist()
        }
    }


@app.post("/calc/quantiles_boot")
async def calc_quantiles_boot(payload: QuantileStochRequest):
    user_session = SESSIONS.get_session(payload.user_id)
    cached = user_session.get_simulation(payload.request_id)

    if cached is None:
        raise HTTPException(status_code=403, detail="Nie znaleziono danych boot")

    sim_results_np = cached["sim"]
    sim_diff_np = cached["diff"]

    quantiles = payload.quantiles
    values_sim = np.quantile(sim_results_np, quantiles)
    values_diff = np.quantile(sim_diff_np, quantiles)

    quantile_result = {
        str(round(q, 4)): {
            "value": float(round(v1, 2)) if np.isfinite(v1) else None,
            "value_minus_latest": float(round(v2, 2)) if np.isfinite(v2) else None
        }
        for q, v1, v2 in zip(quantiles, values_sim, values_diff)
    }

    stats = {
        "mean": {
            "value": float(np.mean(sim_results_np)),
            "value_minus_latest": float(np.mean(sim_diff_np)),
        },
        "variance": {
            "value": float(np.var(sim_results_np)),
            "value_minus_latest": float(np.var(sim_diff_np)),
        },
        "std_dev": {
            "value": float(np.std(sim_results_np)),
            "value_minus_latest": float(np.std(sim_diff_np)),
        },
        "min": {
            "value": float(np.min(sim_results_np)),
            "value_minus_latest": float(np.min(sim_diff_np)),
        },
        "max": {
            "value": float(np.max(sim_results_np)),
            "value_minus_latest": float(np.max(sim_diff_np)),
        },
    }

    return {
        "quantiles": quantile_result,
        "stats": stats
    }


@app.post("/calc/percentile_boot")
async def calc_percentile_boot(payload: PercentileStochRequest):
    user_session = SESSIONS.get_session(payload.user_id)
    cached = user_session.get_simulation(payload.request_id)

    if cached is None:
        raise HTTPException(status_code=404, detail="Nie znaleziono danych boot")

    value = payload.value
    source = payload.source

    def compute_percentile(data, val):
        sorted_data = np.sort(data)
        percentile = np.searchsorted(sorted_data, val, side='right') / len(sorted_data)
        return float(round(percentile, 6))

    if source == 'sim':
        percentile = compute_percentile(cached["sim"], value)
    elif source == 'diff':
        percentile = compute_percentile(cached["diff"], value)
    else:
        raise HTTPException(status_code=400, detail="Nieprawidłowe źródło: musi być 'sim' lub 'diff'")

    return {
        "source": source,
        "percentile": percentile
    }
