import * as React from '@/libs/react';

import { Keymap } from '@/libs/components/Keymap';
import { Keymaps } from '@/libs/components/Keymaps';
import { TextGrid } from '@/libs/components/TextGrid';
import { Value } from '@/libs/components/Value';
import { View } from '@/libs/components/View';
import { VisibilityContext } from '@/libs/components/VisibilityContext';
import { VisibilityGroup } from '@/libs/components/VisibilityGroup';
import { KeyInfoPosition, W1_4, W3_4 } from '@/pixel/240x320/ui/constants';

export type Props = {
    name: string;
};

export function SeqView({ name }: Props) {
    return (
        <View name={name}>
            {/* progressbar */}
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

            {/* progressbar shifted */}
            <TextGrid
                position={KeyInfoPosition}
                rows={['Menu &icon::play::filled ^...', 'Clips ? Save']}
            >
                <VisibilityGroup condition="SHOW_WHEN" group={0} />
                <VisibilityContext index={254} condition="SHOW_WHEN" value={1} />
                <Keymaps
                    keys={[
                        { key: 'q', action: 'setView:Menu', action2: 'contextToggle:254:1:0' },
                        { key: 'w', action: 'playPause' },
                        { key: 'a', action: 'setView:Clips' },
                        { key: 'e', action: 'contextToggle:254:1:0' },
                    ]}
                />
            </TextGrid>

            <Value
                value="Tempo BPM"
                position={[W3_4, KeyInfoPosition[1], W1_4, 22]}
                SHOW_LABEL_OVER_VALUE={0}
                BAR_HEIGHT={0}
                VALUE_FONT_SIZE={16}
            />
        </View>
    );
}
