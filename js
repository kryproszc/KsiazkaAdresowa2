@app.post("/calc/mult_stoch")
async def calc_mult_stoch(payload: MatrixRequest):
    data = {}
    headers = payload.paid_data[0][1:]

    for i, col in enumerate(headers):
        data[col] = []
        for row in payload.paid_data[1:]:
            value = row[i + 1]
            try:
                num_value = float(value)
            except (ValueError, TypeError):
                num_value = None
            data[col].append(num_value)

    train_devide = TriangleCalculator.elementwise_division(data)
    train_devide_serializable = np.array(train_devide).tolist()
    return {
        "train_devide": train_devide_serializable
    }
