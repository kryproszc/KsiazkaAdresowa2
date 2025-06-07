'use client';

import React, { useEffect, useRef, useState } from 'react';
import { useTrainDevideStore } from '@/stores/trainDevideStore';
import { useTableStore } from '@/stores/tableStore';
import { TrainDevideTable } from '@/components/TrainDevideTable';
import { useUserStore } from '@/app/_components/useUserStore';
import { useStochResultsStore } from '@/stores/useStochResultsStore';

export default function WspolczynnikiMultiplikatywna(): React.ReactElement {
  const userId = useUserStore((s) => s.userId);
  const {
    trainDevide,
    selectedWeights: selectedCells,
    setTrainDevide,
    setSelectedWeights: setSelectedCells,
  } = useTrainDevideStore();
  const { selectedSheetJSON } = useTableStore();
  const hasInitializedRef = useRef(false);
  const [version, setVersion] = useState(0); // 🔁 do wymuszenia renderu

  const dev = useStochResultsStore((s) => s.dev);
  const sd = useStochResultsStore((s) => s.sd);
  const sigma = useStochResultsStore((s) => s.sigma);
  const setDev = useStochResultsStore((s) => s.setDev);
  const setSd = useStochResultsStore((s) => s.setSd);
  const setSigma = useStochResultsStore((s) => s.setSigma);

  // 🆕 Wyciągamy nagłówki wierszy i kolumn z Excela
  const rowHeaders = selectedSheetJSON?.slice(1).map((row) => row[0] ?? "") ?? [];
  const colHeaders = selectedSheetJSON?.[0]?.slice(1).map((cell) => cell ?? "") ?? [];

  useEffect(() => {
    const fetchTrainDevide = async () => {
      if (!selectedSheetJSON) return;
      const res = await fetch('http://localhost:8000/calc/mult_stoch', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          user_id: userId,
          paid_weights: [],
          paid_data: selectedSheetJSON,
          cl_data: [],
          cl_weights: [],
          triangle_raw: selectedSheetJSON,
          cl_weights_raw: [],
        }),
      });

      const json = await res.json();

      if (json.train_devide) {
        let matrix: number[][] = json.train_devide;
        const lastRow = matrix.at(-1);
        if (lastRow && lastRow.every((val) => val === 1)) {
          matrix = matrix.slice(0, -1);
        }
        setTrainDevide(matrix);
        if (!selectedCells) {
          const selected = matrix.map((row) => row.map(() => 1));
          setSelectedCells(selected);
        }
      }
    };

    if (!hasInitializedRef.current) {
      fetchTrainDevide();
      hasInitializedRef.current = true;
    }
  }, [selectedSheetJSON, selectedCells, setTrainDevide, setSelectedCells]);

  if (!trainDevide || !selectedCells) {
    return <div className="text-red-400 p-6 text-center text-lg">Brak danych w arkuszu...</div>;
  }

  const toggleCell = (i: number, j: number) => {
    const updated = selectedCells.map((row, rowIdx) =>
      rowIdx === i ? row.map((cell, colIdx) => (colIdx === j ? (cell === 1 ? 0 : 1) : cell)) : row
    );
    setSelectedCells(updated);
  };

  const handleSubmit = async () => {
    try {
      const res = await fetch('http://localhost:8000/calc/wspolczynniki_mult', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          user_id: userId,
          wagi_mult: selectedCells,
          paid_data: selectedSheetJSON,
        }),
      });

      const data = await res.json();
      setDev(data.dev || []);
      setSd(data.sd || []);
      setSigma(data.sigma || []);
      setVersion((v) => v + 1); // 🔁 wymusza rerender
    } catch (err) {
      console.error('❌ Błąd podczas wysyłania:', err);
    }
  };

 return (
  <div className="p-6 text-white">
    <div className="flex gap-6">
      <div className="w-1/10 min-w-[200px] p-4 bg-[#1e1e2f] text-white/80 border border-white/10 rounded self-stretch">
        <h4 className="font-semibold mb-4">Panel przycisków</h4>
        <button
          onClick={handleSubmit}
          className="w-full mt-4 bg-slate-700 hover:bg-slate-600 text-white px-4 py-1 rounded"
        >
          Wyznacz współczynniki
        </button>
      </div>

      <div className="flex-1">
        <h3 className="text-lg font-bold mb-4">Wynik: train_devide</h3>

        <TrainDevideTable 
          data={trainDevide} 
          rowHeaders={rowHeaders}
          colHeaders={colHeaders}
          selected={selectedCells} 
          onClick={toggleCell}
        />

        {(dev.length || sd.length || sigma.length) > 0 && (
          <div key={version} className="mt-10 overflow-auto rounded-xl border border-slate-700 shadow-md">
            <h4 className="text-white/90 text-md font-semibold px-4 pt-4">
              Współczynniki
            </h4>
            <table className="min-w-full table-auto border-collapse">
              <thead>
                <tr>
                  <td className="px-4 py-2 border bg-slate-800 text-white font-semibold">Rodzaj</td>
                  {colHeaders.map((header, i) => (
                    <td key={i} className="px-4 py-2 border bg-slate-800 text-center text-white font-semibold">
                      {header}
                    </td>
                  ))}
                </tr>
              </thead>
              <tbody>
                {[{ label: 'Dev', data: dev }, { label: 'SD', data: sd }, { label: 'σ', data: sigma }].map((row, i) => (
                  <tr key={i}>
                    <td className="px-4 py-2 border font-bold text-slate-400 bg-slate-800">
                      {row.label}
                    </td>
                    {row.data.map((val, j) => (
                      <td key={j} className="px-2 py-1 border bg-slate-800 text-right brightness-90">
                        <input
                          type="number"
                          step="0.0000000001"
                          defaultValue={val}
                          className="bg-transparent text-right w-full px-1 outline-none"
                          onBlur={(e) => {
                            const parsed = parseFloat(e.target.value);
                            if (!isNaN(parsed) && parsed !== val) {
                              const updated = [...row.data];
                              updated[j] = parsed;

                              if (row.label === 'Dev') setDev(updated);
                              if (row.label === 'SD') setSd(updated);
                              if (row.label === 'σ') setSigma(updated);
                              setVersion((v) => v + 1); // 🔁 odśwież po edycji
                            }
                          }}
                        />
                      </td>
                    ))}
                  </tr>
                ))}
              </tbody>
            </table>
          </div>
        )}
      </div>
    </div>
  </div>
);
}