import { join } from 'path';

import { defineConfig } from 'vite';

import react from '@vitejs/plugin-react';

import tailwindcss from '@tailwindcss/vite';

export default defineConfig({
  base: './',
  plugins: [
    react(),
    tailwindcss()
  ],
  resolve: {
    alias: {
      '@ui': join(__dirname, 'src/ui'),
    },
  },
  root: join(__dirname, 'src/ui'),
});
