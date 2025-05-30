import { create } from "zustand"
import * as XLSX from 'xlsx';
import type { RowData } from "@/types/table";
import { parseRange, toExcelRange } from "../utils/parseTableRange";
import { validateTriangle } from "@/utils/validateTriangle";
import { resetAllStores } from "@/lib/resetAllStores";

type TableStore = {
  table?: File
  isHydrated: boolean
  workbook?: XLSX.WorkBook
  selectedSheet?: XLSX.WorkSheet
  selectedSheetJSON?: RowData[]
  selectedCells?: number[][]
  selectedSheetName?: string | undefined
  isValid?: boolean
  validationErrorReason?: string

  sheetRange?: string
  startRow: number,
  endRow: number,
  startCol: number,
  endCol: number

  processedData?: RowData[]
  setProcessedData: (data: RowData[]) => void

  processedTriangle?: RowData[]
  setProcessedTriangle: (data: RowData[]) => void

  clData?: RowData[]
  setClData: (data: RowData[]) => void

  clWeights?: RowData[]
  setClWeights: (weights: RowData[]) => void

  setStartRow: (startRow: number) => void
  setEndRow: (endRow: number) => void
  setStartCol: (startCol: number) => void
  setEndCol: (endCol: number) => void
  setRangeAndUpdate: (range: { startRow: number, endRow: number, startCol: number, endCol: number }) => void
  getDefaultRange: () => { startRow: number, startCol: number, endRow: number, endCol: number } | undefined

  setIsHydrated: (isHydrated: boolean) => void
  setWorkbook: (workbook?: XLSX.WorkBook) => void
  setSelectedSheetName: (selectedSheetName: string | undefined) => void
  updateSheetJSON: () => void
  resetData: () => void;


  getSheetNames: () => string[] | undefined
  setSelectedCells: (selectedCells: number[][]) => void
}

export const useTableStore = create<TableStore>((set, get) => ({
  table: undefined,
  isHydrated: false,

  startRow: 1,
  endRow: 1,
  startCol: 1,
  endCol: 1,

  processedData: undefined,
  setProcessedData: (data) => set({ processedData: data }),

  processedTriangle: undefined,
  setProcessedTriangle: (data) => set({ processedTriangle: data }),

  clData: undefined,
  setClData: (data) => set({ clData: data }),

  clWeights: undefined,
  setClWeights: (weights) => set({ clWeights: weights }),

  setStartRow: (startRow) => set({ startRow }),
  setEndRow: (endRow) => set({ endRow }),
  setStartCol: (startCol) => set({ startCol }),
  setEndCol: (endCol) => set({ endCol }),

  setRangeAndUpdate: (range) => {
    set({
      startRow: range.startRow,
      endRow: range.endRow,
      startCol: range.startCol,
      endCol: range.endCol,
    });
    get().updateSheetJSON();
  },

  getDefaultRange: () => {
    const defaultRef = get().sheetRange;
    if (!defaultRef) return;
    return parseRange(defaultRef);
  },

  setIsHydrated: (isHydrated) => set({ isHydrated }),

  setWorkbook: (workbook) => {
    const selected = workbook?.SheetNames[0];
    const sheet = selected ? workbook?.Sheets[selected] : undefined;

    set({
      workbook,
      selectedSheetName: selected,
      sheetRange: sheet ? sheet['!ref'] : undefined,
    });
  },

setSelectedSheetName: (sheetName) => {
  // 🔄 Resetujemy cały stan (również clData / clWeights)
  resetAllStores();

  if (!sheetName) {
    set({
      selectedSheetName: undefined,
      selectedSheet: undefined,
      selectedSheetJSON: undefined,
      isValid: undefined,
      validationErrorReason: undefined,
    });
    return;
  }

  const sheet = get().workbook?.Sheets[sheetName];
  if (!sheet || !sheet['!ref']) {
    set({
      selectedSheetName: undefined,
      selectedSheet: undefined,
      selectedSheetJSON: undefined,
      isValid: undefined,
      validationErrorReason: undefined,
    });
    return;
  }

  set({
    selectedSheetName: sheetName,
    selectedSheet: sheet,
    sheetRange: sheet['!ref'],
  });
},


updateSheetJSON: () => {
  const sheetName = get().selectedSheetName;
  if (!sheetName) return;

  const sheet = get().workbook?.Sheets[sheetName];
  if (!sheet) return;

  const range = toExcelRange({
    startRow: get().startRow,
    endRow: get().endRow,
    startCol: get().startCol,
    endCol: get().endCol,
  });

  const tempSheet = { ...sheet, ['!ref']: range };

  const jsonData = XLSX.utils.sheet_to_json<RowData>(tempSheet, {
    header: 1,
    blankrows: true,
    defval: '',
  });

  const validation = validateTriangle(jsonData);

  // 🛑 Jeśli dane są niepoprawne – nie zapisujemy sheetJSON
  if (!validation.isValid) {
    set({
      isValid: false,
      validationErrorReason: validation.reason,
      selectedSheetJSON: undefined,
      selectedCells: undefined,
    });
    return;
  }

  const selectedCells = jsonData.map((row) =>
    row.map((cell) => {
      const numeric = typeof cell === 'number' ? cell : Number(cell);
      return !isNaN(numeric) ? 1 : 0;
    })
  );

  set({
    selectedSheetJSON: jsonData,
    selectedCells,
    isValid: true,
    validationErrorReason: undefined,
  });
}
,
resetData: () => set({
  selectedSheetJSON: undefined,
  selectedCells: undefined,
  isValid: undefined,
  validationErrorReason: undefined,
  processedData: undefined,
  processedTriangle: undefined,
  clData: undefined,
  clWeights: undefined,
}),


  getSheetNames: () => get().workbook?.SheetNames,
  setSelectedCells: (selectedCells) => set({ selectedCells }),
}));


