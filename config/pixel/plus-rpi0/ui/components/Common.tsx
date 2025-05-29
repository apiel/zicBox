import * as React from '@/libs/react';

import { Rect } from '@/libs/nativeComponents/Rect';
import { TextGrid } from '@/libs/tsComponents/TextGrid';
import { lighten, rgb } from '@/libs/ui';
import {
    btn1,
    btn10,
    btn2,
    btn5,
    btn6,
    btn7,
    btn8,
    btn9,
    btnUp,
    KeyInfoPosition,
    KeyTopInfoPosition,
    SelectorPosition2,
} from '../constants';

export const Clips = 'Clips';
export const Drum1 = 'Drum23';
export const Rack1 = 'Rack1';
export const Rack2 = 'Rack2';
export const Rack3 = 'Rack3';
export const Rack4 = 'Rack4';

export function Common({
    selectedBackground,
    selected,
    synthName,
}: {
    track: number; // To be deprecated ??
    selectedBackground?: string;
    selected: string;
    synthName: string;
}) {
    return (
        <>{/* <ShiftedTextGrid selectedBackground={selectedBackground} selected={selected} /> */}</>
    );
}

export function ShiftedTextGrid({
    selectedBackground,
    selected,
}: {
    selectedBackground?: string;
    selected: string;
}) {
    return (
        <>
            <TextGrid
                bounds={KeyTopInfoPosition}
                rows={[`Clips &icon::play::filled ^Tracks`]}
                contextValue={[1]}
                selected={selected}
                {...(selectedBackground && { selectedBackground })}
                bgColor={rgb(77, 86, 88)}
                // textColor={rgb(163, 163, 163)}
                // textColor={rgb(207, 206, 206)}
                shiftedTextColor={rgb(134, 146, 148)}
            />
            <Rect
                bounds={KeyTopInfoPosition}
                color="background"
                visibilityContext={[
                    {
                        index: 254,
                        condition: 'SHOW_WHEN',
                        value: 0,
                    },
                ]}
            />
            {/* <TextGrid
                bounds={KeyInfoPosition}
                rows={[
                    `${Kick} ${Synth1} ${Synth2} ${Synth3}`,
                    `${Drum1} ${Sample1} ${Sample2} ${Sample3}`,
                ]}
                keys={[
                    { key: btn1, action: 'setView:Kick' },
                    { key: btn2, action: 'setView:Synth1' },
                    { key: btn3, action: 'setView:Synth2' },
                    { key: btn4, action: 'setView:Synth3' },

                    { key: btn5, action: `setView:Drum1` },
                    { key: btn6, action: `setView:Sample1` },
                    { key: btn7, action: `setView:Sample2` },
                    { key: btn8, action: `setView:Sample3` },

                    { key: btnUp, action: 'setView:Clips' },
                    { key: btnDown, action: 'playPause' },
                    { key: btnShift, action: 'contextToggle:254:1:0' },
                ]}
                selected={selected}
                contextValue={[1]}
                {...(selectedBackground && { selectedBackground })}
            /> */}
            {/* <TracksTextGrid selectedBackground={selectedBackground} selected={selected} contextValue={1} /> */}

            <TextGrid
                bounds={KeyInfoPosition}
                rows={[`Menu Tempo &empty &empty`, `&icon::play::filled &empty &empty &empty`]}
                keys={[
                    { key: btn1, action: 'setView:Menu' },
                    { key: btn2, action: 'setView:Tempo' },
                    // { key: btn3, action: 'setView:Synth2' },
                    // { key: btn4, action: 'setView:Synth3' },

                    { key: btn5, action: `playPause` },
                    // { key: btn6, action: `setView:Sample1` },
                    // { key: btn7, action: `setView:Sample2` },
                    // { key: btn8, action: `setView:Sample3` },

                    { key: btnUp, action: 'contextToggle:253:1:0' },
                    // { key: btnDown, action: 'playPause' },
                    // { key: btnShift, action: 'contextToggle:254:1:0' },
                ]}
                selected={selected}
                contextValue={[0, 1]}
                {...(selectedBackground && { selectedBackground })}
            />
        </>
    );
}

export function TracksSelector({
    selectedBackground,
    selected,
    viewName,
    pageCount,
    currentPage,
    contextValue = 0,
}: {
    selectedBackground?: string;
    selected?: string;
    viewName: string;
    contextValue?: number;
    pageCount?: number;
    currentPage?: number;
}) {
    return (
        <TextGrid
            pageCount={pageCount}
            currentPage={currentPage}
            bounds={SelectorPosition2}
            rows={[`${Drum1} ${Rack1} ${Rack2} ${Rack3} ${Rack4}`]}
            keys={[
                {
                    key: btn6,
                    action: viewName === `Drum1` ? `setView:Drum1:page2` : `setView:Drum1`,
                    action2: 'setActiveTrack:1',
                },
                { key: btn7, action: 'setView:Rack1', action2: 'setActiveTrack:2' },
                { key: btn8, action: 'setView:Rack2', action2: 'setActiveTrack:3' },
                { key: btn9, action: 'setView:Rack3', action2: 'setActiveTrack:4' },
                { key: btn10, action: `setView:Rack4`, action2: 'setActiveTrack:5' },
            ]}
            selected={selected}
            contextValue={[contextValue]}
            {...(selectedBackground && {
                selectedBackground,
                shiftedTextColor: lighten(selectedBackground, 0.5),
            })}
        />
    );
}
