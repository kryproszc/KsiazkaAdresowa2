
'use client';

import React, { useEffect, useRef, useState } from 'react';
import { useUserStore } from '@/app/_components/useUserStore';
import { useBootParamStore } from '@/stores/bootParamStore';
import { useBootParamResultsStore } from '@/stores/useBootParamResultsStore';
import { TrainDevideTable } from '@/components/TrainDevideTable';
import { useBootResultsStore } from '@/stores/useBootResultsStore'; // dodaj import

export default function WspolczynnikiBootParam(): React.ReactElement {
  const userId = useUserStore((s) => s.userId);
  const {
    selectedSheetJSON: sheetJSON,
    selectedCells,
    setSelectedCells,
  } = useBootParamStore();

  const {
    dev,
    sd,
    sigma,
    setDev,
    setSd,
    setSigma,
  } = useBootParamResultsStore();

  const [matrix, setMatrix] = useState<(number | null)[][]>([]);
  const [version, setVersion] = useState(0);
  const hasInitializedRef = useRef(false);

  useEffect(() => {
    const fetchTrainDevide = async () => {
      if (!sheetJSON) return;

      const res = await fetch('http://localhost:8000/calc/mult_stoch', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          user_id: userId,
          paid_weights: [],
          paid_data: sheetJSON,
          cl_data: [],
          cl_weights: [],
          triangle_raw: sheetJSON,
          cl_weights_raw: [],
        }),
      });

      const json = await res.json();
      if (json.train_devide) {
        const rawMatrix = json.train_devide as (number | string | null)[][];
        const cleanedMatrix: (number | null)[][] = rawMatrix.map((row) =>
          row.map((cell) => {
            const n = typeof cell === 'number' ? cell : Number(cell);
            return isNaN(n) ? null : n;
          })
        );

        setMatrix(cleanedMatrix);

        const isShapeValid =
          selectedCells?.length === cleanedMatrix.length &&
          selectedCells?.[0]?.length === cleanedMatrix[0]?.length;

        if (!isShapeValid) {
          const defaultSelected: number[][] = cleanedMatrix.map((row) =>
            row.map(() => 1)
          );
          setSelectedCells(defaultSelected);
        }

      }
    };

    if (!hasInitializedRef.current) {
      fetchTrainDevide();
      hasInitializedRef.current = true;
    }
  }, [sheetJSON, selectedCells, setSelectedCells, userId]);

  if (!matrix.length || !selectedCells || !selectedCells.length) {
    return (
      <div className="text-red-400 p-6 text-center text-lg">
        Brak danych w arkuszu...
      </div>
    );
  }

  const toggleCell = (i: number, j: number) => {
    const updated = selectedCells.map((row, rowIdx) =>
      rowIdx === i
        ? row.map((cell, colIdx) => (colIdx === j ? (cell === 1 ? 0 : 1) : cell))
        : row
    );
    setSelectedCells(updated);
  };

const handleSubmit = async () => {
  if (!sheetJSON || !selectedCells) {
    alert("Brakuje danych do wysłania.");
    return;
  }

  try {
    const res = await fetch('http://localhost:8000/calc/wspolczynniki_boot', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({
        user_id: userId,
        wagi_boot: selectedCells,
        paid_data: sheetJSON,
        triangle: sheetJSON, // ✅ DODANE
      }),
    });

    const data = await res.json();

    setDev(data.dev || []);
    setSd(data.sd || []);
    setSigma(data.sigma || []);

    const { setDev: setBootDev, setSd: setBootSd, setSigma: setBootSigma } = useBootResultsStore.getState();
    setBootDev(data.dev || []);
    setBootSd(data.sd || []);
    setBootSigma(data.sigma || []);

    setVersion((v) => v + 1);
  } catch (err) {
    console.error('❌ Błąd podczas wysyłania:', err);
  }
};


const selectedSanitized: (0 | 1)[][] = (selectedCells || []).map((row) =>
  row.map((cell) => (Number(cell) === 1 ? 1 : 0))
);
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
            data={matrix}
            selected={selectedSanitized}
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
                    {matrix[0]?.map((_, i) => (
                      <td key={i} className="px-4 py-2 border bg-slate-800 text-right text-white font-semibold">
                        {i + 1}
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
                                setVersion((v) => v + 1);
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


