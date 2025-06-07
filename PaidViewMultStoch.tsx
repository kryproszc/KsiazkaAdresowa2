'use client';

import { TableData } from '@/components/TableData';
import { useMultStochStore } from '@/stores/multStochStore';
import { useMutation } from '@tanstack/react-query';
import { useEffect } from 'react';
import { useTrainDevideStore } from '@/stores/trainDevideStore';
import { useUserStore } from '@/app/_components/useUserStore';
import { useTableStore } from '@/stores/tableStore'; // 🆕 dodane

export function PaidViewMultStoch() {
  const userId = useUserStore((s) => s.userId);
  const {
    selectedCells,
    selectedSheetJSON: sheetJSON,
    selectedSheetName: sheetName,
    hydrateFromTableStore,
  } = useMultStochStore();

  // 🆕 Obserwuj dane z tableStore (tak samo jak w PaidViewBootParam)
  const sourceSheetJSON = useTableStore((s) => s.selectedSheetJSON);
  const sourceSelectedCells = useTableStore((s) => s.selectedCells);
  const sourceSheetName = useTableStore((s) => s.selectedSheetName);

  useEffect(() => {
    if (sourceSheetJSON && sourceSelectedCells) {
      console.log('[hydrate] Sync MultStochStore with TableStore');
      hydrateFromTableStore();
    }
  }, [sourceSheetJSON, sourceSelectedCells, sourceSheetName, hydrateFromTableStore]);

  const { setTrainDevide } = useTrainDevideStore();

  const mutation = useMutation({
    mutationFn: async () => {
      const res = await fetch("http://localhost:8000/calc/mult_stoch", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({
          user_id: userId,
          paid_weights: selectedCells,
          paid_data: sheetJSON,
          cl_data: [],
          cl_weights: [],
          triangle_raw: sheetJSON,
          cl_weights_raw: selectedCells,
        }),
      });

      if (!res.ok) throw new Error('Błąd backendu');
      return res.json();
    },
    onSuccess: (data) => {
      console.log('✅ MultStoch OK', data);
      if (data.train_devide) {
        setTrainDevide(data.train_devide);
      }
    },
    onError: (err) => console.error('❌ MultStoch error:', err),
  });

  if (!sheetJSON) {
    return <div className="text-red-400">Brak danych arkusza</div>;
  }

  return (
    <div className="flex flex-col gap-6 p-6">
      <div className="flex gap-6">
        {/* Panel przycisków */}
        <div className="w-1/5 min-w-[200px] p-4 bg-[#1e1e2f] text-white/80 border border-white/10 rounded">
          <h4 className="font-semibold mb-4">Panel przycisków</h4>

          <button
            onClick={() => mutation.mutate()}
            className="w-full mt-4 bg-slate-700 hover:bg-slate-600 text-white px-4 py-1 rounded"
          >
            Zatwierdź trójkąt
          </button>
        </div>

        {/* Tabela danych */}
        <div className="flex-1">
          <h3 className="text-lg font-bold mb-4 text-white">Arkusz: {sheetName}</h3>
          <TableData data={sheetJSON} /> {/* 👈 bez columns, identycznie jak w BootParam */}
        </div>
      </div>
    </div>
  );
}
