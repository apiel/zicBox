import { Adsr } from './libs/components/Adsr';
import { applyZic } from './libs/core';
import * as React from './libs/react';

function GroupContainer({ a }: { a: string }) {
    return [{ COMPONENT: 'GroupContainer' }, { A: a }];
}

function List({ b }: { b: string }) {
    return [{ COMPONENT: 'List' }, { B: b }];
}

function Item({ k }: { k: string }) {
    return [{ ITEM: k }];
}

function MyCustomComponent() {
    return (
        <>
            <List b={'bla'} />
            <List b={'blo'} />
        </>
    );
}

applyZic(
    <GroupContainer a={'test'}>
        <List b={'test123'} />
        <List b={'test456'}>
            <Item k={'yo'} />
        </List>
        <MyCustomComponent />
        <Adsr position="0 0" plugin="plugin" values="values" />
    </GroupContainer>
);
