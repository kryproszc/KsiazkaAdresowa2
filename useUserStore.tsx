import { create } from 'zustand';
import { persist, createJSONStorage } from 'zustand/middleware';

type UserStore = {
  userId: string | null;
  setUserId: (id: string | null) => void;
};

export const useUserStore = create<UserStore>()(
  persist(
    (set) => ({
      userId: null,
      setUserId: (id) => set({ userId: id }),
    }),
    {
      name: 'user-storage',
      storage: createJSONStorage(() => sessionStorage), // ← Zmień na localStorage jeśli chcesz pamiętać po zamknięciu
    }
  )
);
