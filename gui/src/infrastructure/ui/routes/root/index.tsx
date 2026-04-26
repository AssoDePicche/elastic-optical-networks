import { FC, ReactElement, useEffect, useState } from 'react';

interface DockerContainer {
  Id: string;
  Names: string[];
  Image: string;
  State: string;
}

export const Root: FC = (): ReactElement => {
  const [containers, setContainers] = useState<DockerContainer[]>([]);

  useEffect(() => {
    window.electronAPI.getContainers().then(setContainers);

    window.electronAPI.onContainersUpdate((updatedContainers) => {
      setContainers(updatedContainers);
    });
  }, []);

  return (
    <div>
      <h1>Root</h1>

      {containers.map((container: DockerContainer) => (
        <div key={container.Id}>{...container.Names} - {container.State}</div>
      ))}
    </div>
  );
};
