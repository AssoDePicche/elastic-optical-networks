import { FC, ReactNode } from 'react';

import { styled } from 'styled-components';

import { Navigation } from './navigation';

import { Theme } from './theme';

const Main = styled.main`
  background-color: ${Theme.colors.primary};
  color: ${Theme.colors.fontColor};
  font-size: ${Theme.fontSize.base}rem;
  height: 100%;
  padding: 0 1rem;
`;

interface Properties {
  children: ReactNode;
}

export const RootLayout: FC<Properties> = ({ children }): ReactNode => {
  return (
    <>
      <Navigation />

      <Main>{children}</Main>
    </>
  );
};
