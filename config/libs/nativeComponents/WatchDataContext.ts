import { getJsonComponent } from '../ui';

export interface DataContext {
    dataId: string;
    contextIndex: number;
}

export const WatchDataContext = (
    props: {
        audioPlugin: string;
        track: number;
        data: DataContext[];
    }
) => getJsonComponent('WatchDataContext')({ ...props, bounds: [0, 0, 0, 0] });
