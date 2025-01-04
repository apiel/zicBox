import { ZicObj } from '../../libs/core';
import { getView } from '../../libs/ui';

export interface Props extends ZicObj {
    name: string;
}

export function View({ name, ...props }: Props) {
    return getView(name, [props]);
}