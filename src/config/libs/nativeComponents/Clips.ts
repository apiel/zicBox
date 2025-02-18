import { getJsonComponent } from '../ui';

export const Clips = getJsonComponent<{
  groupAll?: number;
  bgColor?: string;
  textColor?: string;
  foregroundColor?: string;
  color?: string;
  visibleCount?: number;
  sequencerPlugin?: string;
  serializerPlugin?: string;
}>('Clips', 'libzic_ClipsComponent.so');
