import * as React from '@/libs/react';

import { ProgressBar } from './ProgressBar';
import { TextGridCommon } from './TextGridCommon';

export function Common({ selected, page }: { selected: number, page: string }) {
    return (
        <>
            <ProgressBar />
            <TextGridCommon selected={selected} page={page} />
        </>
    );
}
