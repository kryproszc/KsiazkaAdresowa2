import {
  Tabs,
  TabsContent,
  TabsList,
  TabsTrigger,
} from "@/components/ui/tabs"
import { PaidView } from "./PaidView"
import { CLTab } from "./CLTab"
import { PaidViewMultStoch } from "./PaidViewMultStoch"
import { PaidViewBootParam } from "./PaidViewBootParam"
import WspolczynnikiMultiplikatywna from "./WspolczynnikiMultiplikatywna";
import WspolczynnikiBootParam from "./WspolczynnikiBootParam";
import  {UltimateTab} from "./UltimateTab"
import UltimateTab_boot from './UltimateTab_boot';
import  UltimateTab_stoch from "./UltimateTab_stoch"





export function PaidTabs() {
  return (
    <Tabs defaultValue="triangle" className="w-full">
      <TabsList className="flex flex-grow w-full">
        <TabsTrigger value="triangle">1. Trójkąt</TabsTrigger>
        <TabsTrigger value="cl">2. Reszty</TabsTrigger>
        <TabsTrigger value="ultimate">3. Ultimate</TabsTrigger>
      </TabsList>
      <TabsContent value="triangle">
        <PaidView />
      </TabsContent>
      <TabsContent value="cl">
        <CLTab />
      </TabsContent>
      <TabsContent value="ultimate">
        <UltimateTab />
      </TabsContent>
    </Tabs>
  )
}


export function MultStoch() {
  return (
    <Tabs defaultValue="triangle" className="w-full">
      <TabsList className="flex flex-grow w-full">
        <TabsTrigger value="triangle">1. Trójkąt</TabsTrigger>
        <TabsTrigger value="wspolczynniki_mult">2. Parametry modelu</TabsTrigger>
        <TabsTrigger value="ultimate_mult">3. Ultimate</TabsTrigger>
      </TabsList>
      <TabsContent value="triangle">
        <PaidViewMultStoch />
      </TabsContent>
            <TabsContent value="wspolczynniki_mult">
        <WspolczynnikiMultiplikatywna />
      </TabsContent>
            <TabsContent value="ultimate_mult">
        <UltimateTab_stoch />
      </TabsContent>
    </Tabs>
  )
}

export function BootParam() {
  return (
    <Tabs defaultValue="triangle" className="w-full">
      <TabsList className="flex flex-grow w-full">
        <TabsTrigger value="triangle">1. Trójkąt</TabsTrigger>
        <TabsTrigger value="wspolczynniki_boot">2. Parametry modelu</TabsTrigger>
        <TabsTrigger value="ultimate_boot">2. Ultimate</TabsTrigger>

      </TabsList>
      <TabsContent value="triangle">
        <PaidViewBootParam />
      </TabsContent>
      <TabsContent value="wspolczynniki_boot">
        <WspolczynnikiBootParam />
      </TabsContent>
            <TabsContent value="ultimate_boot">
        <UltimateTab_boot />
      </TabsContent>
      
    </Tabs>
  );
}
