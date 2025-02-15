import * as React from '@/libs/react';

import { getBounds, rgb } from '@/libs/ui';
import { ComponentProps } from '../nativeComponents/component';
import { Text } from '../nativeComponents/Text';

export function TextGrid({
    bounds,
    rows,
    bgColor = "background",
    textColor = "text",
    activeBgColor = "primary",
    shiftedTextColor = rgb(80, 75, 75),
    ...props
}: ComponentProps<{
    rows: string[];
    bgColor?: string;
    textColor?: string;
    activeBgColor?: string;
    shiftedTextColor?: string;
}>) {
    const h = 12;
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
            if (item[0] === '^') {
                item = item.substr(1);
                bg = activeBgColor;
            }

            let color = textColor;
            if (item[0] === '!') {
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
                />
            );
            marginLeft += width;
        }
        marginTop += h;
    }

    return <>{children}</>;
}
