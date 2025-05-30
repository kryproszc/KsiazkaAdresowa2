import { create } from 'zustand';
import type { RowData } from '@/types/table';
import { useTableStore } from './tableStore';

type MultStochStore = {
  selectedSheetJSON?: RowData[];
  selectedSheetName?: string;
  selectedCells?: RowData[];
  setSelectedCells: (cells: RowData[]) => void;
  resetSelection: () => void;
  hydrateFromTableStore: () => void;
};

export const useMultStochStore = create<MultStochStore>((set, get) => ({
  selectedSheetJSON: undefined,
  selectedSheetName: undefined,
  selectedCells: undefined,

  setSelectedCells: (cells) => set({ selectedCells: cells }),

  resetSelection: () => {
    const sheet = get().selectedSheetJSON;
    if (!sheet) return;
    const selected = sheet.map((row) => row.map(() => 1));
    set({ selectedCells: selected });
  },

  hydrateFromTableStore: () => {
    const mainStore = useTableStore.getState();
    const sheet = mainStore.selectedSheetJSON;
    const name = mainStore.selectedSheetName;

    if (!sheet) return;

    const selected = sheet.map((row) => row.map(() => 1));

    set({
      selectedSheetJSON: sheet,
      selectedSheetName: name,
      selectedCells: selected,
    });
  },
}));
