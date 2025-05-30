import { create } from 'zustand';

type StochResultsStore = {
  dev: number[];
  sd: number[];
  sigma: number[];

  setDev: (dev: number[]) => void;
  setSd: (sd: number[]) => void;
  setSigma: (sigma: number[]) => void;

  reset: () => void; // ✅ nowa funkcja
};

export const useStochResultsStore = create<StochResultsStore>((set) => ({
  dev: [],
  sd: [],
  sigma: [],

  setDev: (dev) => set({ dev }),
  setSd: (sd) => set({ sd }),
  setSigma: (sigma) => set({ sigma }),

  reset: () => set({ dev: [], sd: [], sigma: [] }), // ✅ czyści wszystko
}));
