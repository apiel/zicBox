import * as React from '@/libs/react';

import { getBounds, rgb } from '@/libs/ui';
import { ComponentProps } from '../nativeComponents/component';
import { Text } from '../nativeComponents/Text';
import { VisibilityContext } from '../nativeComponents/VisibilityContext';

export function TextGrid({
    bounds,
    rows,
    bgColor = 'background',
    textColor = 'text',
    activeBgColor = 'primary',
    shiftedTextColor = rgb(80, 75, 75),
    visibilityCondition,
    visibilityContext,
    visibilityValue,
    ...props
}: ComponentProps<{
    rows: string[];
    bgColor?: string;
    textColor?: string;
    activeBgColor?: string;
    shiftedTextColor?: string;
    visibilityContext?: number;
    visibilityCondition?: 'SHOW_WHEN_NOT' | 'SHOW_WHEN_OVER' | 'SHOW_WHEN_UNDER' | 'SHOW_WHEN';
    visibilityValue?: number;
}>) {
    const h = 11;
    const [x, y, w] = getBounds(bounds);
    const textY = Number(y) + 2;
    const children = [];
    let marginTop = 0;
    for (const row of rows) {
        const items = row.split(' ');
        const width = Number(w) / items.length;
        let marginLeft = 0;
        for (let item of items) {
            let bg = bgColor;
            if (item[0] === '!') {
                item = item.substr(1);
                bg = activeBgColor;
            }

            let color = textColor;
            if (item[0] === '^') {
                item = item.substr(1);
                color = shiftedTextColor;
            }

            children.push(
                <Text
                    text={item}
                    bounds={[Number(x) + marginLeft, textY + marginTop, width, h]}
                    {...props}
                    centered
                    bgColor={bg}
                    color={color}
                >
                    {visibilityContext && visibilityCondition && visibilityValue && (
                        <VisibilityContext
                            index={visibilityContext}
                            condition={visibilityCondition}
                            value={visibilityValue}
                        />
                    )}
                </Text>
            );
            marginLeft += width;
        }
        marginTop += h;
    }

    return children;
}
