export const client = createClient(createConfig<ClientOptions>({
    baseUrl: process.env.NEXT_PUBLIC_API_URL as string || 'http://localhost:8000'
}));
