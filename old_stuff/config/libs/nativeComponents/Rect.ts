/** Description:
This script serves as a definition step, introducing a reusable visual element into the application's user interface: the Rectangle component, named `Rect`.

The core function of this code is to leverage a specialized utility that streamlines the creation of screen elements. Instead of manually writing all the complex drawing instructions, the system uses a helper function that automatically registers the component based on its name (`'Rect'`).

When other parts of the application need to display a rectangle, they reference this definition. Crucially, this component is customizable. The definition specifies two optional settings, or "properties," that control its appearance:

1.  **`color`:** Allows users to set the color of the rectangle.
2.  **`filled`:** A simple on/off setting that determines if the rectangle should be solid (filled in) or just an outline.

In essence, this small piece of code prepares a standardized, customizable rectangle tool that the software can easily draw onto the screen wherever needed.

sha: aa09715e1edf238e62e9b2034456204ebc4e90af67a8c8f64bdd4009e28495f7 
*/
import { getJsonComponent } from '../ui';

export const Rect = getJsonComponent<{
    color?: string;
    filled?: boolean;
}>('Rect');