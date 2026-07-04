import * as React from '@/libs/react';

import { ComponentProps, VisibilityContext } from '@/libs/nativeComponents/component';
import { HiddenValue } from '@/libs/nativeComponents/HiddenValue';
import { Text } from '@/libs/nativeComponents/Text';
import { Bounds, getBounds, rgb } from '@/libs/ui';
import { Rect } from '../nativeComponents/Rect';

export function TextGrid({
    bounds,
    selected,
    rows,
    keys,
    contextValue,
    selectedBackground,
    bgColor = 'background',
    textColor,
    shiftedTextColor,
    pageCount,
    currentPage,
    context,
}: {
    bounds: Bounds;
    selected?: string;
    rows: string[];
    keys?: { key: string; action: string; action2?: string }[];
    contextValue?: number[];
    context?: VisibilityContext[];
    selectedBackground?: string;
    bgColor?: string;
    textColor?: string;
    shiftedTextColor?: string;
    pageCount?: number;
    currentPage?: number;
}) {
    if (!context && contextValue !== undefined) {
        context = [];
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
                textColor={textColor}
                shiftedTextColor={shiftedTextColor}
                selected={selected}
                pageCount={pageCount}
                currentPage={currentPage}
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
    textColor = 'text',
    selected,
    pageCount,
    currentPage,
    ...props
}: ComponentProps<{
    rows: string[];
    bgColor?: string;
    activeBgColor?: string;
    shiftedTextColor?: string;
    context?: VisibilityContext[];
    textColor?: string;
    selected?: string;
    pageCount?: number;
    currentPage?: number;
}>) {
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
            const bg = item === selected ? activeBgColor : bgColor;

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

            if (pageCount !== undefined && item === selected) {
                children.push(
                    <Rect
                        bounds={[Number(x) + marginLeft + 3, textY + marginTop + 2, 3, 3]}
                        color={currentPage === 1 ? shiftedTextColor : color}
                    />
                );

                children.push(
                    <Rect
                        bounds={[Number(x) + marginLeft + 3, textY + marginTop + 6, 3, 3]}
                        color={currentPage === 2 ? shiftedTextColor : color}
                    />
                );

                if (pageCount > 2) {
                    children.push(
                        <Rect
                            bounds={[Number(x) + marginLeft + 8, textY + marginTop + 2, 3, 3]}
                            color={currentPage === 3 ? shiftedTextColor : color}
                        />
                    );
                }
            }

            marginLeft += width;
        }
        marginTop += h;
    }

    return children;
}
