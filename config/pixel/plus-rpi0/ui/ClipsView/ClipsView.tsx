import * as React from '@/libs/react';

import { HiddenContext } from '@/libs/nativeComponents/HiddenContext';
import { View } from '@/libs/nativeComponents/View';
import {
    btn1,
    btn2,
    btn3,
    btn4,
    btn5,
    ColorTrack1,
    ColorTrack2,
    ColorTrack3,
    ColorTrack4,
    ColorTrack5,
    Drum1Track,
    Rack1Track,
    Rack2Track,
    Rack3Track,
    Rack4Track
} from '../constants';
import { Clips } from './Clips';

export type Props = {
    name: string;
};

export function ClipsView({ name }: Props) {
    return (
        <View name={name}>
            {/* Put ShiftedTextGrid first, so it will overtake clip key actions */}
            {/* <ShiftedTextGrid selectedBackground={ColorTrackMaster} selected="Clips" /> */}

            <Clips color={ColorTrack1} track={Drum1Track} title="T.1" key={btn1} />
            <Clips color={ColorTrack2} track={Rack1Track} title="T.2" key={btn2} />
            <Clips color={ColorTrack3} track={Rack2Track} title="T.3" key={btn3} />
            <Clips color={ColorTrack4} track={Rack3Track} title="T.4" key={btn4} />
            <Clips color={ColorTrack5} track={Rack4Track} title="T.5" key={btn5} />
            <HiddenContext encoderId={0} contextId={30} min={0} max={16} inverted />

            {/* <TextGridClips /> */}
        </View>
    );
}
