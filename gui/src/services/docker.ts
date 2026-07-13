import Docker from 'dockerode';

export interface DockerContainer {
  id: string;
  image: string;
  names: string[];
  state: string;
}

export interface DockerService {
  listContainers: () => Promise<DockerContainer[]>;
}

export class DockerServiceImpl implements DockerService {
  constructor(socketPath: string) {
    this.docker = new Docker({ socketPath: socketPath });

    this.docker.getEvents((err, stream) => {
      console.log(err)
      console.log(stream)
      if (err) {
        return;
      }

      stream.on('data', (chunk) => {
        console.log(chunk);
      });
    });
  }

  async listContainers(): Promise<DockerContainer[]> {
    try {
      return await docker.listContainers({ all: true });
    } catch (_) {
      return [];
    }
  }
}
