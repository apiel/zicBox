import * as React from '@/libs/react';

import { HiddenValue } from '@/libs/nativeComponents/HiddenValue';
import { KnobAction } from '@/libs/nativeComponents/KnobAction';
import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { Text } from '@/libs/nativeComponents/Text';
import { A1, A2, A3, A4, B1, B2, B3, B4, C1, C2, C3, C4, ScreenWidth } from '../constants';
import { enc1, enc4 } from '../constantsValue';
import { Layout } from './Layout';

export type Props = {
    name: string;
    track: number;
    synthName: string;
    color: string;
    title: string;
};

export function Keyboard({ name, track, synthName, color, title }: Props) {
    return (
        <Layout
            viewName={name}
            color={color}
            synthName={synthName}
            title={title}
            skipMainKeys
            content={
                <>
                    <KnobValue
                        audioPlugin="Sequencer"
                        param="PLAYING_LOOPS"
                        {...enc1}
                        color="primary"
                        track={track}
                    />

                    <Text fontSize={12} text="Turn right knob to exit and save." bounds={[120, 4, ScreenWidth, 16]} font="PoppinsLight_12" color={'#8f8f8fff'} />
                    <Text fontSize={12} text="&icon::arrowDown::filled" bounds={[270, 25, 16, 16]} font="PoppinsLight_12" color={'#8f8f8fff'} />

                    <Text fontSize={32} text="OK" bounds={enc4.bounds} font="PoppinsLight_16" centered />
                    <KnobAction action={`data:Sequencer:${track}:SAVE_RECORD`} action2={`setView:&previous`} encoderId={enc4.encoderId} />

                    <HiddenValue
                        keys={[
                            { key: A1, action: `record:Sequencer:67:${track}` },
                            { key: A2, action: `record:Sequencer:69:${track}` },
                            { key: A3, action: `record:Sequencer:71:${track}` },
                            { key: A4, action: `record:Sequencer:72:${track}` },

                            { key: B1, action: `record:Sequencer:60:${track}` },
                            { key: B2, action: `record:Sequencer:62:${track}` },
                            { key: B3, action: `record:Sequencer:64:${track}` },
                            { key: B4, action: `record:Sequencer:65:${track}` },

                            { key: C1, action: `record:Sequencer:53:${track}` },
                            { key: C2, action: `record:Sequencer:55:${track}` },
                            { key: C3, action: `record:Sequencer:56:${track}` },
                            { key: C4, action: `record:Sequencer:58:${track}` },
                        ]}
                    />
                </>
            }
        />
    );
}
