'use client';

import { useUltimateStore } from '@/stores/useUltimateStore';
import {
  ResponsiveContainer,
  BarChart,
  Bar,
  CartesianGrid,
  XAxis,
  YAxis,
  Tooltip,
} from 'recharts';

export function Histogram() {
  const { histogramData } = useUltimateStore();

  if (histogramData.length === 0) return null;

  return (
    <div className="mt-2">
      <h4 className="font-semibold mb-2">Histogram wyników symulacji:</h4>
        <ResponsiveContainer width="100%" height={10000}>
          <BarChart data={histogramData}>
            <CartesianGrid strokeDasharray="3 3" />
            <XAxis
              dataKey="bin"
              angle={-45}
              textAnchor="end"
              height={90}
              interval={0}
              tick={{ fontSize: 11 }}
              tickFormatter={(value) => {
                const parsed = parseFloat(value.split(' ')[0].replace(/[^0-9.-]/g, ''));
                if (isNaN(parsed)) return value;
                return `${(parsed / 1000).toFixed(0)}k`;
              }}
            />
            <YAxis />
            <Tooltip />
            <Bar dataKey="count" fill="#8884d8" />
          </BarChart>
        </ResponsiveContainer>

    </div>
  );
}
