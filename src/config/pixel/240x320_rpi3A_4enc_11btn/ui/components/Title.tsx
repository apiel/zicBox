import * as React from '@/libs/react';

import { Text } from '@/libs/components/Text';
import { ScreenWidth } from '../constants';

export function Title({ title }: { title: string }) {
    return <Text font_size={16} text={title} position={[0, 20, ScreenWidth, 16]} centered />;
}
