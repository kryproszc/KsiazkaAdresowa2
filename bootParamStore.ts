import { create } from 'zustand';
import type { RowData } from '@/types/table';
import { useTableStore } from './tableStore';

type BootParamStore = {
  selectedSheetJSON?: RowData[];
  selectedSheetName?: string;
  selectedCells?: number[][];

  setSelectedCells: (cells: number[][]) => void;
  resetSelection: () => void;
  hydrateFromTableStore: () => void;
  reset: () => void; // ✅ DODANE
};

export const useBootParamStore = create<BootParamStore>((set, get) => ({
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
    const {
      selectedSheetJSON,
      selectedSheetName,
      selectedCells,
    } = useTableStore.getState();

    if (!selectedSheetJSON || !selectedCells) return;

    console.log('[BootParamStore] Hydrating from TableStore');

    set({
      selectedSheetJSON,
      selectedSheetName,
      selectedCells,
    });
  },

  reset: () => set({
    selectedSheetJSON: undefined,
    selectedSheetName: undefined,
    selectedCells: undefined,
  }), // ✅ IMPLEMENTACJA RESETU
}));
