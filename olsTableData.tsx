import type { RowData } from '@/types/table';
import {
  useReactTable,
  getCoreRowModel,
  flexRender,
} from '@tanstack/react-table';
import type { ColumnDef, } from '@tanstack/react-table';
import { useMemo } from 'react';

interface Props {
  data: RowData[];
}

export function TableData({ data }: Props) {
  console.log("--------data----")
  console.log(data)
  if (data[0]) {
    console.log(Object.keys(data[0]))
  }
  const columns = useMemo<ColumnDef<RowData>[]>(
    () =>
      data[0]
        ? Object.keys(data[0]).map((key) => ({
          accessorKey: key,
          header: key,
          cell: (info) => info.getValue(),
        }))
        : [],
    [data]
  );

  const table = useReactTable({
    data,
    columns,
    getCoreRowModel: getCoreRowModel(),
  });

  return (
    <div className="overflow-auto dark rounded border">
      <table className="min-w-full table-auto border-collapse">
        <thead className="bg-gray-100 text-left text-sm font-medium text-gray-700 dark:bg-gray-900 dark:text-gray-200">
          {table.getHeaderGroups().map((headerGroup) => (
            <tr key={headerGroup.id}>
              {headerGroup.headers.map((header) => (
                <th key={header.id} className="px-4 py-2  border-b">
                  {header.isPlaceholder
                    ? null
                    : flexRender(header.column.columnDef.header, header.getContext())}
                </th>
              ))}
            </tr>
          ))}
        </thead>
        <tbody className="text-sm text-white">
          {table.getRowModel().rows.map((row) => (
            <tr
              key={row.id}
              className="odd:bg-white even:bg-gray-50 hover:bg-blue-50 dark:odd:bg-gray-800 dark:even:bg-gray-700 dark:hover:bg-blue-900 transition"
            >
              {row.getVisibleCells().map((cell) => (
                <td key={cell.id} className="px-4 py-2 border-b whitespace-nowrap">
                  {flexRender(cell.column.columnDef.cell, cell.getContext())}
                </td>
              ))}
            </tr>
          ))}
        </tbody>
      </table>
    </div>
  );
}
