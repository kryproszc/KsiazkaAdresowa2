const API_URL = process.env.NEXT_PUBLIC_API_URL;

const mutation = useMutation({
  mutationFn: async () => {
    const res = await fetch(`${API_URL}/calc/wspolczynniki_boot`, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({
        user_id: userId,
        wagi_boot: selectedCells,
        paid_data: sheetJSON,
      }),
    });

    if (!res.ok) throw new Error('Błąd backendu');
    return res.json();
  },
  onSuccess: (data) => {
    console.log('✅ BootParam OK', data);
  },
  onError: (err) => console.error('❌ BootParam error:', err),
});
