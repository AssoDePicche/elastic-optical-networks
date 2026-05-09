import { createBrowserRouter } from 'react-router-dom';

import { Editor } from './editor';

import { Root } from './root';

export const router = createBrowserRouter([
  {
    path: '/editor',
    element: <Editor />
  },
  {
    path: '/',
    element: <Root />,
  },
]);
