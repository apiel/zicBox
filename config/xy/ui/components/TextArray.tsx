import * as React from '@/libs/react';

import { VisibilityContext } from '@/libs/nativeComponents/component';
import { Text } from '@/libs/nativeComponents/Text';
import { rgb } from '@/libs/ui';
import { W1_8 } from '../constants';

export type Props = {
    texts: string[];
    top: number;
    color?: string;
    bgColor?: string;
    visibilityContext?: VisibilityContext[];
};

const menuTextColor = rgb(110, 110, 110);

export function TextArray({ texts, top, color, bgColor, visibilityContext }: Props) {
    return (
        <>
            {texts.map((text, index) => {
                return (
                    <Text
                        text={text}
                        bounds={[index * W1_8,  top, W1_8, 16]}
                        centered={true}
                        visibilityContext={visibilityContext}
                        color={text === 'Shift' ? rgb(80, 80, 80) : color || menuTextColor}
                        bgColor={bgColor} //  || rgb(40, 40, 40)
                    />
                );
            })}
        </>
    );
}
