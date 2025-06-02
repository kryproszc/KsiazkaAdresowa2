mutationFn: async () => {
  const res = await fetch(`${process.env.NEXT_PUBLIC_API_URL}/calc/mult_stoch`, {
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
}
