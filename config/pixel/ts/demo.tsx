import * as React from './libs/react';

function GroupContainer({ a }: { a: string }) {
    return [
        'COMPONENT: GroupContainer',
        `A: ${a}`,
    ]
}

function List({ b }: { b: string }) {
    return [
        'COMPONENT: List',
        `B: ${b}`,
    ]
}

function MyCustomComponent() {
    return (
        <>
            <List b={'bla'} />
            <List b={'blo'} />
        </>
    );
}

const elements = (
    <GroupContainer a={'test'}>
        <List b={'test123'} />
        <List b={'test456'} />
        <MyCustomComponent />
    </GroupContainer>
);
console.log('end:');
for (const element of elements) {
    console.log('- ', element);
}
