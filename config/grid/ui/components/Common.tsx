import * as React from '@/libs/react';

import { HiddenValue } from '@/libs/nativeComponents/HiddenValue';
import { TextGrid } from '@/libs/tsComponents/TextGrid';
import { lighten } from '@/libs/ui';
import {
    A11,
    A12,
    B12,
    btn10,
    btn6,
    btn7,
    btn8,
    btn9,
    ColorButton,
    SelectorPosition,
    SelectorPosition2,
    shiftContext,
} from '../constants';

export const Clips = 'Clips';
export const Drum1 = 'Kick23';
export const Drum2 = 'Drum2';
export const Drum3 = 'Drum3';
export const Drum4 = 'Drum4';
export const Synth1 = 'Synth1';
export const Synth2 = 'Synth2';
export const Synth3 = 'Synth3';
export const Sample1 = 'Sample1';
export const Sample2 = 'Sample2';
export const Sample3 = 'Sample3';

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
            rows={[`${Drum1} ${Sample1} ${Sample2}`]}
            keys={[
                {
                    key: btn6,
                    action: viewName === `Drum1` ? `setView:Drum1:page2` : `setView:Drum1`,
                    action2: 'setActiveTrack:1',
                },
                { key: btn7, action: 'setView:Sample1', action2: 'setActiveTrack:2' },
                { key: btn8, action: 'setView:Sample2', action2: 'setActiveTrack:3' },
                { key: btn9, action: 'setView:Sample3', action2: 'setActiveTrack:4' },
                { key: btn10, action: `setView:Sample4`, action2: 'setActiveTrack:5' },
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
    viewName,
    forcePatchView = false,
}: {
    synthName: string;
    viewName: string;
    forcePatchView?: boolean;
}) {
    return (
        <>
            <HiddenValue
                keys={[
                    {
                        key: A11,
                        action:
                            viewName === `${synthName}Seq`
                                ? `setView:${synthName}`
                                : `setView:${synthName}Seq`,
                    },

                    { key: A12, action: `contextToggle:${shiftContext}:1:0` },
                    { key: B12, action: `playPause` },
                ]}
                controllerColors={[
                    {
                        controller: 'Default',
                        colors: [
                            { key: A11, color: ColorButton },
                            { key: A12, color: ColorButton },
                            { key: B12, color: ColorButton },
                        ],
                    },
                ]}
            />
            {/* <HiddenValue
                keys={[
                    { key: right1, action: 'setView:Clips' },
                    { key: right3, action: 'contextToggle:254:2:0' },

                    // { key: left1, action: `setView:${synthName}Sequencer`, action2: `setContext:254:0` },
                    // { key: left1, action: `setView:DrumSeq`, action2: `setContext:254:0` },
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
                rows={[`Project Shutdown &empty &icon::stop::filled &icon::play::filled`, `Server &empty &empty &empty BPM`]}
                keys={[
                    { key: btn1, action: `setView:Workspaces` },
                    { key: btn2, action: `setView:Shutdown` },
                    { key: btn4, action: `stop` },
                    { key: btn5, action: `playPause` },

                    { key: btn6, action: `setView:FileServer` },
                    { key: btn10, action: `setView:Tempo` },
                ]}
                contextValue={[2]}
            /> */}
        </>
    );
}
