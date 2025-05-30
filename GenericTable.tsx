import React from 'react';

type GenericTableProps = {
  data: number[][];
  selectedCells?: number[][];
  onCellClick?: (i: number, j: number) => void;
  startYear?: number; // <- można dodać domyślny rok np. 1981
};

export const GenericTable = ({
  data,
  selectedCells,
  onCellClick,
  startYear = 1981, // 👈 domyślnie zaczynamy od 1981
}: GenericTableProps): React.ReactElement => (
  <div className="overflow-auto border border-white/10 rounded mb-6">
    <table className="min-w-full text-sm text-white/90">
      <thead>
        <tr>
          <th className="bg-[#1e1e2f] px-3 py-1 font-semibold text-left">AY</th>
          {data[0]?.map((_, colIdx) => (
            <th key={colIdx} className="bg-[#1e1e2f] px-3 py-1 font-semibold text-center">
              {colIdx + 1}
            </th>
          ))}
        </tr>
      </thead>
      <tbody>
        {data.map((row, i) => (
          <tr key={i}>
            <td className="bg-[#1e1e2f] px-3 py-1 text-left text-white/80 font-medium">
              {startYear + i}
            </td>
            {row.map((value, j) => {
              const selected = selectedCells?.[i]?.[j] === 1;
              return (
                <td
                  key={j}
                  onClick={() => onCellClick?.(i, j)}
                  className={`px-3 py-1 text-center whitespace-nowrap cursor-pointer ${
                    selected ? 'bg-blue-900' : 'bg-slate-800'
                  } hover:brightness-125 transition`}
                >
                  {typeof value === 'number' ? value.toFixed(3) : '-'}
                </td>
              );
            })}
          </tr>
        ))}
      </tbody>
    </table>
  </div>
);
