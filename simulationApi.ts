export async function runFullSimulation(body: any) {
  const res = await fetch('http://localhost:8000/calc/full', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(body),
  });

  if (!res.ok) throw new Error('Simulation failed');
  return res.json();
}

export async function getQuantiles(userId: string, requestId: string, quantiles: number[]) {
  const res = await fetch('http://localhost:8000/calc/quantiles', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
    },
    body: JSON.stringify({
      user_id: userId,
      request_id: requestId,
      quantiles,
    }),
  });

  if (!res.ok) {
    throw new Error('Quantiles fetch failed');
  }

  return res.json();
}

export async function getPercentile(
  userId: string, // ✅ dodajemy userId jako argument
  requestId: string,
  value: number,
  source: 'sim' | 'diff'
) {
  const res = await fetch('http://localhost:8000/calc/percentile', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
    },
    body: JSON.stringify({
      user_id: userId, // ✅ dodajemy do body
      request_id: requestId,
      value,
      source,
    }),
  });

  if (!res.ok) {
    throw new Error('Percentile fetch failed');
  }

  return res.json();
}

export async function runStochSimulation(body: {
  user_id: string;
  dev: number[];
  sd: number[];
  sigma: number[];
  triangle: any[][];
  sim_total?: number;
  batch_sim?: number;
  main_seed?: number;
  ultimate_param_resrisk?: number;
}) {
  const res = await fetch('http://localhost:8000/calc/obliczenia_stoch_multiplikatywna', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
    },
    body: JSON.stringify(body),
  });

  if (!res.ok) {
    const errorText = await res.text();
    throw new Error(`Stochastic simulation failed: ${errorText}`);
  }

  return res.json();
}


// simulationApi.ts
export async function getQuantilesStoch(userId: string, requestId: string, quantiles: number[]) {
  const res = await fetch('http://localhost:8000/calc/quantiles_stoch', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
    },
    body: JSON.stringify({
      user_id: userId,         
      request_id: requestId,   
      quantiles,               
    }),
  });

  if (!res.ok) {
    throw new Error('Quantiles (stoch) fetch failed');
  }

  return res.json();
}


export async function getPercentileStoch(
  userId: string,
  requestId: string,
  value: number,
  source: 'sim' | 'diff'
) {
  const res = await fetch('http://localhost:8000/calc/percentile_stoch', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
    },
    body: JSON.stringify({
      user_id: userId,
      request_id: requestId,
      value,
      source,
    }),
  });

  if (!res.ok) {
    throw new Error('Percentile (stoch) fetch failed');
  }

  return res.json();
}

export const runBootSimulation = async (payload: any) => {
  const res = await fetch('http://localhost:8000/calc/obliczenia_boot_multiplikatywna', {
    method: 'POST',
    body: JSON.stringify(payload),
    headers: { 'Content-Type': 'application/json' },
  });
  if (!res.ok) {
    const error = await res.text();
    throw new Error(`Błąd przy wywołaniu symulacji bootstrap: ${error}`);
  }
  return res.json();
};

