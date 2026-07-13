import { FC, ReactNode } from 'react';

import { styled } from 'styled-components';

import { Theme } from '../theme';

export const Heading = styled.h1`
  colors: ${Theme.colors.fontColor};
  font-size: ${Theme.fontSize.large}rem;
  font-weight: bold;
`;
