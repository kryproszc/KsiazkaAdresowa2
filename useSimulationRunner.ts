'use client';

import { useMutation } from '@tanstack/react-query';
import { runFullSimulation, getQuantiles } from '@/services/simulationApi';
import { useUltimateStore } from '@/stores/useUltimateStore';
import { useTableStore } from '@/stores/tableStore';
import { useUserStore } from '@/app/_components/useUserStore';

export function useSimulationRunner() {
  const userId = useUserStore((s) => s.userId);

  const {
    setRequestId,
    setHistogramData,
    setQuantileResult,
    setStats,
    quantileInput,
    simulationCount,
  } = useUltimateStore();

  const {
    clData,
    clWeights,
    selectedSheetJSON,
    selectedCells,
  } = useTableStore();

  const simulation = useMutation({
    mutationFn: async () => {
      if (!userId) throw new Error('Brak userId');

      const res = await runFullSimulation({
        cl_data: clData,
        cl_weights: clWeights,
        paid_data: selectedSheetJSON,
        paid_weights: selectedCells,
        triangle_raw: selectedSheetJSON,
        cl_weights_raw: clWeights,
        user_id: userId,
        nbr_samples: parseInt(simulationCount) || 1000,
      });

      setRequestId(res.request_id);

      const bins = res.histogram?.bins ?? [];
      const counts = res.histogram?.counts ?? [];
      const formattedHistogram = bins.slice(0, -1).map((start: number, i: number) => ({
        bin: `${start.toLocaleString('pl-PL')} - ${bins[i + 1].toLocaleString('pl-PL')}`,
        count: counts[i],
      }));

      setHistogramData(formattedHistogram);

      const quantiles = quantileInput
        .split(',')
        .map((q) => parseFloat(q.trim()))
        .filter((q) => !isNaN(q) && q >= 0 && q <= 1);

      const quantileData = await getQuantiles(userId, res.request_id, quantiles);
      setQuantileResult(quantileData.quantiles);
      setStats(quantileData.stats ?? null);
    },
  });

  const refetchQuantiles = async (requestId: string) => {
    if (!userId) throw new Error('Brak userId');

    const quantiles = quantileInput
      .split(',')
      .map((q) => parseFloat(q.trim()))
      .filter((q) => !isNaN(q) && q >= 0 && q <= 1);

    const data = await getQuantiles(userId, requestId, quantiles);
    setQuantileResult(data.quantiles);
    setStats(data.stats ?? null);
  };

  return {
    mutate: simulation.mutate,
    mutateAsync: simulation.mutateAsync,
    isPending: simulation.isPending,
    refetchQuantiles,
  };
}
