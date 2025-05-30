import {
  Tabs,
  TabsContent,
  TabsList,
  TabsTrigger,
} from "@/components/ui/tabs"
import { InputDataTab } from "./InputDataTab"
import { DeterministicTabs,MultiplikatywnyPaid,BootParametryczny } from "./DeterministicTabs"

export function HomeTabs() {
  return (
    <Tabs defaultValue="input" className="w-full h-full">
      <TabsList className="grid w-full grid-cols-4">
        <TabsTrigger value="input">Wprowadź dane</TabsTrigger>
        <TabsTrigger value="deterministic">Metody deterministyczne</TabsTrigger>
        <TabsTrigger value="mult_stoch">Multiplikatywna Stochastyczna</TabsTrigger>
        <TabsTrigger value="boot_param">Bootstrap parametryczny</TabsTrigger>
      </TabsList>
      <TabsContent value="input">
        <InputDataTab />
      </TabsContent>
      <TabsContent value="deterministic">
        <DeterministicTabs />
      </TabsContent>
      <TabsContent value="mult_stoch">
        <MultiplikatywnyPaid />
      </TabsContent>
      <TabsContent value="boot_param">
        <BootParametryczny />
      </TabsContent>
    </Tabs>
  )
}
