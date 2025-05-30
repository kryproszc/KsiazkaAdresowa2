import {
  Select,
  SelectContent,
  SelectItem,
  SelectTrigger,
  SelectValue,
} from "@/components/ui/select";
import { useTableStore } from "@/stores/tableStore";

export function SheetSelect() {
  const sheetNames = useTableStore((state) => state.getSheetNames());
  const selectedSheetName = useTableStore((state) => state.selectedSheetName);
  const setSelectedSheetName = useTableStore((state) => state.setSelectedSheetName);

  return <div>
    <Select onValueChange={(v) => {
      if (!v) return
      setSelectedSheetName(v)
    }} value={selectedSheetName ?? ""}>
      <SelectTrigger className="w-[200px]">
        <SelectValue placeholder="Wybierz arkusz" />
      </SelectTrigger>
      <SelectContent>
        {sheetNames?.map((name) => (
          <SelectItem key={name} value={name}>
            {name}
          </SelectItem>
        ))}
      </SelectContent>
    </Select>
  </div>
}
