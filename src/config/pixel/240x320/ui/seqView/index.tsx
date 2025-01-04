import * as React from '@/libs/react';

import { Keymap } from '@/libs/components/Keymap';
import { TextGrid } from '@/libs/components/TextGrid';
import { View } from '@/libs/components/View';
import { VisibilityContext } from '@/libs/components/VisibilityContext';
import { VisibilityGroup } from '@/libs/components/VisibilityGroup';
import { ZicObj } from '@/libs/core';
import { KeyInfoPosition } from '../constants';

export type Props = ZicObj & {
    name: string;
};

export function SeqView({ name, ...props }: Props) {
    return (
        <View name={name} {...props}>
            <TextGrid
                position={KeyInfoPosition}
                rows={[
                    '&icon::play::filled &icon::arrowUp::filled ...',
                    '&icon::arrowLeft::filled &icon::arrowDown::filled &icon::arrowRight::filled',
                ]}
            >
                <VisibilityGroup condition="SHOW_WHEN" group={0} />
                <VisibilityContext index={254} condition="SHOW_WHEN" value={0} />
                <Keymap key="q" action="playPause" />
                <Keymap key="e" action="contextToggle:254:1:0" />
            </TextGrid>

            <TextGrid
                position={KeyInfoPosition}
                rows={[
                    'Menu &icon::play::filled ^...',
                    'Clips ? Save',
                ]}
            >
                <VisibilityGroup condition="SHOW_WHEN" group={0} />
                <VisibilityContext index={254} condition="SHOW_WHEN" value={1} />
                <Keymap key="q" action="setView:Menu" action2="contextToggle:254:1:0" />
                <Keymap key="w" action="playPause" />
                <Keymap key="a" action="setView:Clips" />
                <Keymap key="e" action="contextToggle:254:1:0" />
            </TextGrid>
        </View>
    );
}
