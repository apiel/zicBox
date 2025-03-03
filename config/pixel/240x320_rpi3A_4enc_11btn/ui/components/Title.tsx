import * as React from '@/libs/react';

import { Text } from '@/libs/nativeComponents/Text';
import { ScreenWidth } from '../constants';

export function Title({ title }: { title: string }) {
    return <Text fontSize={24} text={title} bounds={[0, 20, ScreenWidth, 16]} centered />;
}
