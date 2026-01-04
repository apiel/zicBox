import * as React from '@/libs/react';
import {
    ColorTrack1,
    ColorTrack2,
    ColorTrack3,
    ColorTrack4,
    ColorTrack5,
    ColorTrack6,
    ColorTrack7,
    ColorTrack8,
    ScreenHeight,
    ScreenWidth,
    Track1,
    Track2,
    Track3,
    Track4,
    Track5,
    Track6,
    Track7,
    Track8,
    W1_4,
    W2_4,
    W3_4,
    seqContextTrack1,
    seqContextTrack2,
    seqContextTrack3,
    seqContextTrack4,
    seqContextTrack5,
    seqContextTrack6,
    seqContextTrack7,
    seqContextTrack8,
} from './constants';
import { MasterView } from './MasterView';
import { PresetSaveView } from './PresetSaveView';
import { PresetView } from './PresetView';
import { RecView } from './RecView';
import { SeqView } from './SeqView';
import { ShuttingDownView } from './ShuttingDownView';
import { SynthView } from './SynthView';

export const ui = {
    pixelController: 'xy',
    screen: {
        windowPosition: { x: 200, y: 300 },
        screenSize: { width: ScreenWidth, height: ScreenHeight },
        zonesEncoders: [
            [0, 0, 0, 0], // lets skip encoder id 0
            [0, 0, W1_4, ScreenHeight],
            [W1_4, 0, W1_4, ScreenHeight],
            [W2_4, 0, W1_4, ScreenHeight],
            [W3_4, 0, W1_4, ScreenHeight],
        ],
        st7789: {
            resetPin: 17,
            yRamMargin: 0,
        },
    },
    renderer: 'ST7789',
    taggedViews: { track: 'Track1' },
    views: (
        <>
            <MultiSynthViews track={Track1} synthName="Track1" color={ColorTrack1} title="1." contextId={seqContextTrack1} />
            <MultiSynthViews track={Track2} synthName="Track2" color={ColorTrack2} title="2." contextId={seqContextTrack2} />
            <MultiSynthViews track={Track3} synthName="Track3" color={ColorTrack3} title="3." contextId={seqContextTrack3} />
            <MultiSynthViews track={Track4} synthName="Track4" color={ColorTrack4} title="4." contextId={seqContextTrack4} />

            <MultiSynthViews track={Track5} synthName="Track5" color={ColorTrack5} title="5." contextId={seqContextTrack5} />
            <MultiSynthViews track={Track6} synthName="Track6" color={ColorTrack6} title="6." contextId={seqContextTrack6} />
            <MultiSynthViews track={Track7} synthName="Track7" color={ColorTrack7} title="7." contextId={seqContextTrack7} />
            <MultiSynthViews track={Track8} synthName="Track8" color={ColorTrack8} title="8." contextId={seqContextTrack8} />

            <MasterView name="Master" />
            <ShuttingDownView name="ShuttingDown" />
        </>
    ),
};

type Props = {
    track: number;
    synthName: string;
    color: string;
    contextId: number;
    title: string;
};
function MultiSynthViews({ track, synthName, color, contextId, title }: Props) {
    return (
        <>
            <SynthView name={synthName} track={track} synthName={synthName} color={color} title={title} />
            <SynthView name={`${synthName}:page2`} track={track} synthName={synthName} color={color} title={title} />
            <SynthView name={`${synthName}:page3`} track={track} synthName={synthName} color={color} title={title} />
            <SynthView name={`${synthName}:page4`} track={track} synthName={synthName} color={color} title={title} />
            <SeqView name={`${synthName}Seq`} track={track} synthName={synthName} color={color} contextId={contextId} title={title} />
            <RecView name={`${synthName}Rec`} track={track} synthName={synthName} color={color} title={title} />
            <PresetSaveView name={`${synthName}PresetSave`} track={track} synthName={synthName} color={color} title={title} />
            <PresetView name={`${synthName}Preset`} track={track} synthName={synthName} color={color} title={title} />
        </>
    );
}
