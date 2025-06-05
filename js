@app.post("/calc/cl")
async def calc_cl(
    data: List[List[Union[str, int, float, None]]],
    selected: List[List[int]],
):
    print("selected")
    print(selected)
    df = pd.DataFrame(data).iloc[1:, 1:]
    df = df.apply(pd.to_numeric, errors="coerce")
    tri = TriangleCalculator.full_ci_calculation(df)[0]
    print("tri")
    print(pd.DataFrame(tri))
    safe_tri = tri.replace([np.inf, -np.inf], np.nan).astype(object).where(pd.notnull(tri), None)

    matrix = [[tri.columns.name or "AY"] + list(tri.columns)]
    for idx in tri.index:
        values = list(safe_tri.loc[idx])
        while values and values[-1] is None:
            values.pop()
        row = [idx] + values
        matrix.append(row)
    print("matrix")
    print(pd.DataFrame(matrix))
    return {"data": matrix}
