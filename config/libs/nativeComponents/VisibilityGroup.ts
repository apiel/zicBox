/** Description:
### Analysis of the Visibility Group Configuration Utility

This TypeScript code defines a small but critical utility responsible for generating standardized rules related to showing or hiding elements within a user interface. It ensures that complex visibility logic is defined uniformly, making it easier for a larger system to interpret and apply these rules.

#### 1. Core Purpose

This code acts as a configuration template. It does not control visibility directly; instead, it generates a standardized "instruction set" that tells the main application framework *how* and *when* to show a specific group of elements.

#### 2. Required Inputs (Props)

The utility requires two key pieces of information to create a valid visibility rule:

*   **Condition:** This is a fixed text string defining the type of visibility check. The code limits this input to four specific predefined rules, such as `SHOW_WHEN_OVER` (displaying content only if a value exceeds a limit) or `SHOW_WHEN_NOT` (displaying content only when a specific flag is absent).
*   **Group:** This is a numerical identifier. It tells the system which specific collection or set of elements this new visibility rule should be applied to.

#### 3. How the Function Works

The core function, `VisibilityGroup`, takes the chosen **Condition** and the **Group** number and combines them into a single, cohesive instruction.

For example, if the inputs were the condition `SHOW_WHEN_OVER` and the group number `5`, the function combines them into a formatted text string like "SHOW\_WHEN\_OVER 5."

This string is then placed inside a designated container labeled `VISIBILITY_GROUP`. This standardized packaging ensures that the rest of the application knows exactly where to look for the visibility instructions.

#### 4. Role in the Application

The output of this utility (the structured instruction) is never displayed to the end-user. It is immediately consumed by the application's rendering engine. The engine reads the `VISIBILITY_GROUP` instruction and applies the correct filtering logic, ensuring that only the elements matching the defined rule are visible on the screen. This system simplifies complex dynamic visibility management across large applications.

sha: de9d73c365f93df0b6ab70b85b1785fa4b406598ffb1e38b239027196f40bdb3 
*/

export type Props = {
    condition: 'SHOW_WHEN_NOT' | 'SHOW_WHEN_OVER' | 'SHOW_WHEN_UNDER' | 'SHOW_WHEN';
    group: number;
};

export function VisibilityGroup({ condition, group }: Props) {
    return [{ VISIBILITY_GROUP: `${condition} ${group}` }];
}
