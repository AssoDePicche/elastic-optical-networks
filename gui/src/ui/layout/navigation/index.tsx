import { FC, ReactNode } from 'react';

import { styled } from 'styled-components';

import { Theme } from '../theme';

const Nav = styled.nav`
  align-items: center;
  background-color: ${Theme.colors.primary};
  display: flex;
  height: 46px;
  justify-content: space-between;
  padding: 0 1rem;
`;

const Ul = styled.ul`
  align-items: center;
  display: flex;
  gap: 1rem;
`;

const Li = styled.li`
  color: ${Theme.colors.fontColor};
  font-size: ${Theme.fontSize.base}rem;
`;

const AppName = styled.span`
  color: ${Theme.colors.fontColor};
  font-size: ${Theme.fontSize.base}rem;
`;

interface Path {
  name: string;
  path: string;
}

export const Navigation: FC = (): ReactNode => {
  const paths: Path[] = [
    { name: 'Dashboard', path: '/' },
    { name: 'Editor', path: '/editor' },
  ];

  return (
    <Nav>
      <AppName>App</AppName>
      <Ul>
        {paths.map((path: Path, index: number) => (<Li key={index}><a href={path.path}>{path.name}</a></Li>))}
      </Ul>
    </Nav>
  );
};
