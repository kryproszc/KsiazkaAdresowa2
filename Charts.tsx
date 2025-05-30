'use client';

import { useUltimateStore } from '@/stores/useUltimateStore';
import {
  BarChart,
  Bar,
  LineChart,
  Line,
  XAxis,
  YAxis,
  Tooltip,
  ResponsiveContainer,
  CartesianGrid,
  Label,
  ReferenceLine, // ✅ dodane
} from 'recharts';

// Histogram
function OnlyHistogram() {
  const { histogramData } = useUltimateStore();

  if (!histogramData.length) return null;

  return (
    <div>
      <h4 className="font-semibold mb-2">Histogram wyników symulacji:</h4>
      <ResponsiveContainer width="100%" height={300}>
        <BarChart data={histogramData}>
          <CartesianGrid strokeDasharray="3 3" />
          <XAxis dataKey="bin" angle={-45} textAnchor="end" height={60} />
          <YAxis />
          <Tooltip />
          <Bar dataKey="count" fill="#8884d8" />
        </BarChart>
      </ResponsiveContainer>
    </div>
  );
}

// SimChart
function SimChart() {
  const { quantileResult, percentileMatch } = useUltimateStore();

  const data = quantileResult
    ? Object.entries(quantileResult).map(([q, val]) => ({
        quantile: parseFloat(q),
        sim: val?.value ?? 0,
      }))
    : [];

  if (!quantileResult) return null;

  return (
    <div>
      <h4 className="font-semibold mb-2">Kwantyle – symulacja (sim_results)</h4>
      <ResponsiveContainer width="100%" height={250}>
        <LineChart data={data}>
          <CartesianGrid strokeDasharray="3 3" />
          <XAxis dataKey="quantile" type="number" domain={[0, 1]}>
            <Label value="Kwantyl" offset={-10} position="insideBottom" />
          </XAxis>
          <YAxis>
            <Label value="Wartość (zł)" angle={-90} position="insideLeft" />
          </YAxis>
          <Tooltip formatter={(value: number) => `${value.toLocaleString('pl-PL')} zł`} />
          <Line type="monotone" dataKey="sim" stroke="#82ca9d" strokeWidth={3} />

          {/* ✅ Pionowa linia percentyla */}
{percentileMatch?.sim != null && (
  <ReferenceLine
    x={percentileMatch.sim}
    stroke="#ffffff" // ✅ lepszy kontrast
    strokeWidth={2}  // ✅ pogrubienie
    label={{
      value: '▲',
      position: 'top',
      fill: '#ffffff',
      fontSize: 16,
    }}
  />
)}

        </LineChart>
      </ResponsiveContainer>
    </div>
  );
}

// DiffChart
function DiffChart() {
  const { quantileResult, percentileMatch } = useUltimateStore();

  const data = quantileResult
    ? Object.entries(quantileResult).map(([q, val]) => ({
        quantile: parseFloat(q),
        diff: val?.value_minus_latest ?? 0,
      }))
    : [];

  if (!quantileResult) return null;

  return (
    <div>
      <h4 className="font-semibold mb-2">Kwantyle – różnice (sim_diff)</h4>
      <ResponsiveContainer width="100%" height={250}>
        <LineChart data={data}>
          <CartesianGrid strokeDasharray="3 3" />
          <XAxis dataKey="quantile" type="number" domain={[0, 1]}>
            <Label value="Kwantyl" offset={-10} position="insideBottom" />
          </XAxis>
          <YAxis>
            <Label value="Wartość (zł)" angle={-90} position="insideLeft" />
          </YAxis>
          <Tooltip formatter={(value: number) => `${value.toLocaleString('pl-PL')} zł`} />
          <Line type="monotone" dataKey="diff" stroke="#8884d8" strokeWidth={3} />

          {/* ✅ Pionowa linia percentyla */}
          {percentileMatch?.diff != null && (
            <ReferenceLine
              x={percentileMatch.diff}
              stroke="#ffffff"
              strokeWidth={2}
              label={{
                value: '▲',
                position: 'top',
                fill: '#ffffff',
                fontSize: 16,
              }}
            />
          )}

        </LineChart>
      </ResponsiveContainer>
    </div>
  );
}

// ✅ Eksport grupowy
export const Charts = {
  OnlyHistogram,
  SimChart,
  DiffChart,
};
