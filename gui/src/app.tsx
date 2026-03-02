import { createRoot } from 'react-dom/client';

const App = () => {
  return (
    <div>
      <h1 className="uppercase">Hello, World!</h1>
    </div>
  );
};

const root = createRoot(document.body);

root.render(
  <>
    <App/>
  </>
);
