'use client';

import { useEffect } from 'react';
import * as XLSX from 'xlsx';
import { saveAs } from 'file-saver';

import { ControlPanelBoot } from '@/components/Simulation/ControlPanelBoot';
import { ChartsBoot } from '@/components/Simulation/ChartsBoot';
import { StatsTablesBoot } from '@/components/Simulation/StatsTablesBoot';
import { getPercentileStoch } from '@/services/simulationApi';
import { useBootStore } from '@/stores/useBootStore';
import { OverlayLoaderWithProgress } from '@/components/ui/OverlayLoaderWithProgress';
import { useUserStore } from '@/app/_components/useUserStore';
import { useBootSimulationRunner } from '@/hooks/useBootSimulationRunner';
import { useBootParamResultsStore } from '@/stores/useBootParamResultsStore'; // ✅ Dodane

export default function UltimateTab_boot() {
  const userId = useUserStore((s) => s.userId);
  const simulation = useBootSimulationRunner();

  const {
    percentileInputSim,
    percentileInputDiff,
    percentileMatch,
    setPercentileMatch,
    requestId,
    quantileResult,
    stats,
  } = useBootStore();

  const handleQuantileClick = async () => {
    if (!requestId) return;
    await simulation.refetchQuantiles(requestId);
  };

  useEffect(() => {
    if (requestId) {
      handleQuantileClick();
    }
  }, [requestId]);

  useEffect(() => {
    const runPercentileSync = async () => {
      if (!requestId || !userId) return;

      const { percentileInputSim, percentileInputDiff } = useBootStore.getState();
      const updates: { sim: number | null; diff: number | null } = {
        sim: null,
        diff: null,
      };

      if (percentileInputSim.trim()) {
        const simVal = parseFloat(percentileInputSim);
        if (!isNaN(simVal)) {
          const simRes = await getPercentileStoch(userId, requestId, simVal, 'sim');
          updates.sim = simRes.percentile;
        }
      }

      if (percentileInputDiff.trim()) {
        const diffVal = parseFloat(percentileInputDiff);
        if (!isNaN(diffVal)) {
          const diffRes = await getPercentileStoch(userId, requestId, diffVal, 'diff');
          updates.diff = diffRes.percentile;
        }
      }

      useBootStore.getState().setPercentileMatch(updates);
    };

    runPercentileSync();
  }, [requestId, userId]);

  const handlePercentileClick = async (source: 'sim' | 'diff') => {
    if (!requestId || !userId) return;

    const val =
      source === 'sim'
        ? parseFloat(percentileInputSim)
        : parseFloat(percentileInputDiff);

    if (isNaN(val)) return;

    const result = await getPercentileStoch(userId, requestId, val, source);
    setPercentileMatch({
      sim: source === 'sim' ? result.percentile : percentileMatch?.sim ?? null,
      diff: source === 'diff' ? result.percentile : percentileMatch?.diff ?? null,
    });
  };

  const handleRunAll = async () => {
    try {
      if (!userId) throw new Error('Brak userId');

      // ✅ Pobranie dev/sd/sigma z bootParamResultsStore i zapisanie do bootStore
      const { dev, sd, sigma } = useBootParamResultsStore.getState();
      const { setDev, setSd, setSigma } = useBootStore.getState();
      setDev(dev || []);
      setSd(sd || []);
      setSigma(sigma || []);

      await simulation.mutateAsync();

      const waitForRequestId = async (): Promise<string> => {
        return new Promise((resolve, reject) => {
          const interval = setInterval(() => {
            const id = useBootStore.getState().requestId;
            if (id) {
              clearInterval(interval);
              resolve(id);
            }
          }, 100);

          setTimeout(() => {
            clearInterval(interval);
            reject(new Error('Timeout: requestId not set'));
          }, 5000);
        });
      };

      const id = await waitForRequestId();
      await simulation.refetchQuantiles(id);
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
    } = useBootStore.getState();

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

    rows.push({});
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

    rows.push({
      Statystyka: 'Dla wartości',
      'Wartość (sim_results)': percentileInputSim || '',
      Metryka: 'Dla wartości',
      'Wartość (sim_diff)': percentileInputDiff || '',
    });

    const sheet = XLSX.utils.json_to_sheet(rows);
    const wb = XLSX.utils.book_new();
    XLSX.utils.book_append_sheet(wb, sheet, 'Symulacja');

    const wbout = XLSX.write(wb, { bookType: 'xlsx', type: 'array' });
    const blob = new Blob([wbout], { type: 'application/octet-stream' });
    saveAs(blob, 'symulacja_boot.xlsx');
  };

  return (
    <div className="flex flex-col gap-6 p-6 text-white">
      <div className="flex gap-6">
        <ControlPanelBoot
          onRun={handleRunAll}
          onQuantileClick={handleQuantileClick}
          onPercentileClick={handlePercentileClick}
          onExport={handleExportToExcel}
          isLoading={simulation.isPending}
        />

        <div className="flex-1 flex flex-col gap-6">
          <ChartsBoot.OnlyHistogram />
          <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
            <div className="flex flex-col gap-6">
              <StatsTablesBoot.SimResults />
              <StatsTablesBoot.SimDiff />
            </div>
            <div className="flex flex-col gap-10 mt-6">
              <div className="h-[350px]">
                <ChartsBoot.SimChart />
              </div>
              <div className="h-[350px]">
                <ChartsBoot.DiffChart />
              </div>
            </div>
          </div>
        </div>
      </div>

      {simulation.isPending && <OverlayLoaderWithProgress />}
    </div>
  );
}
