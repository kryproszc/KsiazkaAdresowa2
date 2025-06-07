'use client';

import React from 'react';

interface TrainDevideTableProps {
  data: (number | null)[][];
  rowHeaders: (string | number)[];
  colHeaders: (string | number)[];
  selected?: number[][]; // Tablica 0/1 do zaznaczania komórek
  onClick?: (rowIdx: number, colIdx: number) => void;
}

// Funkcja pomocnicza do liczenia ostatniej niepustej kolumny
const getMaxNonEmptyCols = (matrix: (number | null)[][]) => {
  return Math.max(
    ...matrix.map((row) => {
      for (let i = row.length - 1; i >= 0; i--) {
        if (row[i] !== null && row[i] !== 0) {
          return i + 1;
        }
      }
      return 0;
    })
  );
};

export const TrainDevideTable = ({
  data,
  rowHeaders,
  colHeaders,
  selected,
  onClick,
}: TrainDevideTableProps): React.ReactElement => {
  if (data.length === 0) {
    return (
      <div className="text-red-400 text-center py-4">Brak danych</div>
    );
  }

  // 🔥 Automatyczne przycięcie kolumn
  const maxCols = getMaxNonEmptyCols(data);
  const trimmedData = data.map((row) => row.slice(0, maxCols));
  const trimmedColHeaders = colHeaders.slice(0, maxCols);
  const trimmedSelected = selected?.map((row) => row.slice(0, maxCols));

  return (
    <div className="overflow-auto rounded-xl border border-slate-700 shadow-md">
      <table className="min-w-full table-auto border-collapse text-sm text-slate-200">
        <thead>
          <tr>
            {/* Pierwsza komórka na rogu tabeli */}
            <th className="bg-slate-800 px-4 py-2 text-left font-semibold">AY</th>
            {/* Dynamiczne nagłówki kolumn */}
            {trimmedColHeaders.map((header, colIdx) => (
              <th
                key={colIdx}
                className="bg-slate-800 px-4 py-2 text-center font-semibold"
              >
                {header}
              </th>
            ))}
          </tr>
        </thead>
        <tbody>
          {trimmedData.map((row, i) => (
            <tr key={i}>
              {/* Dynamiczne nagłówki wierszy */}
              <td className="bg-slate-800 px-4 py-2 text-slate-400 font-bold">
                {rowHeaders[i]}
              </td>
              {row.map((cell, j) => {
                const isNone = cell === null || cell === undefined;
                const isSelected = trimmedSelected?.[i]?.[j] === 1;

                const cellClasses = [
                  'px-4 py-2 text-center border border-slate-700 whitespace-nowrap transition-colors duration-200',
                  isNone
                    ? 'bg-slate-900 text-slate-600' // Brak danych — neutralny wygląd
                    : isSelected
                    ? 'bg-indigo-700/40 border-indigo-500 shadow-inner' // Wybrane
                    : 'bg-slate-900 brightness-75 hover:bg-slate-700/50', // Standard + hover
                ].join(' ');

                return (
                  <td
                    key={j}
                    onClick={() => !isNone && onClick?.(i, j)}
                    className={cellClasses}
                  >
                    {typeof cell === 'number'
                      ? cell.toLocaleString('pl-PL', { maximumFractionDigits: 6 })
                      : '-'}
                  </td>
                );
              })}
            </tr>
          ))}
        </tbody>
      </table>
    </div>
  );
};
