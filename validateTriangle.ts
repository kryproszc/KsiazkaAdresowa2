// src/utils/validateTriangle.ts

import type { RowData } from "@/types/table";

function countNonEmptyCells(row: RowData | undefined): number {
  if (!row) return 0;
  const values = Object.values(row);
  let lastNonEmptyIndex = -1;

  for (let i = 0; i < values.length; i++) {
    if (values[i] !== null && values[i] !== undefined && values[i] !== '') {
      lastNonEmptyIndex = i;
    }
  }

  return lastNonEmptyIndex + 1;
}

export function validateTriangle(sheet: RowData[]): { isValid: boolean; reason?: string } {
  let prevCount = Infinity;
  let firstCount = Infinity;
  let isEnd = false;

  for (let i = 0; i < sheet.length; i++) {
    const row = sheet[i];
    const values = countNonEmptyCells(row);

    if (values === 0) {
      isEnd = true;
      continue;
    } else if (isEnd) {
      return { isValid: false, reason: `Wiersz ${i + 1} po pustym wierszu` };
    }

    if (i === 0) {
      firstCount = values;
      prevCount = values;
      continue;
    }

    if (i === 1 && values !== firstCount) {
      return { isValid: false, reason: `Wiersz 2 ma inną długość niż wiersz 1` };
    }

    if (values > prevCount) {
      return { isValid: false, reason: `Wiersz ${i + 1} ma więcej danych niż poprzedni` };
    }

    prevCount = values;
  }

  if (prevCount < 2) {
    return { isValid: false, reason: `Za mało danych w ostatnim wierszu` };
  }

  return { isValid: true };
}



