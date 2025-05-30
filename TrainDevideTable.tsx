import React from 'react';

interface Props {
  data: (number | null)[][];
  selected?: number[][];
  onClick?: (rowIdx: number, colIdx: number) => void;
}

export const TrainDevideTable = ({ data, selected, onClick }: Props): React.ReactElement => {
  return (
    <div className="overflow-auto rounded-xl border border-slate-700 shadow-md">
      <table className="min-w-full table-auto border-collapse text-sm text-slate-200">
        <thead>
          <tr>
            <th className="bg-slate-800 px-4 py-2 text-left font-semibold">AY</th>
            {data[0]?.map((_, colIdx) => (
              <th key={colIdx} className="bg-slate-800 px-4 py-2 text-center font-semibold">
                {colIdx + 1}
              </th>
            ))}
          </tr>
        </thead>
        <tbody>
          {data.map((row, i) => (
            <tr key={i}>
              <td className="bg-slate-800 px-4 py-2 text-slate-400 font-bold">{1981 + i}</td>
              {row.map((cell, j) => {
                const isNone = cell === null || cell === undefined;
                const isSelected = selected?.[i]?.[j] === 1;

                const cellClasses = [
                  'px-4 py-2 text-center border border-slate-700 whitespace-nowrap transition-colors duration-200',
                  isNone
                    ? 'bg-slate-900 text-slate-600' // brak danych = neutralny wygląd
                    : isSelected
                    ? 'bg-indigo-700/40 border-indigo-500 shadow-inner'
                    : 'bg-slate-900 brightness-75 hover:bg-slate-700/50',
                ].join(' ');

                return (
                  <td key={j} onClick={() => !isNone && onClick?.(i, j)} className={cellClasses}>
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
