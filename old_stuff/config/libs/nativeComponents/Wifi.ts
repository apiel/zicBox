/** Description:
This TypeScript code defines a standardized configuration blueprint for a graphical user interface element specifically dedicated to "Wifi" settings.

### Core Functionality

This file utilizes a specialized helper function (`getJsonComponent`) to create and export a constant named `Wifi`. This constant acts as a ready-to-use definition for a component within a larger application framework, typically used in systems that generate UI elements based on structured data (like JSON).

In simple terms, this code establishes the *contract* for the Wifi component. It tells the system exactly what properties or settings are allowed when this component is used.

### Configurable Settings

The blueprint includes various optional settings to customize the component’s appearance and behavior:

1.  **Appearance:** Settings like `bgColor`, `textColor`, and `foregroundColor` control the visual scheme. Options for `fontSize` and `font` manage typography.
2.  **Security/Input:** The `masked` setting determines if sensitive data (like a password) should be hidden.
3.  **Data Handling:** Several ID fields (`ssidEncoderId`, `passwordEncoderId`) ensure that network names and passwords are processed, encoded, or stored using the correct system-specific security protocols.

By structuring the component this way, the application ensures consistency and reliability whenever a configurable Wifi input field needs to be displayed.

sha: 07eac6135aea2a467d3c9e9fde7b0b6f3e10a05be29684797b167bd7a4b863fe 
*/
import { getJsonComponent } from '../ui';

export const Wifi = getJsonComponent<{
    bgColor?: string;
    textColor?: string;
    foregroundColor?: string;
    activeColor?: string;
    fontSize?: number;
    font?: string;
    masked?: boolean;
    ssidEncoderId?: number;
    passwordEncoderId?: number;
    cursorEncoderId?: number;
    contextId?: number;
    default?: string;
}>('Wifi');
