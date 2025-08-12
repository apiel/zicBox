import * as React from '@/libs/react';

import { ShiftLayout } from '../components/ShiftLayout';
import { Drum1Track } from '../constants';

export function Shift() {
    return <ShiftLayout track={Drum1Track} />;
}
