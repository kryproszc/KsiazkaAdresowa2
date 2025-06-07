import { useState } from "react";
import { InputDataTab } from "./InputDataTab";
import { DeterministicTabs, MultiplikatywnyPaid, BootParametryczny } from "./DeterministicTabs";
import { 
  Folder, 
  Calculator, 
  Dice5, 
  LineChart 
} from "lucide-react"; // Ikonki

export function HomeTabs() {
  const [selectedTab, setSelectedTab] = useState<"input" | "deterministic" | "mult_stoch" | "boot_param">("input");

  const renderContent = () => {
    switch (selectedTab) {
      case "input":
        return <InputDataTab />;
      case "deterministic":
        return <DeterministicTabs />;
      case "mult_stoch":
        return <MultiplikatywnyPaid />;
      case "boot_param":
        return <BootParametryczny />;
      default:
        return null;
    }
  };

  return (
    <div className="flex h-full min-h-screen">
      {/* Sidebar */}
      <aside className="w-64 bg-gray-900 text-gray-100 flex flex-col py-6 px-4 border-r border-gray-800">
        <h2 className="text-2xl font-bold mb-8 text-center">Menu</h2>
        <nav className="flex flex-col gap-4">
          <SidebarItem 
            icon={<Folder size={20} />} 
            label="Wprowadź dane" 
            isActive={selectedTab === "input"} 
            onClick={() => setSelectedTab("input")} 
          />
          <SidebarItem 
            icon={<Calculator size={20} />} 
            label="Metody deterministyczne" 
            isActive={selectedTab === "deterministic"} 
            onClick={() => setSelectedTab("deterministic")} 
          />
          <SidebarItem 
            icon={<Dice5 size={20} />} 
            label="Multiplikatywna Stochastyczna" 
            isActive={selectedTab === "mult_stoch"} 
            onClick={() => setSelectedTab("mult_stoch")} 
          />
          <SidebarItem 
            icon={<LineChart size={20} />} 
            label="Bootstrap parametryczny" 
            isActive={selectedTab === "boot_param"} 
            onClick={() => setSelectedTab("boot_param")} 
          />
        </nav>
      </aside>

      {/* Main Content */}
      <main className="flex-1 bg-gray-100 p-6 overflow-auto">
        {renderContent()}
      </main>
    </div>
  );
}

// SidebarItem as a small subcomponent
function SidebarItem({ icon, label, isActive, onClick }: { 
  icon: React.ReactNode;
  label: string;
  isActive: boolean;
  onClick: () => void;
}) {
  return (
    <button
      onClick={onClick}
      className={`flex items-center gap-3 p-3 rounded-lg transition-colors 
        ${isActive ? "bg-gray-800 text-blue-400" : "hover:bg-gray-800"}`}
    >
      {icon}
      <span className="text-lg">{label}</span>
    </button>
  );
}
