
visually-hidden.tsx

export function VisuallyHidden({ children }: { children: React.ReactNode }) {
  return (
    <span
      style={{
        position: "absolute",
        width: "1px",
        height: "1px",
        margin: "-1px",
        padding: "0",
        overflow: "hidden",
        clip: "rect(0, 0, 0, 0)",
        whiteSpace: "nowrap",
        border: "0",
      }}
    >
      {children}
    </span>
  );
}


'use client';

import { useForm } from "react-hook-form";
import { zodResolver } from "@hookform/resolvers/zod";
import * as XLSX from "xlsx";
import { useEffect, useState } from "react";
import { useTableStore } from "@/stores/tableStore";
import { z } from "zod";
import { resetAllStores } from "@/lib/resetAllStores";
import { SheetSelect } from "@/components/SheetSelect";
import { Input } from "@/components/ui/input";
import { Button } from "@/components/ui/button";
import { Label } from "@/components/ui/label";
import {
  Card, CardHeader, CardTitle,
  CardContent, CardFooter
} from "@/components/ui/card";
import {
  AlertDialog, AlertDialogCancel, AlertDialogContent,
  AlertDialogDescription, AlertDialogFooter, AlertDialogHeader, AlertDialogTitle
} from "@/components/ui/alert-dialog";
import { Loader2 } from "lucide-react";
import { VisuallyHidden } from "@/components/ui/visually-hidden";

const schema = z.object({
  rowStart: z.coerce.number().min(1),
  rowEnd: z.coerce.number().min(1),
  colStart: z.coerce.number().min(1),
  colEnd: z.coerce.number().min(1),
  file: z.any()
});

type FormField = z.infer<typeof schema>;

function validateDataValues(data: any[][]) {
  console.log("Sprawdzam dane:");
  console.log(data);

  for (let rowIdx = 1; rowIdx < data.length; rowIdx++) { // Pomijamy nagłówek
    const row = data[rowIdx]!;

    // Policz puste na końcu
    let emptyCount = 0;
    for (let colIdx = row.length - 1; colIdx >= 0; colIdx--) {
      if (row[colIdx] === "") {
        emptyCount++;
      } else {
        break;
      }
    }

    const dataPart = row.slice(0, row.length - emptyCount); // tylko wypełnione dane

    for (const [cellIdx, cell] of dataPart.entries()) {
      console.log(`Row ${rowIdx}, Cell ${cellIdx}:`, cell, `Type: ${typeof cell}`);

      // 🛑 Jeśli puste miejsce w środku danych
      if (cell === "") {
        console.warn(`❌ Pusta komórka w środku danych!`, `Row ${rowIdx} Cell ${cellIdx}`);
        return false;
      }

      const numericValue = typeof cell === 'number' ? cell : Number(cell);

      if (isNaN(numericValue)) {
        console.warn(`❌ NIE jest liczbą:`, cell);
        return false;
      }
    }
  }
  return true;
}






export function InputDataTab() {
  const [showDialog, setShowDialog] = useState(false);
  const [showSuccessDialog, setShowSuccessDialog] = useState(false);
  const [showNoChangesDialog, setShowNoChangesDialog] = useState(false);
  const [showWarningDialog, setShowWarningDialog] = useState(false); // 🆕 Nowe ostrzeżenie
  const [isLoading, setIsLoading] = useState(false);
  const [progress, setProgress] = useState(0);

  const {
    workbook,
    isValid,
    selectedSheetJSON,
    previousSheetJSON,
    validationErrorReason,
    setWorkbook,
    getDefaultRange,
    setRangeAndUpdate,
    uploadedFileName,
    setUploadedFileName,
  } = useTableStore();

  const {
    register,
    handleSubmit,
    watch,
    setValue,
  } = useForm<FormField>({
    resolver: zodResolver(schema),
    defaultValues: {
      rowStart: 1,
      rowEnd: 1,
      colStart: 1,
      colEnd: 1,
    }
  });

  const file = watch("file");

  useEffect(() => {
    const { startRow, endRow, startCol, endCol } = useTableStore.getState();
    setValue("rowStart", startRow);
    setValue("rowEnd", endRow);
    setValue("colStart", startCol);
    setValue("colEnd", endCol);
  }, [setValue]);

  useEffect(() => {
    const unsubscribe = useTableStore.subscribe((state) => {
      setValue("rowStart", state.startRow);
      setValue("rowEnd", state.endRow);
      setValue("colStart", state.startCol);
      setValue("colEnd", state.endCol);
    });
    return unsubscribe;
  }, [setValue]);

  const handleFileLoad = () => {
    const f = file?.[0];
    if (!f) {
      alert("Najpierw wybierz plik.");
      return;
    }

    const reader = new FileReader();

    reader.onloadstart = () => {
      setIsLoading(true);
      setProgress(0);
    };

    reader.onprogress = (event) => {
      if (event.lengthComputable) {
        const percent = Math.round((event.loaded / event.total) * 100);
        setProgress(percent);
      }
    };

    reader.onload = (evt) => {
      const binaryStr = evt.target?.result;
      if (typeof binaryStr === "string") {
        try {
          const wb = XLSX.read(binaryStr, { type: "binary" });

          resetAllStores();

          const sheetName = wb.SheetNames[0];
          setWorkbook(wb);
          useTableStore.getState().setSelectedSheetName(sheetName);
          setUploadedFileName(f.name);
        } catch (error) {
          alert("Błąd podczas wczytywania pliku.");
        }
      }
      setIsLoading(false);
      setProgress(0);
    };

    reader.onerror = () => {
      alert("Błąd podczas wczytywania pliku.");
      setIsLoading(false);
    };

    reader.readAsBinaryString(f);
  };

  const handleAutoRange = () => {
    const range = getDefaultRange();
    if (!range) return;
    setValue("rowStart", range.startRow);
    setValue("rowEnd", range.endRow);
    setValue("colStart", range.startCol);
    setValue("colEnd", range.endCol);
  };

  const onSubmit = async (data: FormField) => {
    setShowDialog(false);
    setShowSuccessDialog(false);
    setShowNoChangesDialog(false);
    setShowWarningDialog(false); // 🆕 Resetujemy ostrzeżenie

    setRangeAndUpdate({
      startRow: data.rowStart,
      endRow: data.rowEnd,
      startCol: data.colStart,
      endCol: data.colEnd
    });

    setTimeout(() => {
      const { isValid, selectedSheetJSON, previousSheetJSON } = useTableStore.getState();
      const isSameData = JSON.stringify(selectedSheetJSON) === JSON.stringify(previousSheetJSON);

      if (!isValid) {
        setShowDialog(true);
      } else if (isSameData) {
        setShowNoChangesDialog(true);
      } else {
        // 🆕 sprawdzenie poprawności danych
        const validData = validateDataValues(selectedSheetJSON || []);
        if (!validData) {
          setShowWarningDialog(true);
        } else {
          setShowSuccessDialog(true);
        }
      }
    }, 0);
  };

  return (
    <div>
      {/* FORMULARZ */}
      <form onSubmit={handleSubmit(onSubmit)} className="p-4 border rounded flex flex-col gap-4">
        <Card>
          <CardHeader>
            <CardTitle>Wprowadź dane</CardTitle>
          </CardHeader>
          <CardContent className="space-y-4">
            {/* INPUT FILE */}
            <div className="flex items-center gap-4">
              <input
                type="file"
                accept=".xlsx, .xls"
                className="border p-2 rounded-lg"
                {...register("file")}
              />
              <Button
                type="button"
                onClick={handleFileLoad}
                disabled={!file || file.length === 0}
                className="bg-blue-500 text-white"
              >
                Załaduj plik
              </Button>
              {uploadedFileName && (
                <span className="text-sm text-green-400 ml-2">
                  Wczytano: <strong>{uploadedFileName}</strong>
                </span>
              )}
            </div>
            {/* SHEET SELECT */}
            <div>
              <Label>Wiersz arkusz</Label>
              <SheetSelect />
            </div>
            {/* INPUT RANGE */}
            <div className="grid grid-cols-2 gap-4">
              <div>
                <Label>Wiersz początkowy</Label>
                <Input type="number" disabled={!workbook} {...register("rowStart")} />
              </div>
              <div>
                <Label>Wiersz końcowy</Label>
                <Input type="number" disabled={!workbook} {...register("rowEnd")} />
              </div>
              <div>
                <Label>Kolumna początkowa</Label>
                <Input type="number" disabled={!workbook} {...register("colStart")} />
              </div>
              <div>
                <Label>Kolumna końcowa</Label>
                <Input type="number" disabled={!workbook} {...register("colEnd")} />
              </div>
            </div>
            <Button
              type="button"
              onClick={handleAutoRange}
              variant="outline"
              disabled={!workbook}
              className="bg-blue-500 text-white"
            >
              Wykryj zakres automatycznie
            </Button>
          </CardContent>
          <CardFooter>
            <Button
              type="submit"
              className="bg-blue-500 text-white"
              disabled={!workbook}
            >
              Wybierz
            </Button>
          </CardFooter>
        </Card>
      </form>

      {/* ALERTS */}
      {/* Błąd formatowania */}
      <AlertDialog open={showDialog} onOpenChange={setShowDialog}>
        <AlertDialogContent>
          <AlertDialogHeader className="flex flex-col items-center">
            <VisuallyHidden>
              <AlertDialogTitle>Błąd danych</AlertDialogTitle>
            </VisuallyHidden>
            <div className="flex items-center justify-center w-12 h-12 rounded-full bg-red-100 mb-4">
              <svg className="w-6 h-6 text-red-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                <path strokeLinecap="round" strokeLinejoin="round" strokeWidth="2" d="M6 18L18 6M6 6l12 12" />
              </svg>
            </div>
            <AlertDialogDescription className="text-center text-red-600 font-medium">
              {validationErrorReason || "Dane wejściowe nie spełniają określonego formatu. Sprawdź dane!"}
            </AlertDialogDescription>
          </AlertDialogHeader>
          <AlertDialogFooter>
            <AlertDialogCancel>Zamknij</AlertDialogCancel>
          </AlertDialogFooter>
        </AlertDialogContent>
      </AlertDialog>

      {/* Błąd — złe dane */}
      <AlertDialog open={showWarningDialog} onOpenChange={setShowWarningDialog}>
        <AlertDialogContent>
          <AlertDialogHeader className="flex flex-col items-center">
            <VisuallyHidden>
              <AlertDialogTitle>Ostrzeżenie</AlertDialogTitle>
            </VisuallyHidden>
            <div className="flex items-center justify-center w-12 h-12 rounded-full bg-yellow-100 mb-4">
              <svg className="w-6 h-6 text-yellow-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                <path strokeLinecap="round" strokeLinejoin="round" strokeWidth="2" d="M13 16h-1v-4h-1m0-4h.01M12 20c4.418 0 8-3.582 8-8s-3.582-8-8-8-8 3.582-8 8 3.582 8 8 8z" />
              </svg>
            </div>
            <AlertDialogDescription className="text-center text-yellow-600 font-medium">
              Dane są w poprawnym formacie, ale występują w nich braki lub niedozwolone wartości.
            </AlertDialogDescription>
          </AlertDialogHeader>
          <AlertDialogFooter>
            <AlertDialogCancel>OK</AlertDialogCancel>
          </AlertDialogFooter>
        </AlertDialogContent>
      </AlertDialog>

      {/* Sukces */}
      <AlertDialog open={showSuccessDialog} onOpenChange={setShowSuccessDialog}>
        <AlertDialogContent>
          <AlertDialogHeader className="flex flex-col items-center">
            <VisuallyHidden>
              <AlertDialogTitle>Powiadomienie</AlertDialogTitle>
            </VisuallyHidden>
            <div className="flex items-center justify-center w-12 h-12 rounded-full bg-green-100 mb-4">
              <svg className="w-6 h-6 text-green-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                <path strokeLinecap="round" strokeLinejoin="round" strokeWidth="2" d="M5 13l4 4L19 7" />
              </svg>
            </div>
            <AlertDialogDescription className="text-center text-green-600 font-medium">
              Dane zostały poprawnie wczytane.
            </AlertDialogDescription>
          </AlertDialogHeader>
          <AlertDialogFooter>
            <AlertDialogCancel>OK</AlertDialogCancel>
          </AlertDialogFooter>
        </AlertDialogContent>
      </AlertDialog>

      {/* Brak zmian */}
      <AlertDialog open={showNoChangesDialog} onOpenChange={setShowNoChangesDialog}>
        <AlertDialogContent>
          <AlertDialogHeader className="flex flex-col items-center">
            <VisuallyHidden>
              <AlertDialogTitle>Informacja</AlertDialogTitle>
            </VisuallyHidden>
            <div className="flex items-center justify-center w-12 h-12 rounded-full bg-blue-100 mb-4">
              <svg className="w-6 h-6 text-blue-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                <path strokeLinecap="round" strokeLinejoin="round" strokeWidth="2" d="M13 16h-1v-4h-1m0-4h.01M12 20c4.418 0 8-3.582 8-8s-3.582-8-8-8-8 3.582-8 8 3.582 8 8 8z" />
              </svg>
            </div>
            <AlertDialogDescription className="text-center text-blue-600 font-medium">
              Nie dokonano żadnych zmian w danych.
            </AlertDialogDescription>
          </AlertDialogHeader>
          <AlertDialogFooter>
            <AlertDialogCancel>OK</AlertDialogCancel>
          </AlertDialogFooter>
        </AlertDialogContent>
      </AlertDialog>

      {/* Spinner podczas ładowania */}
      {isLoading && (
        <div className="fixed inset-0 z-50 flex items-center justify-center bg-black bg-opacity-50">
          <div className="bg-[#1e1e2f] rounded-lg p-8 flex flex-col items-center w-80">
            <Loader2 className="animate-spin h-10 w-10 text-white mb-6" />
            <div className="w-full bg-gray-700 rounded-full h-4 mb-4 overflow-hidden">
              <div
                className="bg-gray-300 h-full transition-all duration-300 ease-in-out"
                style={{ width: `${progress}%` }}
              />
            </div>
            <div className="text-white text-sm font-medium">
              Ładowanie... {progress}%
            </div>
          </div>
        </div>
      )}
    </div>
  );
}



import { create } from "zustand";
import * as XLSX from "xlsx";
import type { RowData } from "@/types/table";
import { parseRange, toExcelRange } from "../utils/parseTableRange";
import { resetAllStores } from "@/lib/resetAllStores";

type TableStore = {
  table?: File;
  isHydrated: boolean;
  workbook?: XLSX.WorkBook;
  selectedSheet?: XLSX.WorkSheet;
  selectedSheetJSON?: RowData[];
  selectedCells?: number[][];
  selectedSheetName?: string | undefined;
  isValid?: boolean;
  validationErrorReason?: string;

  previousSheetJSON?: RowData[]; // 🆕 nowość: trzymanie starego JSONa
  setPreviousSheetJSON: (data: RowData[]) => void; // 🆕 setter do starego JSONa

  sheetRange?: string;
  startRow: number;
  endRow: number;
  startCol: number;
  endCol: number;

  uploadedFileName?: string;

  processedData?: RowData[];
  setProcessedData: (data: RowData[]) => void;

  processedTriangle?: RowData[];
  setProcessedTriangle: (data: RowData[]) => void;

  clData?: RowData[];
  setClData: (data: RowData[]) => void;

  clWeights?: RowData[];
  setClWeights: (weights: RowData[]) => void;

  setStartRow: (startRow: number) => void;
  setEndRow: (endRow: number) => void;
  setStartCol: (startCol: number) => void;
  setEndCol: (endCol: number) => void;
  setRangeAndUpdate: (range: { startRow: number; endRow: number; startCol: number; endCol: number }) => void;
  getDefaultRange: () => { startRow: number; startCol: number; endRow: number; endCol: number } | undefined;

  setIsHydrated: (isHydrated: boolean) => void;
  setWorkbook: (workbook?: XLSX.WorkBook) => void;
  setSelectedSheetName: (selectedSheetName: string | undefined) => void;
  updateSheetJSON: () => void;
  resetData: () => void;

  getSheetNames: () => string[] | undefined;
  setSelectedCells: (selectedCells: number[][]) => void;

  setUploadedFileName: (fileName: string) => void;
};

function customValidate(jsonData: any[][]) {
  if (!Array.isArray(jsonData) || jsonData.length < 2) {
    return {
      isValid: false,
      reason: "Brak danych lub dane mają niepoprawny format."
    };
  }

  const firstRow = jsonData[0];
  if (!Array.isArray(firstRow) || firstRow.length < 2) {
    return {
      isValid: false,
      reason: "Pierwszy wiersz nie ma wystarczającej liczby kolumn."
    };
  }

  for (let i = 1; i < firstRow.length; i++) {
    const val = firstRow[i];
    if (val === undefined || val === null) {
      return {
        isValid: false,
        reason: `Pierwszy wiersz: Kolumna ${i + 1} nie ma wartości.`
      };
    }
  }

  // Sprawdzamy kolejne wiersze
  for (let rowIdx = 1; rowIdx < jsonData.length; rowIdx++) {
    const row = jsonData[rowIdx];

    if (!row) {
      return {
        isValid: false,
        reason: `Rząd ${rowIdx + 1}: Brak danych w tym wierszu.`
      };
    }

    if (typeof row[0] !== 'number') {
      return {
        isValid: false,
        reason: `Rząd ${rowIdx + 1}: Pierwsza kolumna nie jest rokiem (liczbą).`
      };
    }

    let emptyCount = 0;
    for (let colIdx = row.length - 1; colIdx >= 1; colIdx--) {
      if (row[colIdx] === "") {
        emptyCount++;
      } else {
        break;
      }
    }

    const expectedEmpty = rowIdx - 1;

    if (emptyCount !== expectedEmpty) {
      return {
        isValid: false,
        reason: `Rząd ${rowIdx + 1}: Oczekiwano ${expectedEmpty} pustych wartości na końcu, znaleziono ${emptyCount}.`
      };
    }
  }

  return {
    isValid: true
  };
}

export const useTableStore = create<TableStore>((set, get) => ({
  table: undefined,
  isHydrated: false,

  startRow: 1,
  endRow: 1,
  startCol: 1,
  endCol: 1,

  uploadedFileName: undefined,

  processedData: undefined,
  setProcessedData: (data) => set({ processedData: data }),

  previousSheetJSON: undefined, // 🆕 dodane
  setPreviousSheetJSON: (data) => set({ previousSheetJSON: data }), // 🆕 dodane

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
      sheetRange: sheet ? sheet["!ref"] : undefined,
    });
  },

  setSelectedSheetName: (sheetName) => {
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
    if (!sheet || !sheet["!ref"]) {
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
      sheetRange: sheet["!ref"],
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

    const tempSheet = { ...sheet, ["!ref"]: range };

    const jsonData = XLSX.utils.sheet_to_json<RowData>(tempSheet, {
      header: 1,
      blankrows: true,
      defval: "",
    });
    console.log("JSON Data:", JSON.stringify(jsonData, null, 2));

    const validation = customValidate(jsonData);

    if (!validation.isValid) {
      set({
        isValid: false,
        validationErrorReason: "Dane wejściowe nie spełniają określonego formatu. Sprawdź dane!",
        selectedSheetJSON: undefined,
        selectedCells: undefined,
      });
      return;
    }

    const selectedCells = jsonData.map((row) =>
      row.map((cell) => {
        const numeric = typeof cell === "number" ? cell : Number(cell);
        return !isNaN(numeric) ? 1 : 0;
      })
    );

    set({
      previousSheetJSON: get().selectedSheetJSON, // 🆕 zapamiętujemy stare przed nadpisaniem
      selectedSheetJSON: jsonData,
      selectedCells,
      isValid: true,
      validationErrorReason: undefined,
    });
  },

  resetData: () =>
    set({
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

  setUploadedFileName: (fileName) => set({ uploadedFileName: fileName }),
}));

