export type Props = {
    text: string;
};

export function Log({ text }: Props) {
    return [{ print: text }];
}
