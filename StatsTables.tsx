'use client';

import { useUltimateStore } from '@/stores/useUltimateStore';

const tableBox = "w-full rounded-xl bg-[#101624] border border-white/10 p-4 shadow-sm";
const tableStyle = "w-full text-sm text-white/80 border-separate border-spacing-y-1";
const cell = "px-3 py-1 text-center bg-[#1e2235] rounded-md";
const headerCell = "px-3 py-1 text-center font-semibold text-white bg-[#1f2a3a] rounded-md";

function SimResults() {
  const { stats, quantileResult, percentileMatch, percentileInputSim } = useUltimateStore();
  if (!quantileResult) return null;

  return (
    <div className={tableBox}>
      <h4 className="font-semibold mb-3">Statystyki i kwantyle: Ultimate</h4>
      <div className="flex flex-col lg:flex-row gap-6">
        <table className={tableStyle}>
          <thead>
            <tr>
              <th className={headerCell}>Statystyka</th>
              <th className={headerCell}>Wartość</th>
            </tr>
          </thead>
          <tbody>
            {stats &&
              Object.entries(stats).map(([key, val]) => (
                <tr key={`sim-stat-${key}`}>
                  <td className={cell}>{key}</td>
                  <td className={cell}>{val?.value?.toLocaleString('pl-PL') ?? '—'}</td>
                </tr>
              ))}
            {Object.entries(quantileResult).map(([key, val]) => (
              <tr key={`sim-q-${key}`}>
                <td className={cell}>{key}</td>
                <td className={cell}>{val?.value?.toLocaleString('pl-PL') ?? '—'}</td>
              </tr>
            ))}
          </tbody>
        </table>

        {percentileMatch?.sim != null && (
          <table className={`${tableStyle} lg:w-1/3`}>
            <caption className="font-semibold text-left mb-2 text-white">
              Percentyl dla: {percentileInputSim}
            </caption>
            <thead>
              <tr>
                <th className={headerCell}>Statystyka</th>
                <th className={headerCell}>Percentyl</th>
              </tr>
            </thead>
            <tbody>
              <tr>
                <td className={cell}>sim_results</td>
                <td className={cell}>{(percentileMatch.sim * 100).toFixed(2)}%</td>
              </tr>
            </tbody>
          </table>
        )}
      </div>
    </div>
  );
}

function SimDiff() {
  const { stats, quantileResult, percentileMatch, percentileInputDiff } = useUltimateStore();
  if (!quantileResult) return null;

  return (
    <div className={tableBox}>
      <h4 className="font-semibold mb-3">Statystyki i kwantyle: Ultimate - przekątna</h4>
      <div className="flex flex-col lg:flex-row gap-6">
        <table className={tableStyle}>
          <thead>
            <tr>
              <th className={headerCell}>Metryka</th>
              <th className={headerCell}>Wartość</th>
            </tr>
          </thead>
          <tbody>
            {stats &&
              Object.entries(stats).map(([key, val]) => (
                <tr key={`diff-stat-${key}`}>
                  <td className={cell}>{key}</td>
                  <td className={cell}>{val?.value_minus_latest?.toLocaleString('pl-PL') ?? '—'}</td>
                </tr>
              ))}
            {Object.entries(quantileResult).map(([key, val]) => (
              <tr key={`diff-q-${key}`}>
                <td className={cell}>{key}</td>
                <td className={cell}>{val?.value_minus_latest?.toLocaleString('pl-PL') ?? '—'}</td>
              </tr>
            ))}
          </tbody>
        </table>

        {percentileMatch?.diff != null && (
          <table className={`${tableStyle} lg:w-1/3`}>
            <caption className="font-semibold text-left mb-2 text-white">
              Percentyl dla: {percentileInputDiff}
            </caption>
            <thead>
              <tr>
                <th className={headerCell}>Źródło</th>
                <th className={headerCell}>Percentyl</th>
              </tr>
            </thead>
            <tbody>
              <tr>
                <td className={cell}>sim_diff</td>
                <td className={cell}>{(percentileMatch.diff * 100).toFixed(2)}%</td>
              </tr>
            </tbody>
          </table>
        )}
      </div>
    </div>
  );
}

export const StatsTables = {
  SimResults,
  SimDiff,
};
