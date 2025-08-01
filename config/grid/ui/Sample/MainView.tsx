import * as React from '@/libs/react';

import { KeysTracks } from '../components/KeysTracks';
import { Layout } from '../components/Layout';

export type Props = {
    name: string;
    track: number;
    synthName: string;
    color: string;
    title: string;

};

export function MainView({ name, track, synthName, color, title }: Props) {
    return (
        <Layout
            viewName={name}
            color={color}
            synthName={synthName}
            title={title}
            content={
                <>
                    <KeysTracks synthName={synthName} viewName={name} />
                </>
            }
        />
    );
}
