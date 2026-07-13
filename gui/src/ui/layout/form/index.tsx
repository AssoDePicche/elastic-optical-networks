import { FC, ReactNode } from 'react';

import { styled } from 'styled-components';

import { Theme } from '../theme';

export const Button = styled.button`
  border: 2px solid ${Theme.colors.fontColor};
  border-radius: 9999px;
  font-weight: bold;
  height: 56px;
  width: 120px;

  &:hover {
    cursor: pointer;
  }
`;

export const Input = styled.input`
  border: 2px solid ${Theme.colors.fontColor};
  border-radius: 8px;
  font-size: ${Theme.fontSize.base}rem;
  height: 56px;
  padding: 1rem;
  width: 320px;

  &:focus {
    outline-color: ${Theme.colors.fontColor};
  }
`;
