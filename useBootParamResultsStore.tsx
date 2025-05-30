import { create } from 'zustand';

interface BootParamResultStore {
  dev: number[];
  sd: number[];
  sigma: number[];
  setDev: (dev: number[]) => void;
  setSd: (sd: number[]) => void;
  setSigma: (sigma: number[]) => void;
  reset: () => void; // ✅ dodane
}

export const useBootParamResultsStore = create<BootParamResultStore>((set) => ({
  dev: [],
  sd: [],
  sigma: [],
  setDev: (dev) => set({ dev }),
  setSd: (sd) => set({ sd }),
  setSigma: (sigma) => set({ sigma }),
  reset: () => set({ dev: [], sd: [], sigma: [] }), // ✅ implementacja resetu
}));
