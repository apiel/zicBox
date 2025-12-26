import * as React from '@/libs/react';

import { ClipView } from '../components/ClipView';
import { Keyboard } from '../components/Keyboard';
import { PresetView } from '../components/PresetView';
import { SavePresetView } from '../components/SavePresetView';
import { SeqView } from '../components/SeqView';
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
            <MultiSynthLayout name={synthName} track={track} synthName={synthName} color={color} title={title} />
            <MultiSynthLayout name={`${synthName}:page2`} track={track} synthName={synthName} color={color} title={title} />
            <MultiSynthLayout name={`${synthName}:page3`} track={track} synthName={synthName} color={color} title={title} />
            <MultiSynthLayout name={`${synthName}:page4`} track={track} synthName={synthName} color={color} title={title} />
            <SeqView
                name={`${synthName}Seq`}
                track={track}
                synthName={synthName}
                color={color}
                contextId={contextId}
                title={title}
            />
            <ClipView name={`${synthName}Clips`} track={track} synthName={synthName} color={color} title={title} />
            <Keyboard name={`${synthName}Keyboard`} track={track} synthName={synthName} color={color} title={title} />
            <SavePresetView name={`${synthName}SavePreset`} track={track} synthName={synthName} color={color} title={title} />
            <PresetView name={`${synthName}Preset`} track={track} synthName={synthName} color={color} title={title} />
        </>
    );
}
