import { FC, ReactElement, useState } from 'react';

import { RootLayout } from '../../layout';

import { Button, Input } from '../../layout/form';

import { Heading } from '../../layout/typography';

interface Edge {
  source: string;
  destination: string;
  cost: number;
}

export const Editor: FC = (): ReactElement => {
  const [nodes, setNodes] = useState<string[]>([]);

  const [node, setNode] = useState<string>('');

  const [edges, setEdges] = useState<Edge[]>([]);

  const [edge, setEdge] = useState<Edge>({
    source: '',
    destination: '',
    cost: 0,
  });

  const addNode = () => {
    setNodes((previous) => [...previous, node]);

    setNode('');
  };

  const addEdge = () => {
    setEdges((previous) => [...previous, edge]);

    setEdge({
      source: '',
      destination: '',
      cost: 0,
    });
  };

  return (
    <RootLayout>
      <Heading>Editor</Heading>

      <div>
        <h3>Node</h3>
        <Input name='nodeId' onChange={(e) => setNode(e.currentTarget.value)} placeholder='id' type='text' value={node} />
        <Button onClick={addNode}>Add</Button>
      </div>

      <ul>
        {nodes.length === 0 && (<div>No nodes</div>)}
        {nodes.map((node: string, index: number) => (<li key={index}>{node}</li>))}
      </ul>

      <div>
        <h3>Edge</h3>
        <select onChange={(e) => setEdge({
          ...edge,
          source: e.currentTarget.value,
        })} value={edge.source}>
          <option value=''>Select a source</option>
          {nodes.map((node: string, index: number) => (<option key={index} value={node}>{node}</option>))}
        </select>

        <select onChange={(e) => setEdge({
          ...edge,
          destination: e.currentTarget.value,
        })} value={edge.destination}>
          <option value=''>Select a destination</option>
          {nodes.map((node: string, index: number) => (<option key={index} value={node}>{node}</option>))}
        </select>

        <Input min='0' name='cost' onChange={(e) => setEdge({
          ...edge,
          cost: e.currentTarget.value,
        })} placeholder='cost' type='number' value={edge.cost} />

        <Button onClick={addEdge}>Add</Button>

        <ul>
          {edges.length === 0 && (<div>No edges</div>)}
          {edges.map((edge: Edge, index: number) => (<li key={index}>{`${edge.source} -> ${edge.destination} (${edge.cost})`}</li>))}
        </ul>
      </div>
    </RootLayout>
  );
}
