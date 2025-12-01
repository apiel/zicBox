import * as React from '@/libs/react';

import { Container } from '@/libs/nativeComponents/Container';
import { View } from '@/libs/nativeComponents/View';
import { ClipView } from '../components/ClipView';
import { Keyboard } from '../components/Keyboard';
import { PresetView } from '../components/PresetView';
import { SavePresetView } from '../components/SavePresetView';
import { SeqView } from '../components/SeqView';
import { TimelinePart } from '../components/TimelinePart';
import { ColorTrack1, ColorTrack2, ColorTrack3, ColorTrack4, ColorTrack5, ColorTrack6, Track1, Track2, Track3, Track4, Track5, Track6 } from '../constants';
import { MultiSynthLayout } from './MultiSynthLayout';

export type Props = {
    track: number;
    synthName: string;
    color: string;
    contextId: number;
    title: string;
};

export function MultiSynthViews({ track, synthName, color, contextId, title }: Props) {
    return (
        <>
            
            <View name="Timeline">
                <Container name="Track1" position={[0, 0]} height="180px">
                    <MultiSynthLayout name="Track1" track={Track1} synthName="Track1" color={ColorTrack1} title="1." />
                    <MultiSynthLayout name="Track2" track={Track2} synthName="Track2" color={ColorTrack2} title="2." />
                    <MultiSynthLayout name="Track3" track={Track3} synthName="Track3" color={ColorTrack3} title="3." />
                    <MultiSynthLayout name="Track4" track={Track4} synthName="Track4" color={ColorTrack4} title="4." />
                    <MultiSynthLayout name="Track5" track={Track5} synthName="Track5" color={ColorTrack5} title="5." />
                    <MultiSynthLayout name="Track6" track={Track6} synthName="Track6" color={ColorTrack6} title="6." />
                </Container>
                <Container name="Timeline" position={[0, 180]} height="100%">
                    <TimelinePart />
                </Container>
            </View>
            <SeqView
                name={`${synthName}Seq`}
                track={track}
                synthName={synthName}
                color={color}
                contextId={contextId}
                title={title}
            />
            <ClipView
                name={`${synthName}Clips`}
                track={track}
                synthName={synthName}
                color={color}
                title={title}
            />
            <Keyboard
                name={`${synthName}Keyboard`}
                track={track}
                synthName={synthName}
                color={color}
                title={title}
            />
            <SavePresetView
                name={`${synthName}SavePreset`}
                track={track}
                synthName={synthName}
                color={color}
                title={title}
            />
            <PresetView
                name={`${synthName}Preset`}
                track={track}
                synthName={synthName}
                color={color}
                title={title}
            />
        </>
    );
}
