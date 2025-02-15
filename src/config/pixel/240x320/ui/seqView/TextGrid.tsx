{
    /* progressbar */
}
import * as React from '@/libs/react';

import { Keymap } from '@/libs/nativeComponents/Keymap';
import { Keymaps } from '@/libs/nativeComponents/Keymaps';
import { TextGrid } from '@/libs/nativeComponents/TextGrid';
import { VisibilityContext } from '@/libs/nativeComponents/VisibilityContext';
import { VisibilityGroup } from '@/libs/nativeComponents/VisibilityGroup';
import { KeyInfoPosition } from '@/pixel/240x320/ui/constants';

export function TextGridProgressBar() {
    return (
        <TextGrid
            bounds={KeyInfoPosition}
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
    );
}

export function TextGridProgressBarShifted() {
    return (
        <TextGrid
            bounds={KeyInfoPosition}
            rows={['Menu &icon::play::filled ^...', 'Clips &icon::tape Save']}
        >
            <VisibilityGroup condition="SHOW_WHEN" group={0} />
            <VisibilityContext index={254} condition="SHOW_WHEN" value={1} />
            <Keymaps
                keys={[
                    { key: 'q', action: 'setView:Menu', action2: 'contextToggle:254:1:0' },
                    { key: 'w', action: 'playPause' },
                    { key: 'a', action: 'setView:Clips' },
                    { key: 'e', action: 'contextToggle:254:1:0' },
                    { key: 's', action: 'setView:Tape', action2: 'contextToggle:254:1:0' },
                ]}
            />
        </TextGrid>
    );
}

export function TextGridTrack() {
    return (
        <TextGrid
            bounds={KeyInfoPosition}
            rows={[
                '&icon::musicNote::pixelated &icon::arrowUp::filled ...',
                '&icon::arrowLeft::filled &icon::arrowDown::filled &icon::arrowRight::filled',
            ]}
        >
            <VisibilityGroup condition="SHOW_WHEN_NOT" group={0} />
            <VisibilityContext index={254} condition="SHOW_WHEN" value={0} />
            <VisibilityContext index={10} condition="SHOW_WHEN" value={0} />
            <Keymaps keys={[{ key: 'e', action: 'contextToggle:254:1:0' }]} />
        </TextGrid>
    );
}

export function TextGridTrackStepEdit() {
    return (
        <TextGrid
            bounds={KeyInfoPosition}
            rows={[
                '&icon::toggle::rect &icon::arrowUp::filled ...',
                '&icon::arrowLeft::filled &icon::arrowDown::filled &icon::arrowRight::filled',
            ]}
        >
            <VisibilityGroup condition="SHOW_WHEN_NOT" group={0} />
            <VisibilityContext index={254} condition="SHOW_WHEN" value={0} />
            <VisibilityContext index={10} condition="SHOW_WHEN_OVER" value={0} />
            <Keymaps keys={[{ key: 'e', action: 'contextToggle:254:1:0' }]} />
        </TextGrid>
    );
}

export function TextGridTrackShifted() {
    return (
        <TextGrid
            bounds={KeyInfoPosition}
            rows={['&icon::toggle::rect &icon::play::filled ^...', 'Clips &icon::tape Master']}
        >
            <VisibilityGroup condition="SHOW_WHEN_NOT" group={0} />
            <VisibilityContext index={254} condition="SHOW_WHEN" value={1} />
            <Keymaps
                keys={[
                    { key: 'w', action: 'playPause' },
                    { key: 'e', action: 'contextToggle:254:1:0' },
                    { key: 'a', action: 'setView:Clips' },
                    { key: 'd', action: 'setGroup:0' },
                ]}
            />
        </TextGrid>
    );
}
