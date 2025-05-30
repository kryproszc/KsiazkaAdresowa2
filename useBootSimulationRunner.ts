'use client';

import { useMutation } from '@tanstack/react-query';
import { getQuantilesStoch, runBootSimulation } from '@/services/simulationApi';
import { useTableStore } from '@/stores/tableStore';
import { useUserStore } from '@/app/_components/useUserStore';
import { useBootStore } from '@/stores/useBootStore';
import { useBootParamStore } from '@/stores/bootParamStore'; // ✅ DODAJ TO

export function useBootSimulationRunner() {
  const userId = useUserStore((s) => s.userId);

  const {
    setRequestId,
    setHistogramData,
    setQuantileResult,
    setStats,
    quantileInput,
    dev,
    sd,
    sigma,
    sim_total,
    batch_sim,
    main_seed,
  } = useBootStore();

  const { selectedSheetJSON } = useTableStore();
  const { selectedCells } = useBootParamStore(); // ✅ DODAJ TO

  const simulation = useMutation({
    mutationFn: async () => {
      if (!userId) throw new Error('Brak userId');
      if (!dev?.length || !sd?.length || !sigma?.length) {
        throw new Error('Brakuje wartości dev, sd lub sigma');
      }

      const res = await runBootSimulation({
        user_id: userId,
        dev,
        sd,
        sigma,
        triangle: selectedSheetJSON ?? [],
        wagi_boot: selectedCells ?? [], // ✅ UŻYJ WAG
        sim_total,
        batch_sim,
        main_seed,
        ultimate_param_resrisk: 0,
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

      const quantileData = await getQuantilesStoch(userId, res.request_id, quantiles);
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

    const data = await getQuantilesStoch(userId, requestId, quantiles);
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
