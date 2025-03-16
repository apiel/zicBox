import * as React from '@/libs/react';

import { ComponentProps, VisibilityContext } from '@/libs/nativeComponents/component';
import { HiddenValue } from '@/libs/nativeComponents/HiddenValue';
import { Text } from '@/libs/nativeComponents/Text';
import { Bounds, getBounds, rgb } from '@/libs/ui';

export function TextGrid({
    bounds,
    selected,
    rows,
    keys,
    contextValue,
    selectedBackground,
    bgColor = 'background',
}: {
    bounds: Bounds;
    selected?: string;
    rows: string[];
    keys?: { key: string; action: string; action2?: string }[];
    contextValue?: number[];
    selectedBackground?: string;
    bgColor?: string;
}) {
    if (selected) {
        for (let i = 0; i < rows.length; i++) {
            rows[i] = rows[i].replace(selected, `!${selected}`);
        }
    }

    const context: VisibilityContext[] = [];
    if (contextValue !== undefined) {
        for (let i = 0; i < contextValue.length; i++) {
            context.push({
                index: 254 - i,
                condition: 'SHOW_WHEN',
                value: contextValue[i],
            });   
        }
    }

    return (
        <>
            <HiddenValue visibilityContext={context} keys={keys} />
            <TextGridRender
                bounds={bounds}
                rows={rows}
                activeBgColor={selectedBackground}
                context={context}
                bgColor={bgColor}
            />
        </>
    );
}

function TextGridRender({
    bounds,
    rows,
    bgColor = 'background',
    activeBgColor = 'primary',
    shiftedTextColor = rgb(80, 75, 75),
    context,
    ...props
}: ComponentProps<{
    rows: string[];
    bgColor?: string;
    activeBgColor?: string;
    shiftedTextColor?: string;
    context?: VisibilityContext[]
}>) {
    const textColor = 'text';
    const h = 11;
    const [x, y, w] = getBounds(bounds);
    const textY = Number(y);
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
                    visibilityContext={context}
                    font="PoppinsLight_8"
                    fontSize={8}
                />
            );
            marginLeft += width;
        }
        marginTop += h;
    }

    return children;
}
