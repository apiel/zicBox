import * as React from '@/libs/react';

import { Clips } from '@/libs/nativeComponents/Clips';
import {
    clipRenderContext
} from '../constants';

export type Props = {
    track: number;
    color: string;
};

export function ClipBar({ track, color }: Props) {
    return (
        <>
            {/* <Text
                text="Press variation to save preset."
                bounds={[0, ScreenHeight - 34, ScreenWidth, 16]}
                centered={true}
                color={rgb(100, 100, 100)}
                visibilityContext={[shiftVisibilityContext]}
            />
            <Rect
                bounds={[0, ScreenHeight - 34, ScreenWidth, 16]}
                color="background"
                visibilityContext={[unshiftVisibilityContext]}
            /> */}

            <Clips
                bounds={[152, 2, 170, 14]}
                track={track}
                color={color}
                visibleCount={8}
                renderContextId={clipRenderContext}
            />
        </>
    );
}
