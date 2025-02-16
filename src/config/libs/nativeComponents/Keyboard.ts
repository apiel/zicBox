// import { getOldComponentToBeDeprecated, initializePlugin } from '@/libs/ui';
// import { ComponentProps } from './component';

// export type Props = ComponentProps & {
// };

// export function Keyboard({ bounds, items, ...props }: Props) {
//     initializePlugin('Keyboard', 'libzic_KeyboardComponent.so');
//     return getOldComponentToBeDeprecated('Keyboard', bounds, [props]);
// }


import { getJsonComponent } from '../ui';

export const Keyboard = getJsonComponent<{
    redirectView?: string;
    audioPlugin?: string;
    dataId?: string;
    bgColor?: string;
    textColor?: string;
    selectionColor?: string;
    itemBackground?: string;
}>('Keyboard', 'libzic_KeyboardComponent.so');

// <Keyboard
//   // The view to redirect once edit is finished.
//   redirectView="view_name"
//   // The audio plugin to send the input value.
//   audioPlugin="audio_plugin_name"
//   // The data id of the audio plugin where the input value will be sent.
//   dataId="data_id"
//   // Set the background color of the component.
//   bgColor="#000000"
//   // Set the color of the text.
//   textColor="#ffffff"
//   // Set the color of the selection.
//   selectionColor="#ffffff"
//   // Set the color of the item background.
//   itemBackground="#ffffff"
// />