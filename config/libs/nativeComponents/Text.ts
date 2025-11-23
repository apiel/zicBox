/** Description:
This piece of code serves as a blueprint for creating a highly standardized and reusable text display element within a larger application or user interface framework.

### Overview

The primary function of this code is to define a visual component simply named `Text`. Instead of building this component from scratch, the code utilizes a specialized helper function, `getJsonComponent`, which acts like a "factory." This factory takes care of the complicated technical setup, allowing developers to quickly integrate text components using a consistent, predefined structure.

### Component Configuration

The most important part of this definition is the set of rules—or configuration options—it establishes for the `Text` component. These options dictate exactly what information can be provided when a developer wants to use this element:

1.  **Content (`text`):** This is mandatory and specifies the actual words to be displayed.
2.  **Appearance:** Optional settings control the look of the text:
    *   Font size and height.
    *   The color of the text (`color`).
    *   The color of the background area behind the text (`bgColor`).
    *   The specific font family to use (`font`).
3.  **Positioning:** Optional controls determine how the text is aligned:
    *   Whether the text should be horizontally centered (`centered`).
    *   Whether the text should be aligned to the right margin (`right`).

By defining these rules upfront, the system ensures that every `Text` component created throughout the application is built consistently and only accepts valid customization options.

sha: 379c0600c53d5a91b90312557b8b61940071e9f172e901fe11db5d2239875a0b 
*/
import { getJsonComponent } from '../ui';

export const Text = getJsonComponent<{
    text: string;
    fontSize?: number;
    fontHeight?: number;
    color?: string;
    bgColor?: string;
    centered?: boolean;
    right?: boolean;
    font?: string;
}>('Text');
