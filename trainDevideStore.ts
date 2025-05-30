import { create } from 'zustand';
import { useTableStore } from './tableStore';

type TrainDevideStore = {
  trainDevide?: number[][];
  selectedWeights?: number[][];
  selectedSheetName?: string;

  setTrainDevide: (data: number[][]) => void;
  setSelectedWeights: (weights: number[][]) => void;
  resetSelection: () => void;
  hydrateFromTableStore: () => void;
  reset: () => void; // ✅ nowa metoda
};

export const useTrainDevideStore = create<TrainDevideStore>((set, get) => ({
  trainDevide: undefined,
  selectedWeights: undefined,
  selectedSheetName: undefined,

  setTrainDevide: (data) => set({ trainDevide: data }),
  setSelectedWeights: (weights) => set({ selectedWeights: weights }),

  resetSelection: () => {
    const matrix = get().trainDevide;
    if (!matrix) return;
    const selected = matrix.map((row) => row.map(() => 1));
    set({ selectedWeights: selected });
  },

  hydrateFromTableStore: () => {
    const table = useTableStore.getState();
    const sheet = table.selectedSheetJSON;
    const name = table.selectedSheetName;

    if (!sheet) return;

    const converted = sheet.map((row) =>
      row.slice(1).map((cell) => (typeof cell === 'number' ? cell : 1))
    );

    const selected = converted.map((row) => row.map(() => 1));

    set({
      trainDevide: converted,
      selectedSheetName: name,
      selectedWeights: selected,
    });
  },

  reset: () => set({
    trainDevide: undefined,
    selectedWeights: undefined,
    selectedSheetName: undefined,
  }),
}));
