'use client';

import { useUserStore } from '@/app/_components/useUserStore';
import { HomeTabs } from '@/app/_components/HomeTabs';
import { UserSelector } from '@/app/_components/UserSelector';

export default function Page() {
  const userId = useUserStore((s) => s.userId);

  return userId ? <HomeTabs /> : <UserSelector />;
}
