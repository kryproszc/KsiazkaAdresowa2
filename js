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
  Card, CardHeader, CardTitle, CardDescription,
  CardContent, CardFooter
} from "@/components/ui/card";
import {
  AlertDialog, AlertDialogCancel, AlertDialogContent,
  AlertDialogDescription, AlertDialogFooter, AlertDialogHeader, AlertDialogTitle
} from "@/components/ui/alert-dialog";
import { Loader2 } from "lucide-react"; // Dodajemy spinner ikonkę

const schema = z.object({
  rowStart: z.coerce.number().min(1),
  rowEnd: z.coerce.number().min(1),
  colStart: z.coerce.number().min(1),
  colEnd: z.coerce.number().min(1),
  file: z.any()
});

type FormField = z.infer<typeof schema>;

export function InputDataTab() {
  const [showDialog, setShowDialog] = useState(false);
  const [uploadedFileName, setUploadedFileName] = useState<string | null>(null);
  const [isLoading, setIsLoading] = useState(false); // Nowy stan loading
  const [progress, setProgress] = useState(0); // Nowy stan progres

  const {
    workbook,
    isValid,
    validationErrorReason,
    setWorkbook,
    getDefaultRange,
    setRangeAndUpdate
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
    const unsub = useTableStore.subscribe((state) => {
      setValue("rowStart", state.startRow);
      setValue("rowEnd", state.endRow);
      setValue("colStart", state.startCol);
      setValue("colEnd", state.endCol);
    });
    return () => unsub();
  }, [setValue]);

  useEffect(() => {
    if (isValid === false) {
      setShowDialog(true);
    }
  }, [isValid]);

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

  const onSubmit = (data: FormField) => {
    setRangeAndUpdate({
      startRow: data.rowStart,
      endRow: data.rowEnd,
      startCol: data.colStart,
      endCol: data.colEnd
    });
  };

  return (
    <div>
      <form onSubmit={handleSubmit(onSubmit)} className="p-4 border rounded flex flex-col gap-4">
        <Card>
          <CardHeader>
            <CardTitle>Wprowadź dane</CardTitle>
            <CardDescription />
          </CardHeader>

          <CardContent className="space-y-4">
            <div className="flex items-center gap-4">
              <input
                type="file"
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
                <span className="text-sm text-muted-foreground ml-2">
                  Wczytano: <strong>{uploadedFileName}</strong>
                </span>
              )}
            </div>

            <div>
              <Label>Wiersz arkusz</Label>
              <SheetSelect />
            </div>

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

      {/* AlertDialog z dokładnym błędem */}
      <AlertDialog open={showDialog} onOpenChange={setShowDialog}>
        <AlertDialogContent>
          <AlertDialogHeader>
            <AlertDialogTitle>Błąd danych</AlertDialogTitle>
            <AlertDialogDescription>
              {validationErrorReason || "Wprowadzone dane są niepoprawne. Sprawdź zakres i spróbuj ponownie."}
            </AlertDialogDescription>
          </AlertDialogHeader>
          <AlertDialogFooter>
            <AlertDialogCancel>Zamknij</AlertDialogCancel>
          </AlertDialogFooter>
        </AlertDialogContent>
      </AlertDialog>

      {/* Spinner Overlay */}
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

      {/* Debug panel */}
      {process.env.NODE_ENV === "development" && (
        <pre className="mt-4 p-4 bg-muted text-sm overflow-auto max-h-[400px] rounded border">
          <code>{JSON.stringify(useTableStore.getState().selectedSheetJSON, null, 2)}</code>
        </pre>
      )}
    </div>
  );
}
