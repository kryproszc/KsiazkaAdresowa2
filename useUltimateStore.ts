import { create } from 'zustand';
import { persist, createJSONStorage } from 'zustand/middleware';

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

type UltimateStore = {
  userId: string | null;
  setUserId: (id: string | null) => void;

  histogramData: HistogramItem[];
  stats: Record<string, StatEntry> | null;
  quantileResult: Record<string, QuantileEntry> | null;
  percentileMatch: { sim: number | null; diff: number | null } | null;
  requestId: string | null;

  quantileInput: string;
  rawQuantileInput: string;
  percentileInputSim: string;
  percentileInputDiff: string;

  setHistogramData: (data: HistogramItem[]) => void;
  setStats: (data: Record<string, StatEntry> | null) => void;
  setQuantileResult: (data: Record<string, QuantileEntry> | null) => void;
  setPercentileMatch: (data: { sim: number | null; diff: number | null }) => void;
  setRequestId: (id: string | null) => void;

  setQuantileInput: (val: string) => void;
  setRawQuantileInput: (val: string) => void;
  setPercentileInputSim: (val: string) => void;
  setPercentileInputDiff: (val: string) => void;
  simulationCount: string;
  setSimulationCount: (val: string) => void;

  dev: number[];
  sd: number[];
  sigma: number[];
  setDev: (val: number[]) => void;
  setSd: (val: number[]) => void;
  setSigma: (val: number[]) => void;

  // ✅ reset funkcja
  reset: () => void;
};

export const useUltimateStore = create<UltimateStore>()(
  persist(
    (set) => ({
      userId: null,
      setUserId: (id) => set({ userId: id }),

      histogramData: [],
      stats: null,
      quantileResult: null,
      percentileMatch: null,
      requestId: null,

      quantileInput: '0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,0.95,0.99',
      rawQuantileInput: '0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,0.95,0.99',
      percentileInputSim: '',
      percentileInputDiff: '',

      simulationCount: '1000',

      setHistogramData: (data) => set({ histogramData: data }),
      setStats: (data) => set({ stats: data }),
      setQuantileResult: (data) => set({ quantileResult: data }),
      setPercentileMatch: (data) => set({ percentileMatch: data }),
      setRequestId: (id) => set({ requestId: id }),

      setQuantileInput: (val) => set({ quantileInput: val }),
      setRawQuantileInput: (val) => set({ rawQuantileInput: val }),
      setPercentileInputSim: (val) => set({ percentileInputSim: val }),
      setPercentileInputDiff: (val) => set({ percentileInputDiff: val }),
      setSimulationCount: (val) => set({ simulationCount: val }),

      dev: [],
      sd: [],
      sigma: [],
      setDev: (val) => set({ dev: val }),
      setSd: (val) => set({ sd: val }),
      setSigma: (val) => set({ sigma: val }),

      // ✅ Reset — czyści cały stan, poza userId
reset: () =>
  set((state) => ({
    histogramData: [],
    stats: null,
    quantileResult: null,
    percentileMatch: null,
    requestId: null,
    quantileInput: '0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,0.95,0.99',
    rawQuantileInput: '0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,0.95,0.99',
    percentileInputSim: '',
    percentileInputDiff: '',
    simulationCount: '1000',
    dev: [],
    sd: [],
    sigma: [],
    userId: state.userId, // ✨ pozostaw userId bez zmian
  })),

    }),
    {
      name: 'ultimate-storage',
      storage: createJSONStorage(() => sessionStorage),
    }
  )
);
