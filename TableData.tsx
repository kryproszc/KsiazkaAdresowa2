import type { RowData } from '@/types/table';

interface Props {
  data: RowData[];
  selected?: RowData[];
  onClick?: (x: number, y: number) => void;
}

export function TableData({ data, selected, onClick }: Props) {
  return (
    <div className="overflow-auto rounded-xl border border-slate-700 shadow-md">
      <table className="min-w-full table-auto border-collapse">
        <tbody className="text-sm text-slate-200">
          {data.map((row, i) => (
            <tr
              key={i}
              className="hover:bg-slate-800/30 transition-colors duration-200"
            >
              {row.map((cell, j) => {
                const isAY = j === 0;
                const isHeader = i === 0;
                const isEmpty = cell === null || cell === undefined || cell === '' || cell === '-';
                const isSelected = !isEmpty && selected?.[i]?.[j] === 1;

                const content =
                  typeof cell === 'number'
                    ? new Intl.NumberFormat('pl-PL', { maximumFractionDigits: 6 }).format(cell)
                    : isEmpty
                    ? '-'
                    : cell;

                return (
                    <td
                      key={j}
                      className={`px-4 py-2 border border-slate-600 whitespace-nowrap transition-colors duration-200
                        ${
                          isHeader
                            ? "bg-slate-800 text-white font-semibold uppercase"
                            : isAY
                            ? "bg-slate-800 font-bold text-slate-400"
                            : isSelected
                            ? "bg-indigo-700/40 border border-indigo-500 shadow-inner"
                            : "bg-slate-800 brightness-75"
                        }
                        cursor-pointer
                      `}
                      onClick={() => onClick?.(i, j)}
                    >
                      {content}
                    </td>
                );
              })}
            </tr>
          ))}
        </tbody>
      </table>
    </div>
  );
}
