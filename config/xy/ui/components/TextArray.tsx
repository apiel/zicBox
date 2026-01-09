import * as React from '@/libs/react';

import { VisibilityContext } from '@/libs/nativeComponents/component';
import { Text } from '@/libs/nativeComponents/Text';
import { rgb } from '@/libs/ui';
import { menuTextColor, W1_8 } from '../constants';

export type Props = {
    texts: (string | string[])[];
    top: number;
    color?: string;
    bgColor?: string;
    visibilityContext?: VisibilityContext[];
};

export function TextArray({ texts, top, color, bgColor, visibilityContext }: Props) {
    return (
        <>
            {texts.map((text, index) => {
                const c = (Array.isArray(text) ? text[2] : color) || menuTextColor;
                const bg = Array.isArray(text) && text[1] ? text[1] : bgColor;
                const t = Array.isArray(text) ? text[0] : text;
                return (
                    <Text
                        text={t}
                        bounds={[index * W1_8,  top, W1_8, 16]}
                        centered={true}
                        visibilityContext={visibilityContext}
                        color={text === 'Shift' ? rgb(80, 80, 80) : c}
                        bgColor={bg}
                    />
                );
            })}
        </>
    );
}
