'use client';

import { TableData } from '@/components/TableData';
import { useTableStore } from '@/stores/tableStore';

export function CLTab(): React.ReactElement | null {
  const {
    clData,
    clWeights,
    setClWeights,
  } = useTableStore();

  const handleClick = (i: number, j: number) => {
    if (!clWeights) return;

    const updated = clWeights.map((row, rowIndex) =>
      rowIndex === i
        ? row.map((cell, colIndex) =>
            colIndex === j ? (cell === 1 ? 0 : 1) : cell
          )
        : [...row]
    );

    setClWeights(updated);
  };

  const resetSelection = () => {
    if (!clData) return;

    const newWeights = clData.slice(1).map((row) => row.slice(1).map(() => 1));
    setClWeights(newWeights);
  };

  if (!clData || !clWeights) {
    return <div className="text-red-400">Brak danych do wyświetlenia</div>;
  }

  return (
    <div className="flex flex-col gap-6 p-6">
      <div className="flex gap-6">
        {/* Panel boczny */}
        <div className="w-1/5 min-w-[200px] border border-white/10 rounded p-4 bg-[#1e1e2f] text-white/80">
          <h4 className="font-semibold mb-4">Panel przycisków</h4>

          <button
            onClick={resetSelection}
            className="w-full px-4 py-1 rounded bg-slate-600 hover:bg-slate-500 text-white font-medium transition-colors"
          >
            Resetuj zaznaczenia
          </button>
        </div>

        {/* Tabela */}
        <div className="flex-1">
          <h3 className="text-lg font-bold mb-4 text-white">Przetworzony trójkąt:</h3>
          <TableData
            data={clData}
            selected={[
              Array(clData[0]?.length || 0).fill(1),
              ...clWeights.map((row) => [1, ...row]),
            ]}
            onClick={(i, j) => handleClick(i - 1, j - 1)}
          />
        </div>
      </div>
    </div>
  );
}
