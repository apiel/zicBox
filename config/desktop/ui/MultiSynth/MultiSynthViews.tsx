import * as React from '@/libs/react';

import { Container } from '@/libs/nativeComponents/Container';
import { View } from '@/libs/nativeComponents/View';
import { ClipView } from '../components/ClipView';
import { Keyboard } from '../components/Keyboard';
import { PresetView } from '../components/PresetView';
import { SavePresetView } from '../components/SavePresetView';
import { SeqView } from '../components/SeqView';
import { TimelinePart } from '../components/TimelinePart';
import { ColorTrack1 } from '../constants';
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
                    <MultiSynthLayout name="Track1" track={track} synthName="Track1" color={ColorTrack1} title="1." />
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
