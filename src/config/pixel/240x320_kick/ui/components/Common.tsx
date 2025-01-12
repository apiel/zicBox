import * as React from '@/libs/react';

import { ProgressBar } from './ProgressBar';
import { TextGridCommon } from './TextGridCommon';

export function Common({ selected }: { selected: number }) {
    return (
        <>
            <ProgressBar />
            <TextGridCommon selected={selected} />
        </>
    );
}
