import { FC, ReactElement, useEffect, useState } from 'react';

import { type DockerContainer } from '@/services/docker';

import { RootLayout } from '../../layout';

import { Heading } from '../../layout/typography';

export const Root: FC = (): ReactElement => {
  const [containers, setContainers] = useState<DockerContainer[]>([]);

  useEffect(() => {
    window.api.getContainers().then(setContainers);

    window.api.onContainersUpdate((updatedContainers) => {
      setContainers(updatedContainers);
    });
  }, []);

  return (
    <RootLayout>
      <Heading>Root</Heading>

      {containers.length === 0 && (
        <div>No containers</div>
      )}

      {containers.map((container: DockerContainer) => (
        <div key={container.Id}>{...container.names} - {container.state}</div>
      ))}
    </RootLayout>
  );
};
