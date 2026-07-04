import * as React from '@/libs/react';

import { VisibilityContext } from '@/libs/nativeComponents/component';
import { Rect } from '@/libs/nativeComponents/Rect';
import { Value } from '@/libs/nativeComponents/Value';
import {
    Drum1,
    Drum2,
    Drum3,
    Drum4,
    Sample1,
    Sample2,
    Sample3,
    Synth1,
    Synth2,
    Synth3,
} from '../components/Common';
import { KeysTracks } from '../components/KeysTracks';
import { Layout } from '../components/Layout';
import {
    ColorTrack1,
    ColorTrack10,
    ColorTrack2,
    ColorTrack3,
    ColorTrack4,
    ColorTrack5,
    ColorTrack6,
    ColorTrack7,
    ColorTrack8,
    ColorTrack9,
    ColorTrackMaster,
    Drum1Track,
    Drum2Track,
    Drum3Track,
    Drum4Track,
    MasterTrack,
    Sample1Track,
    Sample2Track,
    Sample3Track,
    shiftContext,
    Synth1Track,
    Synth2Track,
    Synth3Track,
} from '../constants';
import {
    backgroundBounds,
    enc1,
    enc10,
    enc11,
    enc12,
    enc2,
    enc3,
    enc4,
    enc5,
    enc6,
    enc7,
    enc8,
    enc9,
} from '../constantsValue';
import { Shift } from './Shift';

export function MainView({ name }: { name: string }) {
    const visibilityContext: VisibilityContext = {
        condition: 'SHOW_WHEN',
        index: shiftContext,
        value: 0,
    };

    const valueParam = (enc: { encoderId: number; bounds: number[] }) => ({
        barHeight: 4,
        alignLeft: true,
        showLabelOverValue: 10,
        valueSize: 24,
        visibilityContext: [visibilityContext],
        ...enc,
        bounds: [enc.bounds[0], enc.bounds[1] + 30, enc.bounds[2], enc.bounds[3] - 30],
    });

    return (
        <Layout
            viewName={name}
            color={ColorTrackMaster}
            title="Master"
            content={
                <>
                    <Shift / >

                    <Rect
                        bounds={backgroundBounds}
                        color="background"
                        visibilityContext={[visibilityContext]}
                    />
                  
                    <Value
                        {...valueParam(enc1)}
                        audioPlugin="TrackFx"
                        param="VOLUME"
                        label={`Mix ${Drum1}`}
                        track={Drum1Track}
                        barColor={ColorTrack1}
                    />
                
                    <Value
                        {...valueParam(enc2)}
                        audioPlugin="TrackFx"
                        param="VOLUME"
                        label={`Mix ${Drum2}`}
                        track={Drum2Track}
                        barColor={ColorTrack2}
                    />

                    <Value
                        {...valueParam(enc3)}
                        audioPlugin="TrackFx"
                        param="VOLUME"
                        label={`Mix ${Drum3}`}
                        track={Drum3Track}
                        barColor={ColorTrack3}
                    />

                    <Value
                        {...valueParam(enc4)}
                        audioPlugin="TrackFx"
                        param="VOLUME"
                        label={`Mix ${Drum4}`}
                        track={Drum4Track}
                        barColor={ColorTrack4}
                    />

                    <Value
                        {...valueParam(enc5)}
                        audioPlugin="TrackFx2"
                        param="VOLUME"
                        label={`Mix ${Synth1}`}
                        track={Synth1Track}
                        barColor={ColorTrack5}
                    />

                    <Value
                        {...valueParam(enc6)}
                        audioPlugin="TrackFx2"
                        param="VOLUME"
                        label={`Mix ${Synth2}`}
                        track={Synth2Track}
                        barColor={ColorTrack6}
                    />

                    <Value
                        {...valueParam(enc7)}
                        audioPlugin="TrackFx2"
                        param="VOLUME"
                        label={`Mix ${Synth3}`}
                        track={Synth3Track}
                        barColor={ColorTrack7}
                    />

                    <Value
                        {...valueParam(enc8)}
                        audioPlugin="TrackFx2"
                        param="VOLUME"
                        label={`Mix ${Sample1}`}
                        track={Sample1Track}
                        barColor={ColorTrack8}
                    />

                    <Value
                        {...valueParam(enc9)}
                        audioPlugin="TrackFx2"
                        param="VOLUME"
                        label={`Mix ${Sample2}`}
                        track={Sample2Track}
                        barColor={ColorTrack9}
                    />

                    <Value
                        {...valueParam(enc10)}
                        audioPlugin="TrackFx2"
                        param="VOLUME"
                        label={`Mix ${Sample3}`}
                        track={Sample3Track}
                        barColor={ColorTrack10}
                    />

                    <Value
                        {...valueParam(enc11)}
                        audioPlugin="TrackFx"
                        param="FX_AMOUNT"
                        track={MasterTrack}
                    />

                    <Value
                        {...valueParam(enc12)}
                        audioPlugin="TrackFx"
                        param="FX_TYPE"
                        track={MasterTrack}
                    />

                    <KeysTracks viewName={name} />
                </>
            }
        />
    );
}
