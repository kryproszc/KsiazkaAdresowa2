export function colLetterToNumber(col: string): number {
  let num = 0;
  for (let i = 0; i < col.length; i++) {
    num = num * 26 + (col.charCodeAt(i) - 'A'.charCodeAt(0) + 1);
  }
  return num;
}

export function colNumberToLetter(num: number): string {
  let col = '';
  while (num > 0) {
    const rem = (num - 1) % 26;
    col = String.fromCharCode('A'.charCodeAt(0) + rem) + col;
    num = Math.floor((num - 1) / 26);
  }
  return col;
}

export function parseRange(range: string) {
  const [start, end] = range.split(':');

  if (!start || !end) throw new Error('Invalid range');

  const matchStart = start.match(/^([A-Z]+)(\d+)$/);
  const matchEnd = end.match(/^([A-Z]+)(\d+)$/);

  if (!matchStart || !matchEnd
    || !matchStart[1] || !matchStart[2] || !matchEnd[1] || !matchEnd[2]
  ) throw new Error('Invalid range');

  const startCol = colLetterToNumber(matchStart[1]);
  const startRow = parseInt(matchStart[2], 10);
  const endCol = colLetterToNumber(matchEnd[1]);
  const endRow = parseInt(matchEnd[2], 10);

  return { startRow, endRow, startCol, endCol };
}

export function toExcelRange({
  startRow,
  endRow,
  startCol,
  endCol,
}: {
  startRow: number;
  endRow: number;
  startCol: number;
  endCol: number;
}): string {
  const start = `${colNumberToLetter(startCol)}${startRow}`;
  const end = `${colNumberToLetter(endCol)}${endRow}`;
  return `${start}:${end}`;
}
