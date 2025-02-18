declare module 'osc' {
    interface Argument {
        type: string;
        value: boolean | number | string | Uint8Array;
    }

    interface Message {
        address: string;
        args: any[];
    }

    export class UDPPort {
        constructor(options: {
            remoteAddress: string;
            remotePort: number;
            metadata: boolean;
        });
        open(): void;
        send(msg: Message): void;
    }
}
