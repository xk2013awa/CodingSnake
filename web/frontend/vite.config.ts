import { fileURLToPath, URL } from 'node:url'

import vue from '@vitejs/plugin-vue'
import { defineConfig } from 'vite'

const defaultBasePath = '/'

// https://vite.dev/config/
export default defineConfig({
  base: process.env.VITE_BASE_PATH || defaultBasePath,
  plugins: [vue()],
  resolve: {
    alias: {
      '@': fileURLToPath(new URL('./src', import.meta.url)),
    },
  },
})
