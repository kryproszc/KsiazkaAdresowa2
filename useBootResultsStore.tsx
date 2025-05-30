import { create } from 'zustand';

type BootResultsStore = {
  dev: number[];
  sd: number[];
  sigma: number[];
  setDev: (val: number[]) => void;
  setSd: (val: number[]) => void;
  setSigma: (val: number[]) => void;
  reset: () => void;
};

export const useBootResultsStore = create<BootResultsStore>((set) => ({
  dev: [],
  sd: [],
  sigma: [],
  setDev: (val) => set({ dev: val }),
  setSd: (val) => set({ sd: val }),
  setSigma: (val) => set({ sigma: val }),
  reset: () => set({ dev: [], sd: [], sigma: [] }), 
}));
