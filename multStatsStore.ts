import { create } from 'zustand';

interface StatsStore {
  dev: number[] | null;
  sd: number[] | null;
  sigma: number[] | null;
  setStats: (dev: number[], sd: number[], sigma: number[]) => void;
  resetStats: () => void;
}

export const useStatsStore = create<StatsStore>((set) => ({
  dev: null,
  sd: null,
  sigma: null,
  setStats: (dev, sd, sigma) => set({ dev, sd, sigma }),
  resetStats: () => set({ dev: null, sd: null, sigma: null }),
}));
