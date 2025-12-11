/** Description:
This piece of TypeScript code serves to define and configure a standard, reusable visual element known as an "Icon" for a digital application.

### 1. The Core Purpose
The primary function of this code is to establish a modular, predictable building block for the user interface. It avoids writing complex visual code directly in this file by importing a utility function (`getJsonComponent`) that handles fetching the full definition of the Icon from a centralized configuration source (often a JSON file).

### 2. Defining the Rules (Component Properties)
Crucially, the code doesn't just create the component; it sets clear rules about how that component must be used. These rules ensure consistency and prevent errors:

*   **`name` (Required):** When using the Icon, you must provide a textual name (e.g., "star" or "home"). This tells the system exactly which graphical symbol to display.
*   **`color` (Optional):** Users can optionally specify a color for the Icon itself.
*   **`bgColor` (Optional):** Users can optionally specify a background color behind the Icon.

### 3. Summary
In essence, this file exports a highly flexible `Icon` component that is ready to use throughout the application. It guarantees that every instance of an Icon will conform to a consistent structure while allowing simple customizations for its color and background.

Tags: User Interface, Visual Elements, Graphic Symbols, Styling Attributes, Configuration Settings
sha: 648e0c97d06080159fc583f0ee5eec8a0ecd63b8c058ee104d95904a82ad5cd2 
*/
import { getJsonComponent } from '../ui';

export const Icon = getJsonComponent<{
    name: string;
    color?: string;
    bgColor?: string;
}>('Icon');
