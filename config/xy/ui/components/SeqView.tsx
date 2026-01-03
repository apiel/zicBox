import * as React from '@/libs/react';

import { VisibilityContext } from '@/libs/nativeComponents/component';
import { Rect } from '@/libs/nativeComponents/Rect';
import { SequencerCard } from '@/libs/nativeComponents/SequencerCard';
import { SequencerValue, SequencerValueType } from '@/libs/nativeComponents/SequencerValue';
import { StringVal } from '@/libs/nativeComponents/StringVal';
import { Text } from '@/libs/nativeComponents/Text';
import { View } from '@/libs/nativeComponents/View';
import { rgb } from '@/libs/ui';
import { A1, A2, A3, A4, A5, B1, B2, B3, B4, B5, B6, B7, B8, ScreenHeight, ScreenWidth, shiftContext, W1_4, W1_8, W3_4 } from '../constants';
import { shiftVisibilityContext, unshiftVisibilityContext } from './ShiftLayout';

const seqmarginLeft = 3;
export const seqTop = 30;
export const seqWidth = W1_4;
export const bounds1Seq = [seqmarginLeft + 0, seqTop, seqWidth, 50];
export const bounds2Seq = [seqmarginLeft + seqWidth, seqTop, seqWidth, 50];
export const bounds3Seq = [seqmarginLeft + seqWidth * 2, seqTop, seqWidth, 50];
export const bounds4Seq = [seqmarginLeft + seqWidth * 3, seqTop, seqWidth, 50];

export const bounds5Seq = [seqmarginLeft + 0, seqTop + 60, seqWidth, 50];
export const bounds6Seq = [seqmarginLeft + seqWidth, seqTop + 60, seqWidth, 50];
export const bounds7Seq = [seqmarginLeft + seqWidth * 2, seqTop + 60, seqWidth, 50];
export const bounds8Seq = [seqmarginLeft + seqWidth * 3, seqTop + 60, seqWidth, 50];

export const enc1Seq = { encoderId: 1, bounds: bounds1Seq };
export const enc2Seq = { encoderId: 2, bounds: bounds2Seq };
export const enc3Seq = { encoderId: 3, bounds: bounds3Seq };
export const enc4Seq = { encoderId: 4, bounds: bounds4Seq };
export const enc5Seq = { encoderId: 5, bounds: bounds5Seq };
export const enc6Seq = { encoderId: 6, bounds: bounds6Seq };
export const enc7Seq = { encoderId: 7, bounds: bounds7Seq };
export const enc8Seq = { encoderId: 8, bounds: bounds8Seq };

export function SeqView({
    name,
    track,
    synthName,
    color,
    contextId,
    title,
}: {
    name: string;
    track: number;
    synthName: string;
    color: string;
    contextId: number;
    title: string;
}) {
    const row1: VisibilityContext[] = [unshiftVisibilityContext];
    const row2: VisibilityContext[] = [shiftVisibilityContext];
    const colorOff = rgb(90, 90, 90);
    const menuTextColor = rgb(128, 128, 128);

    const SeqVal = ({
        encoderId,
        bounds,
        type,
        visibilityContext,
        off,
    }: {
        encoderId: number;
        bounds: number[];
        type: SequencerValueType;
        visibilityContext: VisibilityContext[];
        off?: boolean;
    }) => {
        return (
            <SequencerValue
                bounds={bounds}
                encoderId={off ? -1 : encoderId}
                audioPlugin={`Sequencer`}
                track={track}
                contextId={contextId}
                fontValue={'PoppinsLight_24'}
                fontLabel={'PoppinsLight_12'}
                type={type}
                visibilityContext={visibilityContext}
                {...(off && { labelColor: colorOff, valueColor: colorOff })}
            />
        );
    };

    return (
        <View name={name}>
            <Rect bounds={[W3_4, 0, W1_4, ScreenHeight]} />
            <Rect bounds={[0, 0, 50, 16]} color={color} />
            <Text text={title} bounds={[2, 0, 20, 16]} font="PoppinsLight_12" bgColor={color} />
            <StringVal
                audioPlugin={synthName}
                param="ENGINE"
                bounds={[15, 0, 40, 16]}
                fontLabel="PoppinsLight_12"
                labelColor="#FFFFFF"
                unit
                bgColor={color}
            />
            <StringVal audioPlugin={synthName} param="ENGINE" bounds={[60, 0, 80, 16]} fontLabel="PoppinsLight_12" />

            <Rect bounds={[0, seqTop - 10, 2, 60]} color={rgb(100, 100, 100)} visibilityContext={row1} />
            <SeqVal {...enc1Seq} type={'STEP_SELECTION'} visibilityContext={row1} />
            <SeqVal {...enc2Seq} type={'STEP_LENGTH_AND_TOGGLE'} visibilityContext={row1} />
            <SeqVal {...enc3Seq} type={'STEP_NOTE'} visibilityContext={row1} />
            <SeqVal {...enc4Seq} type={'STEP_VELOCITY'} visibilityContext={row1} />

            <Rect bounds={[0, seqTop + 48, 2, 60]} color="background" visibilityContext={row1} />
            <SeqVal {...enc5Seq} type={'STEP_CONDITION'} visibilityContext={row1} off />
            <SeqVal {...enc6Seq} type={'STEP_MOTION'} visibilityContext={row1} off />

            <Rect bounds={[0, seqTop - 10, 2, 60]} color="background" visibilityContext={row2} />
            <SeqVal {...enc1Seq} type={'STEP_SELECTION'} visibilityContext={row2} off />
            <SeqVal {...enc2Seq} type={'STEP_LENGTH_AND_TOGGLE'} visibilityContext={row2} off />
            <SeqVal {...enc3Seq} type={'STEP_NOTE'} visibilityContext={row2} off />
            <SeqVal {...enc4Seq} type={'STEP_VELOCITY'} visibilityContext={row2} off />

            <Rect bounds={[0, seqTop + 48, 2, 60]} color={rgb(100, 100, 100)} visibilityContext={row2} />
            <SeqVal {...enc5Seq} type={'STEP_CONDITION'} visibilityContext={row2} />
            <SeqVal {...enc6Seq} type={'STEP_MOTION'} visibilityContext={row2} />

            {['&icon::arrowUp::filled', '&icon::arrowDown::filled', '&icon::play::filled', 'Exit', 'Shift'].map((text, index) => {
                return (
                    <Text
                        text={text}
                        bounds={[index * W1_8, ScreenHeight - (text[0] === '&' ? 98 : 100), W1_8, 16]}
                        centered={true}
                        visibilityContext={[unshiftVisibilityContext]}
                        color={menuTextColor}
                    />
                );
            })}

            {['---', '---', '---', 'Clear', 'Shift'].map((text, index) => {
                return (
                    <Text
                        text={text}
                        bounds={[index * W1_8, ScreenHeight - (text[0] === '&' ? 98 : 100), W1_8, 16]}
                        centered={true}
                        visibilityContext={[shiftVisibilityContext]}
                        color={text === 'Shift' ? rgb(80, 80, 80) : menuTextColor}
                    />
                );
            })}

            <SequencerCard
                bounds={[0, ScreenHeight - 81, ScreenWidth - 10, 80]}
                audioPlugin={`Sequencer`}
                track={track}
                contextId={contextId}
                rowsSelection={1}
                stepPerRow={8}
                // rowsSelectionColor={"#28595f"}
                gridKeys={[B1, B2, B3, B4, B5, B6, B7, B8]}
                keys={[
                    {
                        key: A1,
                        action: '.scroll:-1',
                        context: { id: shiftContext, value: 0 },
                    },
                    { key: A2, action: '.scroll', context: { id: shiftContext, value: 0 } },
                    {
                        key: A3,
                        action: `playPause`,
                        context: { id: shiftContext, value: 0 },
                    },
                    {
                        key: A4,
                        action: `setView:${synthName}`,
                        context: { id: shiftContext, value: 0 },
                    },
                    {
                        key: A4,
                        action: `data:Sequencer:${track}:CLEAR_STEPS`,
                        context: { id: shiftContext, value: 1 },
                        action2: '.scroll:0',
                    },
                    {
                        key: A5,
                        action: `contextToggle:${shiftContext}:1:0`,
                        multipleKeyHandler: true,
                    },
                ]}
            />
        </View>
    );
}
