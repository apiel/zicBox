import * as React from '@/libs/react';

import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { Rect } from '@/libs/nativeComponents/Rect';
import { Text } from '@/libs/nativeComponents/Text';
import { WorkspaceKnob } from '@/libs/nativeComponents/WorkspaceKnob';
import { rgb } from '@/libs/ui';
import { Layout } from '../components/Layout';
import {
    C2,
    C3,
    C4,
    MasterTrack,
    menuTextColor,
    ScreenHeight,
    shutdownContext,
    W1_4,
    W2_4,
    W3_4,
} from '../constants';
import { enc1, enc3, enc4 } from '../constantsValue';

export type Props = {
    name: string;
};

export function MenuView({ name }: Props) {
    return (
        <Layout
            viewName={name}
            color={rgb(170, 170, 170)}
            title="Menu"
            content={
                <>
                    <WorkspaceKnob {...enc1} />
                    <KnobValue
                        audioPlugin="TrackFx"
                        param="VOLUME"
                        label="Master Vol."
                        {...enc3}
                        color="tertiary"
                        track={MasterTrack}
                    />

                    <KnobValue
                        audioPlugin="Tempo"
                        param="BPM"
                        {...enc4}
                        color="tertiary"
                        track={MasterTrack}
                    />

                    <Text
                        text="Load"
                        bounds={[0, ScreenHeight - 20, W1_4, 16]}
                        centered={true}
                        color={menuTextColor}
                    />
                    <Text
                        text="Exit"
                        bounds={[W1_4, ScreenHeight - 20, W1_4, 16]}
                        centered={true}
                        color={menuTextColor}
                        keys={[{ key: C2, action: 'setView:&previous' }]}
                    />
                    <Text
                        text="Shutdown"
                        bounds={[W2_4, ScreenHeight - 20, W1_4, 16]}
                        centered={true}
                        color={menuTextColor}
                        keys={[{ key: C3, action: `contextToggle:${shutdownContext}:1:0` }]}
                    />
                    <Rect
                        bounds={[W3_4, ScreenHeight - 20, W1_4, 16]}
                        color="background"
                        visibilityContext={[
                            {
                                condition: 'SHOW_WHEN',
                                index: shutdownContext,
                                value: 0,
                            },
                        ]}
                    />
                    <Text
                        text="YES"
                        bounds={[W3_4, ScreenHeight - 20, W1_4, 16]}
                        centered={true}
                        // color={menuTextColor}
                        bgColor="#ffacac"
                        visibilityContext={[
                            {
                                condition: 'SHOW_WHEN',
                                index: shutdownContext,
                                value: 1,
                            },
                        ]}
                        keys={[{ key: C4, action: `sh:halt -f` }]}
                    />
                </>
            }
        />
    );
}
