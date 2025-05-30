export type RowData = Record<string, any>;

function countValues(row: RowData | undefined): number {
  if (!row) return 0
  let count = 0;
  for (const key in row) {
    if (row[key]) {
      count++
    } else {
      break;
    }
  }
  return count;
}

export function isTriangle(sheet: RowData[]): boolean {
  let prevCount = Infinity;
  let firstCount = Infinity;
  let isEnd = false;

  for (let i = 0; i < sheet.length; i++) {
    const row = sheet[i];
    const values = countValues(row);
    if (values == 0) {
      isEnd = true
      continue
    } else if (isEnd) {
      return false
    }

    if (i === 0 || !row) {
      firstCount = values;
      continue;
    }
    if (i == 1 && values != firstCount) {
      return false;
    }

    if (values >= prevCount) {
      return false;
    }

    prevCount = values;
  }

  return prevCount == 2;
}
