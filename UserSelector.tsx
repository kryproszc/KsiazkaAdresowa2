'use client';

import { useState } from 'react';
import { useUserStore } from '@/app/_components/useUserStore';
import { Input } from '@/components/ui/input';
import { Button } from '@/components/ui/button';
import { UserIcon } from 'lucide-react';

export function UserSelector() {
  const setUserId = useUserStore((s) => s.setUserId);
  const [input, setInput] = useState('');

  const handleLogin = () => {
    if (input.trim()) setUserId(input.trim());
  };

  return (
    <div className="min-h-screen flex items-center justify-center bg-background px-4">
      <div className="w-full max-w-sm bg-white dark:bg-[#1e1e2f] border border-gray-300 dark:border-white/10 rounded-2xl shadow-lg p-8 space-y-6">
        <div className="flex flex-col items-center">
          <div className="bg-slate-800 text-white p-3 rounded-full mb-4">
            <UserIcon className="h-6 w-6" />
          </div>
          <h2 className="text-xl font-semibold text-gray-900 dark:text-white">Zaloguj się</h2>
          <p className="text-sm text-gray-500 dark:text-gray-400">
            Wprowadź swoją nazwę użytkownika
          </p>
        </div>

        <div className="space-y-4">
          <Input
            placeholder="np. user123"
            value={input}
            onChange={(e) => setInput(e.target.value)}
            className="w-full"
          />
          <Button onClick={handleLogin} className="w-full">
            Zaloguj
          </Button>
        </div>
      </div>
    </div>
  );
}
