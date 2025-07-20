import * as React from '@/libs/react';

import { VisibilityContext } from '@/libs/nativeComponents/component';
import { GraphEncoder } from '@/libs/nativeComponents/GraphEncoder';
import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { Rect } from '@/libs/nativeComponents/Rect';
import { SequencerCard } from '@/libs/nativeComponents/SequencerCard';
import { Value } from '@/libs/nativeComponents/Value';
import { rgb } from '@/libs/ui';
import { Drum2 } from '../components/Common';
import { KeysTracks } from '../components/KeysTracks';
import { Layout } from '../components/Layout';
import { shiftContext } from '../constants';
import {
    backgroundBounds,
    bounds2,
    boundsMarginTop,
    enc1,
    enc2,
    enc3,
    encBottomValue,
    seqCardBounds_small,
} from '../constantsValue';

export type Props = {
    name: string;
    track: number;
    synthName: string;
    color: string;
};

export function MainView({ name, track, synthName, color }: Props) {
    const visibilityContext: VisibilityContext = {
        condition: 'SHOW_WHEN',
        index: shiftContext,
        value: 0,
    };

    return (
        <Layout
            viewName={name}
            color={color}
            synthName={synthName}
            title={Drum2}
            content={
                <>
                    <Rect
                        bounds={backgroundBounds}
                        color="background"
                        visibilityContext={[
                            { condition: 'SHOW_WHEN', index: shiftContext, value: 1 },
                        ]}
                    />
                    <KnobValue
                        audioPlugin={synthName}
                        param="ENGINE"
                        {...enc1}
                        color="secondary"
                        track={track}
                        visibilityContext={[
                            { condition: 'SHOW_WHEN', index: shiftContext, value: 1 },
                        ]}
                    />

                    <KnobValue
                        audioPlugin={synthName}
                        param="DURATION"
                        {...enc1}
                        color="secondary"
                        track={track}
                        visibilityContext={[visibilityContext]}
                    />

                    <GraphEncoder
                        bounds={boundsMarginTop(bounds2)}
                        audioPlugin={synthName}
                        dataId="ENV_AMP_FORM"
                        values={['AMP_MORPH']}
                        outlineColor="quaternary"
                        fillColor={rgb(194, 175, 107)}
                        track={track}
                        visibilityContext={[visibilityContext]}
                    />
                    <Value
                        {...encBottomValue(enc2)}
                        audioPlugin={synthName}
                        param="AMP_MORPH"
                        track={track}
                        barHeight={1}
                        barColor="quaternary"
                        alignLeft
                        showLabelOverValue={1}
                        visibilityContext={[visibilityContext]}
                    />

                    <KnobValue
                        audioPlugin={synthName}
                        param="VAL_1"
                        {...enc3}
                        color="primary"
                        track={track}
                        visibilityContext={[visibilityContext]}
                    />

                    {/* 

                    <KnobValue
                        audioPlugin={synthName}
                        param="WAVESHAPE"
                        {...enc4}
                        color="tertiary"
                        track={track}
                    />

                    <KnobValue
                        audioPlugin={synthName}
                        param="COMPRESS"
                        {...enc7}
                        color="secondary"
                        track={track}
                    />
                    <KnobValue
                        audioPlugin={synthName}
                        param="DRIVE"
                        {...enc8}
                        color="quaternary"
                        track={track}
                    />

                    <KnobValue
                        audioPlugin={synthName}
                        param="BASS"
                        {...enc11}
                        color="tertiary"
                        track={track}
                    />
                    <KnobValue
                        audioPlugin={synthName}
                        param="HIGH_FREQ_BOOST"
                        {...enc12}
                        color="tertiary"
                        track={track}
                    />

                    <GraphEncoder
                        bounds={graphBounds(bounds5)}
                        audioPlugin={synthName}
                        dataId="WAVEFORM"
                        renderValuesOnTop={false}
                        values={['WAVEFORM_TYPE', 'SHAPE', 'MACRO']}
                        track={track}
                    />
                    <Value
                        {...encTopValue(enc5)}
                        audioPlugin={synthName}
                        param="WAVEFORM_TYPE"
                        track={track}
                        barHeight={1}
                        // barColor="primary"
                        alignLeft
                        showLabelOverValue={1}
                    />
                    <Value
                        {...encTopValue(enc6)}
                        audioPlugin={synthName}
                        param="SHAPE"
                        track={track}
                        barHeight={1}
                        // barColor="primary"
                        alignLeft
                        showLabelOverValue={1}
                    />
                    <Value
                        {...encBottomValue(enc9)}
                        audioPlugin={synthName}
                        param="MACRO"
                        track={track}
                        barHeight={1}
                        // barColor="primary"
                        alignLeft
                        showLabelOverValue={1}
                    />
                    <Value
                        {...encBottomValue(enc10)}
                        audioPlugin={synthName}
                        param="PITCH"
                        track={track}
                        barHeight={1}
                        // barColor="primary"
                        alignLeft
                        showLabelOverValue={1}
                    />

                    */}

                     <SequencerCard
                        bounds={seqCardBounds_small}
                        audioPlugin={`Sequencer`}
                        track={track}
                        visibilityContext={[visibilityContext]}
                    />

                    <KeysTracks synthName={synthName} viewName={name} />
                </>
            }
        />
    );
}
