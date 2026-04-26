import Docker from 'dockerode';

import { app, BrowserWindow, ipcMain, Menu } from 'electron';

import path from 'node:path';

import started from 'electron-squirrel-startup';

if (started) {
  app.quit();
}

const docker = new Docker({ socketPath: '/var/run/docker.sock' });

const refreshContainers = async (window: BrowserWindow) => {
  const containers = await docker.listContainers({ all: true });

  window.webContents.send('docker-containers-updated', containers);
};

docker.getEvents((err, stream) => {
  if (err) {
    return;
  }

  stream.on('data', (chunk) => {
    const event = JSON.parse(chunk.toString());

    if (event.type === 'container') {
      const mainWindow = BrowserWindow.getAllWindows()[0];

      if (mainWindow) {
        refreshContainers(mainWindow);
      }
    }
  });
});

ipcMain.handle('containers', async () => {
  try {
    const containers = await docker.listContainers({ all: true });

    return containers;
  } catch (error) {
    console.error(error);

    return [];
  }
});

const createWindow = () => {
  Menu.setApplicationMenu(null);

  const mainWindow = new BrowserWindow({
    width: 800,
    height: 600,
    webPreferences: {
      contextIsolation: true,
      nodeIntegration: true,
      preload: path.join(__dirname, 'preload.js'),
    },
  });

  if (MAIN_WINDOW_VITE_DEV_SERVER_URL) {
    mainWindow.loadURL(MAIN_WINDOW_VITE_DEV_SERVER_URL);
  } else {
    mainWindow.loadFile(
      path.join(__dirname, `../renderer/${MAIN_WINDOW_VITE_NAME}/index.html`),
    );
  }

  mainWindow.webContents.openDevTools();
};

app.on('ready', createWindow);

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') {
    app.quit();
  }
});

app.on('activate', () => {
  if (BrowserWindow.getAllWindows().length === 0) {
    createWindow();
  }
});
