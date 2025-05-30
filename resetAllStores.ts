// src/lib/resetAllStores.ts

import { useTableStore } from "@/stores/tableStore";
import { useUltimateStore } from "@/stores/useUltimateStore";
import { useBootStore } from "@/stores/useBootStore";
import { useStochStore } from "@/stores/useStochStore";
import { useMultStochStore } from "@/stores/multStochStore";
import { useTrainDevideStore } from '@/stores/trainDevideStore';
import { useStochResultsStore } from "@/stores/useStochResultsStore";

// ✅ nowe importy
import { useBootParamStore } from "@/stores/bootParamStore";
import { useBootParamResultsStore } from "@/stores/useBootParamResultsStore";
import { useBootResultsStore } from "@/stores/useBootResultsStore";

export function resetAllStores() {
  useTableStore.getState().resetData();
  useUltimateStore.getState().reset();
  useTrainDevideStore.getState().reset();
  useStochResultsStore.getState().reset();

  // ✅ reset store'ów związanych z zakładką BootParam
  useBootParamStore.getState().reset();
  useBootParamResultsStore.getState().reset();
  useBootResultsStore.getState().reset();
  useBootStore.getState().reset();
}
