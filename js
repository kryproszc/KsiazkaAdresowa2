const API_URL = process.env.NEXT_PUBLIC_API_URL as string;
const res = await fetch(`${API_URL}/calc/full`, { ... });
const res = await fetch(`${API_URL}/calc/quantiles`, { ... });

npm run build
npm run start
