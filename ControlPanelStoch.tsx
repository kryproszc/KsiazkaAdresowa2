'use client';

import { useStochStore } from '@/stores/useStochStore';
import { Button } from '@/components/ui/button';
import { Loader2 } from 'lucide-react';

type ControlPanelProps = {
  onRun: () => void;
  onQuantileClick: () => void;
  onPercentileClick: (source: 'sim' | 'diff') => void;
  onExport: () => void;
  isLoading: boolean;
};

export function ControlPanelStoch({
  onRun,
  onQuantileClick,
  onPercentileClick,
  onExport,
  isLoading,
}: ControlPanelProps) {
  const {
    rawQuantileInput,
    setRawQuantileInput,
    setQuantileInput,
    percentileInputSim,
    setPercentileInputSim,
    percentileInputDiff,
    setPercentileInputDiff,
    simulationCount,
    setSimulationCount,
    sim_total,
    setSimTotal,
    batch_sim,
    setBatchSim,
    main_seed,
    setMainSeed,
  } = useStochStore();

  return (
    <div className="w-1/5 min-w-[220px] h-fit border border-white/10 rounded p-4 bg-[#1e1e2f] text-white/80 self-start">
      <h4 className="font-semibold mb-4">Panel kontrolny</h4>

      <label className="block mb-2 text-sm">Parametr: sim_total</label>
      <input
        type="number"
        value={Number.isNaN(sim_total) ? '' : sim_total}
        onChange={(e) => {
          const parsed = parseInt(e.target.value);
          setSimTotal(Number.isNaN(parsed) ? 0 : parsed);
        }}
        className="text-white bg-[#1e1e2f] border border-white/10 p-2 rounded w-full mb-4"
      />

      <label className="block mb-2 text-sm">Parametr: batch_sim</label>
      <input
        type="number"
        value={Number.isNaN(batch_sim) ? '' : batch_sim}
        onChange={(e) => {
          const parsed = parseInt(e.target.value);
          setBatchSim(Number.isNaN(parsed) ? 0 : parsed);
        }}
        className="text-white bg-[#1e1e2f] border border-white/10 p-2 rounded w-full mb-4"
      />

      <label className="block mb-2 text-sm">Parametr: main_seed</label>
      <input
        type="number"
        value={Number.isNaN(main_seed) ? '' : main_seed}
        onChange={(e) => {
          const parsed = parseInt(e.target.value);
          setMainSeed(Number.isNaN(parsed) ? 0 : parsed);
        }}
        className="text-white bg-[#1e1e2f] border border-white/10 p-2 rounded w-full mb-4"
      />

      <Button onClick={onRun} className="w-full mb-3" disabled={isLoading}>
        {isLoading ? (
          <div className="flex items-center justify-center gap-2">
            <Loader2 className="animate-spin w-4 h-4" />
            <span>Symuluję...</span>
          </div>
        ) : (
          'Zatwierdź wszystko'
        )}
      </Button>

      <label className="block mb-2 text-sm">Kwantyle:</label>
      <input
        type="text"
        value={rawQuantileInput}
        onChange={(e) => {
          const raw = e.target.value;
          setRawQuantileInput(raw);
          const cleaned = raw
            .split(',')
            .map((q) => parseFloat(q.trim()))
            .filter((q) => !isNaN(q) && q >= 0 && q <= 1);
          const uniqueSorted = Array.from(new Set(cleaned)).sort((a, b) => a - b);
          setQuantileInput(uniqueSorted.join(','));
        }}
        disabled={isLoading}
        className="text-white bg-[#1e1e2f] border border-white/10 placeholder-white/50 p-2 rounded w-full"
      />

      <Button onClick={onQuantileClick} className="w-full mt-2 mb-4" disabled={isLoading}>
        Wyświetl kwantyle
      </Button>

      <label className="block text-sm">Wartość dla symulacji (sim_results):</label>
      <input
        type="number"
        value={percentileInputSim || ''}
        onChange={(e) => setPercentileInputSim(e.target.value)}
        className="text-white bg-[#1e1e2f] border border-white/10 p-2 rounded w-full mb-2"
        disabled={isLoading}
      />
      <Button onClick={() => onPercentileClick('sim')} className="w-full mb-4" disabled={isLoading}>
        Przelicz percentyl (sim)
      </Button>

      <label className="block text-sm">Wartość dla różnic (sim_diff):</label>
      <input
        type="number"
        value={percentileInputDiff || ''}
        onChange={(e) => setPercentileInputDiff(e.target.value)}
        className="text-white bg-[#1e1e2f] border border-white/10 p-2 rounded w-full mb-2"
        disabled={isLoading}
      />
      <Button onClick={() => onPercentileClick('diff')} className="w-full mb-4" disabled={isLoading}>
        Przelicz percentyl (diff)
      </Button>

      <Button onClick={onExport} className="w-full" disabled={isLoading}>
        Eksport do Excela
      </Button>
    </div>
  );
}
