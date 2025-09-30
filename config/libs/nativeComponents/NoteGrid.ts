import { getJsonComponent } from '../ui';

export const NoteGrid = getJsonComponent<{
    audioPlugin?: string;
    cols?: number;
    rows?: number;
    scaleIndex?: number;
    octave?: number;
    encScale?: number;
    encOctave?: number;
    inverted?: boolean;
}>('NoteGrid');
