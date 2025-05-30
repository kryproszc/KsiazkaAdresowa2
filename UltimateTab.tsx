'use client';

import { useEffect } from 'react';
import * as XLSX from 'xlsx';
import { saveAs } from 'file-saver';

import { ControlPanel } from '@/components/Simulation/ControlPanel';
import { Charts } from '@/components/Simulation/Charts';
import { StatsTables } from '@/components/Simulation/StatsTables';
import { useSimulationRunner } from '@/hooks/useSimulationRunner';
import { getPercentile } from '@/services/simulationApi';
import { useUltimateStore } from '@/stores/useUltimateStore';
import { OverlayLoaderWithProgress } from '@/components/ui/OverlayLoaderWithProgress';
import { useUserStore } from '@/app/_components/useUserStore';

export function UltimateTab() {
  const userId = useUserStore((s) => s.userId);

  const simulation = useSimulationRunner();

  const {
    percentileInputSim,
    percentileInputDiff,
    percentileMatch,
    setPercentileMatch,
  } = useUltimateStore();

  const requestId = useUltimateStore((s) => s.requestId);

  const handleQuantileClick = async () => {
    if (!requestId) return;
    await simulation.refetchQuantiles(requestId);
  };
 
  const handlePercentileClick = async (source: 'sim' | 'diff') => {
    if (!requestId) return;

    const val =
      source === 'sim'
        ? parseFloat(percentileInputSim)
        : parseFloat(percentileInputDiff);

    if (isNaN(val)) return;

    if (!userId) return; // zabezpieczenie
    const result = await getPercentile(userId, requestId, val, source);
    setPercentileMatch({
      sim: source === 'sim' ? result.percentile : percentileMatch?.sim ?? null,
      diff: source === 'diff' ? result.percentile : percentileMatch?.diff ?? null,
    });
  };


  useEffect(() => {
    if (requestId) {
      handleQuantileClick();
    }
  }, [requestId]);

  const handleRunAll = async () => {
    try {
      await simulation.mutateAsync();

      const waitForRequestId = async (): Promise<string> => {
        return new Promise((resolve, reject) => {
          const interval = setInterval(() => {
            const id = useUltimateStore.getState().requestId;
            if (id) {
              clearInterval(interval);
              resolve(id);
            }
          }, 100);

          setTimeout(() => {
            clearInterval(interval);
            reject(new Error("Timeout: requestId not set"));
          }, 5000);
        });
      };

      const id = await waitForRequestId();

      await handleQuantileClick();

      const state = useUltimateStore.getState();
      if (state.percentileInputSim.trim()) await handlePercentileClick('sim');
      if (state.percentileInputDiff.trim()) await handlePercentileClick('diff');
    } catch (err) {
      console.error('❌ Błąd podczas symulacji:', err);
    }
  };

const handleExportToExcel = () => {
  const {
    stats,
    quantileResult,
    percentileMatch,
    percentileInputSim,
    percentileInputDiff,
  } = useUltimateStore.getState();

  if (!stats || !quantileResult) {
    alert('Brak danych do eksportu.');
    return;
  }

  const rows: any[] = [];

  const allKeys = Array.from(
    new Set([...Object.keys(stats), ...Object.keys(quantileResult)])
  );

  for (const key of allKeys) {
    const stat = stats[key];
    const quant = quantileResult[key];

    rows.push({
      Statystyka: key,
      'Wartość (sim_results)': stat?.value ?? quant?.value ?? '',
      Metryka: key,
      'Wartość (sim_diff)': stat?.value_minus_latest ?? quant?.value_minus_latest ?? '',
    });
  }

  // Pusty wiersz
  rows.push({});

  // Percentyle
  rows.push({
    Statystyka: 'Percentyl',
    'Wartość (sim_results)': percentileMatch?.sim != null
      ? `${(percentileMatch.sim * 100).toFixed(2)}%`
      : '',
    Metryka: 'Percentyl',
    'Wartość (sim_diff)': percentileMatch?.diff != null
      ? `${(percentileMatch.diff * 100).toFixed(2)}%`
      : '',
  });

  // Wiersz informujący dla jakiej wartości liczono percentyle
  rows.push({
    Statystyka: 'Dla wartości',
    'Wartość (sim_results)': percentileInputSim || '',
    Metryka: 'Dla wartości',
    'Wartość (sim_diff)': percentileInputDiff || '',
  });

  // 📤 Eksport
  const sheet = XLSX.utils.json_to_sheet(rows);
  const wb = XLSX.utils.book_new();
  XLSX.utils.book_append_sheet(wb, sheet, 'Symulacja');

  const wbout = XLSX.write(wb, { bookType: 'xlsx', type: 'array' });
  const blob = new Blob([wbout], { type: 'application/octet-stream' });
  saveAs(blob, 'symulacja_wyniki.xlsx');
};



  return (
    <div className="flex flex-col gap-6 p-6 text-white">
      <div className="flex gap-6">
        <ControlPanel
          onRun={handleRunAll}
          onQuantileClick={handleQuantileClick}
          onPercentileClick={handlePercentileClick}
          onExport={handleExportToExcel}
          isLoading={simulation.isPending}
        />

        <div className="flex-1 flex flex-col gap-6">
          <Charts.OnlyHistogram />
          <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
            <div className="flex flex-col gap-6">
              <StatsTables.SimResults />
              <StatsTables.SimDiff />
            </div>
            <div className="flex flex-col gap-10 mt-6">
              <div className="h-[350px]">
                <Charts.SimChart />
              </div>
              <div className="h-[350px]">
                <Charts.DiffChart />
              </div>
            </div>
          </div>
        </div>
      </div>

      {simulation.isPending && <OverlayLoaderWithProgress />}
    </div>
  );
}
