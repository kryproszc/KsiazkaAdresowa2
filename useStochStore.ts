import { create } from 'zustand';

type HistogramItem = {
  bin: string;
  count: number;
};

type QuantileEntry = {
  value?: number;
  value_minus_latest?: number;
};

type StatEntry = {
  value: number;
  value_minus_latest?: number | null;
};

type StochStore = {
  requestId: string | null;
  setRequestId: (id: string | null) => void;

  histogramData: HistogramItem[];
  setHistogramData: (data: HistogramItem[]) => void;

  stats: Record<string, StatEntry> | null;
  setStats: (data: Record<string, StatEntry> | null) => void;

  quantileResult: Record<string, QuantileEntry> | null;
  setQuantileResult: (data: Record<string, QuantileEntry> | null) => void;

  percentileMatch: { sim: number | null; diff: number | null } | null;
  setPercentileMatch: (data: { sim: number | null; diff: number | null }) => void;

  percentileInputSim: string;
  percentileInputDiff: string;
  setPercentileInputSim: (val: string) => void;
  setPercentileInputDiff: (val: string) => void;

  quantileInput: string;
  setQuantileInput: (val: string) => void;

  rawQuantileInput: string;
  setRawQuantileInput: (val: string) => void;

  simulationCount: string;
  setSimulationCount: (val: string) => void;

  dev: number[];
  sd: number[];
  sigma: number[];
  setDev: (val: number[]) => void;
  setSd: (val: number[]) => void;
  setSigma: (val: number[]) => void;

  sim_total: number;
  setSimTotal: (val: number) => void;

  batch_sim: number;
  setBatchSim: (val: number) => void;

  main_seed: number;
  setMainSeed: (val: number) => void;
};

export const useStochStore = create<StochStore>((set) => ({
  requestId: null,
  setRequestId: (id) => set({ requestId: id }),

  histogramData: [],
  setHistogramData: (data) => set({ histogramData: data }),

  stats: null,
  setStats: (data) => set({ stats: data }),

  quantileResult: null,
  setQuantileResult: (data) => set({ quantileResult: data }),

  percentileMatch: null,
  setPercentileMatch: (data) => set({ percentileMatch: data }),

  percentileInputSim: '',
  percentileInputDiff: '',
  setPercentileInputSim: (val) => set({ percentileInputSim: val }),
  setPercentileInputDiff: (val) => set({ percentileInputDiff: val }),

  quantileInput: '0.01,0.05,0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,0.95,0.99',
  setQuantileInput: (val) => set({ quantileInput: val }),

  rawQuantileInput: '0.01,0.05,0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,0.95,0.99',
  setRawQuantileInput: (val) => set({ rawQuantileInput: val }),

  simulationCount: '1000',
  setSimulationCount: (val) => set({ simulationCount: val }),

  dev: [],
  sd: [],
  sigma: [],
  setDev: (val) => set({ dev: val }),
  setSd: (val) => set({ sd: val }),
  setSigma: (val) => set({ sigma: val }),

  // ✅ parametry symulacji przekazywane do backendu
  sim_total: 1000,
  setSimTotal: (val) => set({ sim_total: val }),

  batch_sim: 1000,
  setBatchSim: (val) => set({ batch_sim: val }),

  main_seed: 202260011,
  setMainSeed: (val) => set({ main_seed: val }),
}));
