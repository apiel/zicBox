import * as React from '@/libs/react';

import { Text } from '@/libs/components/Text';
import { ScreenWidth } from '../constants';

export function Title({ title }: { title: string }) {
    return <Text fontSize={16} text={title} bounds={[0, 20, ScreenWidth, 16]} centered />;
}
