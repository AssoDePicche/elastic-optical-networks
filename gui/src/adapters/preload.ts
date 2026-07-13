import { contextBridge, ipcRenderer } from 'electron';

contextBridge.exposeInMainWorld('api', {
  getContainers: () => ipcRenderer.invoke('containers'),
  onContainersUpdate: (callback: (containers: any[]) => void) => {
    ipcRenderer.on('docker-containers-updated', (_event, value) => callback(value));
  },
});
