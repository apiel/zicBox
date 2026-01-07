/** Description:
This TypeScript code defines a standardized configuration template named "Preset." This structure acts as a blueprint for saving and loading specific sets of operational and aesthetic choices within an application.

### 1. Purpose of the Preset

The `Preset` template ensures that whenever a user saves their settings—for instance, customizing a view or configuring an audio effect—the data saved is always organized consistently. It prevents errors by guaranteeing that all necessary pieces of information are present and correctly formatted.

### 2. The Core Mechanism

The code uses a specialized tool called `getJsonComponent`. This function likely handles reading external configuration files (often structured data like JSON) and validates that the loaded content perfectly matches the required structure defined in this file. By using this tool, the system ensures that only valid configuration data can be loaded as a `Preset`.

### 3. What the Preset Contains

The `Preset` template specifies various settings, categorized into mandatory and optional fields:

*   **Mandatory Setting:**
    *   `audioPlugin`: Required text specifying which audio device or software component must be used.
*   **Optional Settings (Used for customization):**
    *   Styling parameters, such as `bgColor` (background color), `textColor`, and `scrollColor`.
    *   Font settings, including `fontSize` (a number) and the `font` name (text).
    *   Structural details, like the `folder` where the preset is stored.
    *   Technical identifiers, like `encoderId`, which helps the program correctly route the audio data.

In summary, this code creates a public, reusable definition for a configuration template, ensuring that every saved user setting is complete, structured, and ready to be loaded correctly by the application.

sha: eb724d89546855fcad5b11e90400c7307da23aaa890bc405e0e98ff7e0890dd8 
*/
import { getJsonComponent } from '../ui';

export const Preset = getJsonComponent<{
    audioPlugin: string;
    folder?: string;
    bgColor?: string;
    textColor?: string;
    foregroundColor?: string;
    scrollColor?: string;
    fontSize?: number;
    font?: string;
    encoderId?: number;
    visibleItems?: number;
}>('Preset');
