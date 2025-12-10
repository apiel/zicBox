import * as React from '@/libs/react';

import { ResizeType } from '@/libs/nativeComponents/component';
import { GraphValue } from '@/libs/nativeComponents/GraphValue';

import { Icon } from '@/libs/nativeComponents/Icon';
import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { Val } from './components/Val';
import { MasterTrack } from './constants';
import {
    enc10mini,
    enc11mini,
    enc12mini,
    enc1mini,
    enc2mini,
    enc3mini,
    enc4mini,
    enc5mini,
    enc6mini,
    enc7mini,
    enc8mini,
    enc9mini,
    encAmini,
    encBmini,
    graphHeight,
    knob
} from './constantsValue';
import { Track } from './Track';

export type Props = {
    name: string;
    track: number;
    synthName: string;
    color: string;
    title: string;
};

const knobBg = '#3a3a3a';

const iconParams = {
    bgColor: '#575656ff',
}
const iconSize = [16, 16];

export function PatchEdit({ name, track, synthName, color, title }: Props) {
    const resizeType = ResizeType.RESIZE_W | ResizeType.RESIZE_X;
    const valProps = { track, audioPlugin: synthName, resizeType, bgColor: knobBg };
    const graphProps = (enc: { encoderId: number; bounds: number[] }) => {
        return {
            ...valProps,
            extendEncoderIdArea: enc.encoderId,
            bounds: [enc.bounds[0], enc.bounds[1] - 30, enc.bounds[2], graphHeight],
        };
    };
    return (
        <>

            <Icon name="&icon::backspace" bounds={[650, 130, ...iconSize]} {...iconParams} />
            <Icon name="&icon::backspace::filled" bounds={[690, 130, ...iconSize]} {...iconParams} />
            <Icon name="&icon::play" bounds={[730, 130, ...iconSize]} {...iconParams} />
            <Icon name="&icon::play::filled" bounds={[770, 130, ...iconSize]} {...iconParams} />
            <Icon name="&icon::stop" bounds={[810, 130, ...iconSize]} {...iconParams} />
            <Icon name="&icon::stop::filled" bounds={[850, 130, ...iconSize]} {...iconParams} />
            <Icon name="&icon::pause" bounds={[890, 130, ...iconSize]} {...iconParams} />
            <Icon name="&icon::pause::filled" bounds={[930, 130, ...iconSize]} {...iconParams} />
            
            <Icon name="&icon::arrowRight::filled" bounds={[650, 150, ...iconSize]} {...iconParams} />
            <Icon name="&icon::musicNote" bounds={[690, 150, ...iconSize]} {...iconParams} />
            <Icon name="&icon::musicNote::pixelated" bounds={[730, 150, ...iconSize]} {...iconParams} />
            <Icon name="&icon::tape" bounds={[770, 150, ...iconSize]} {...iconParams} />
            <Icon name="&icon::toggle" bounds={[810, 150, ...iconSize]} {...iconParams} />
            <Icon name="&icon::trash" bounds={[850, 150, ...iconSize]} {...iconParams} />
            <Icon name="&icon::arrowUp" bounds={[890, 150, ...iconSize]} {...iconParams} />
            <Icon name="&icon::arrowUp::filled" bounds={[930, 150, ...iconSize]} {...iconParams} />

            <Icon name="&icon::arrowDown" bounds={[650, 170, ...iconSize]} {...iconParams} />
            <Icon name="&icon::arrowDown::filled" bounds={[690, 170, ...iconSize]} {...iconParams} />
            <Icon name="&icon::arrowLeft" bounds={[730, 170, ...iconSize]} {...iconParams} />
            <Icon name="&icon::arrowLeft::filled" bounds={[770, 170, ...iconSize]} {...iconParams} />
            <Icon name="&icon::arrowRight" bounds={[810, 170, ...iconSize]} {...iconParams} /> 
            <Icon name="&icon::scrollHorizontal" bounds={[850, 170, ...iconSize]} {...iconParams} /> 
            <Icon name="&icon::scrollHorizontal::filled" bounds={[890, 170, ...iconSize]} {...iconParams} /> 
            


            <Track synthName={synthName} viewName={name} track={track} color={color} />
            <Val {...enc1mini} param="VAL_1" color={'secondary'} {...valProps} />

            <GraphValue
                param="VAL_2"
                outlineColor="quaternary"
                fillColor="#c2af6b"
                {...graphProps(enc2mini)}
            />
            <Val {...enc2mini} param="VAL_2" color={'quaternary'} {...valProps} />

            <GraphValue
                param="VAL_3"
                outlineColor="#399462"
                fillColor="#235e3e"
                {...graphProps(enc3mini)}
            />
            <Val {...enc3mini} param="VAL_3" color={'tertiary'} {...valProps} />

            <GraphValue
                param="VAL_4"
                outlineColor="primary"
                fillColor="#315c79"
                {...graphProps(enc4mini)}
            />
            <Val {...enc4mini} param="VAL_4" color={'primary'} {...valProps} />

            <GraphValue
                param="VAL_5"
                outlineColor="secondary"
                fillColor="#ad6565ff"
                {...graphProps(enc5mini)}
            />
            <Val {...enc5mini} param="VAL_5" color={'secondary'} {...valProps} />

            <GraphValue
                param="VAL_6"
                outlineColor="quaternary"
                fillColor="#c2af6b"
                {...graphProps(enc6mini)}
            />
            <Val {...enc6mini} param="VAL_6" color={'quaternary'} {...valProps} />

            <GraphValue
                param="VAL_7"
                outlineColor="tertiary"
                fillColor="#235e3e"
                {...graphProps(enc7mini)}
            />
            <Val {...enc7mini} param="VAL_7" color={'tertiary'} {...valProps} />

            <GraphValue
                param="VAL_8"
                outlineColor="primary"
                fillColor="#315c79"
                {...graphProps(enc8mini)}
            />
            <Val {...enc8mini} param="VAL_8" color={'primary'} {...valProps} />

            <GraphValue
                param="VAL_9"
                outlineColor="secondary"
                fillColor="#ad6565ff"
                {...graphProps(enc9mini)}
            />
            <Val {...enc9mini} param="VAL_9" color={'secondary'} {...valProps} />

            <GraphValue
                param="VAL_10"
                outlineColor="quaternary"
                fillColor="#c2af6b"
                {...graphProps(enc10mini)}
            />
            <Val {...enc10mini} param="VAL_10" color={'quaternary'} {...valProps} />

            <GraphValue
                param="VAL_11"
                outlineColor="tertiary"
                fillColor="#235e3e"
                {...graphProps(enc11mini)}
            />
            <Val {...enc11mini} param="VAL_11" color={'tertiary'} {...valProps} />

            <GraphValue
                param="VAL_12"
                outlineColor="primary"
                fillColor="#315c79"
                {...graphProps(enc12mini)}
            />
            <Val {...enc12mini} param="VAL_12" color={'primary'} {...valProps} />

            <KnobValue
                audioPlugin={synthName}
                param="ENGINE"
                {...knob(encAmini)}
                color="secondary"
                track={track}
                resizeType={resizeType}
                bgColor={knobBg}
            />

            <KnobValue
                {...knob(encBmini)}
                color="quaternary"
                audioPlugin="Mixer"
                param={`TRACK_${track}`}
                label="Track volume"
                track={MasterTrack}
                resizeType={resizeType}
                bgColor={knobBg}
            />

            {/* <KnobValue
                audioPlugin={synthName}
                param="ENGINE"
                {...knob(encCmini)}
                color="secondary"
                track={track}
                resizeType={resizeType}
                bgColor={knobBg}
            /> */}

            {/* <Icon name="&icon::play::filled" bounds={encCmini.bounds} centered={true} /> */}
        </>
    );
}
