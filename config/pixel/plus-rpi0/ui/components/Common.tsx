import * as React from '@/libs/react';

import { HiddenValue } from '@/libs/nativeComponents/HiddenValue';
import { Rect } from '@/libs/nativeComponents/Rect';
import { TextGrid } from '@/libs/tsComponents/TextGrid';
import { lighten, rgb } from '@/libs/ui';
import {
    btn1,
    btn10,
    btn2,
    btn4,
    btn5,
    btn6,
    btn7,
    btn8,
    btn9,
    btnUp,
    KeyInfoPosition,
    KeyTopInfoPosition,
    left1,
    left2,
    left3,
    right1,
    right3,
    SelectorPosition,
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

function TracksSelectorSub({
    selectedBackground,
    selected,
    viewName,
    pageCount,
    currentPage,
    contextValue,
}: {
    selectedBackground?: string;
    selected?: string;
    viewName: string;
    contextValue: number;
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

export function TracksSelector({
    selectedBackground,
    selected,
    trackName,
    viewName,
    pageCount,
    currentPage,
}: {
    selectedBackground?: string;
    selected?: string;
    trackName: string;
    viewName: string;
    pageCount?: number;
    currentPage?: number;
}) {
    return (
        <>
            <TracksSelectorSub
                selectedBackground={selectedBackground}
                selected={selected}
                viewName={viewName}
                pageCount={pageCount}
                currentPage={currentPage}
                contextValue={0}
            />
            <TracksSelectorSub
                selectedBackground={selectedBackground}
                selected={trackName}
                viewName={viewName}
                pageCount={pageCount}
                currentPage={currentPage}
                contextValue={1}
            />
        </>
    );
}

export function MuteTracks() {
    return (
        <TextGrid
            bounds={SelectorPosition}
            rows={[`Mute Mute Mute Mute Mute`]}
            keys={[]}
            contextValue={[1]}
        />
    );
}

export function MainKeys({
    synthName,
    forcePatchView = false,
}: {
    synthName: string;
    forcePatchView?: boolean;
}) {
    return (
        <>
            <HiddenValue
                keys={[
                    { key: right1, action: 'setView:Clips' },
                    { key: right3, action: 'contextToggle:254:2:0' },

                    { key: left1, action: `setView:${synthName}Sequencer`, action2: `setContext:254:0` },
                    {
                        key: left2,
                        action: forcePatchView
                            ? `setContext:254:0`
                            : 'contextToggleOnRelease:254:1:0',
                        action2: `setView:${synthName}`,
                    },
                    { key: left3, action: `noteOn:${synthName}:60` },
                ]}
            />
            <Rect bounds={SelectorPosition} visibilityContext={[{ index: 254, value: 2, condition: 'SHOW_WHEN' }]} />
            <TextGrid
                bounds={SelectorPosition}
                rows={[`&empty &empty &empty &icon::stop::filled &icon::play::filled`, `&empty &empty &empty &empty &empty`]}
                keys={[
                    { key: btn4, action: `stop` },
                    { key: btn5, action: `playPause` },
                ]}
                contextValue={[2]}
            />
        </>
    );
}
