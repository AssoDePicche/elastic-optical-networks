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
      '@domain': join(__dirname, 'src/domain'),
      '@ui': join(__dirname, 'src/infrastructure/ui'),
    },
  },
  root: join(__dirname, 'src/infrastructure/ui'),
});
