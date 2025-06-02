const API_URL = process.env.NEXT_PUBLIC_API_URL;

const mutation = useMutation({
  mutationFn: async () => {
    const res = await fetch(`${API_URL}/calc/mult_stoch`, {
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
