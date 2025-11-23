/** Description:
This TypeScript code defines a standardized tool, or configuration template, named **SavePreset**. Its primary purpose is to ensure that whenever a user saves a custom configuration—like settings for a sound effect or instrument—the data is stored in a consistent and predictable format.

### How It Works (The Basic Idea)

The code uses a special utility function called `getJsonComponent`. You can think of this function as a template generator. It is instructed to create a component named 'SavePreset' based on a strict set of rules, or a "blueprint," that defines exactly what information must be saved.

### The Configuration Blueprint

The blueprint specifies all the possible settings that can be included in a saved preset. These fall into three main categories:

1.  **Mandatory Identity:** It requires the type of software component being configured (`audioPlugin`).
2.  **Visual Customization (Optional):** Users can optionally save settings related to appearance, such as specific background and text colors (`bgColor`, `textColor`), font style, and font size.
3.  **Control Mapping (Optional):** It can store identifiers that link this preset to external physical controllers or encoders (`valueEncoderId`, `cursorEncoderId`).

By rigidly defining this structure, the application prevents saving incomplete or inconsistent data, ensuring that every saved configuration works reliably when loaded later.

sha: f480572aad73eaeda52361bb95f8fdcd578bdd0182838c4a08313638324599ed 
*/
import { getJsonComponent } from '../ui';

export const SavePreset = getJsonComponent<{
    audioPlugin: string;
    folder?: string;
    redirectView?: string;
    bgColor?: string;
    textColor?: string;
    foregroundColor?: string;
    fontSize?: number;
    font?: string;
    valueEncoderId?: number;
    cursorEncoderId?: number;
}>('SavePreset');
