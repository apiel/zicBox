import * as React from '@/libs/react';

import { Text } from '@/libs/nativeComponents/Text';

export function Title({ title }: { title: string }) {
    return <Text fontSize={16} text={title} bounds={[16, 0, 100, 16]} font="PoppinsLight_8" />;

    // return <Text fontSize={12} text={title} bounds={[16, 0, 100, 16]} />;
}
