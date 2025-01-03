import { applyZic, ZicValue } from '../libs/core';
import * as React from '../libs/react';
import { getView } from '../libs/ui';

export interface Props extends ZicValue {
    name: string;
}

function View({ name, ...props }: Props) {
    return getView(name, [props]);
}

applyZic(<View name={'test'}></View>);
