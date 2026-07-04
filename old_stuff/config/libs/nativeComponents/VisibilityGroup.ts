/** Description:
This script functions as a specialized utility for controlling the visibility of elements within an application. It is essentially a tool for generating precise display rules, often used in systems where content dynamically appears or disappears based on certain states or user interactions.

The tool requires two specific inputs to create its rule:

1.  **The Condition:** This defines the exact logic for visibility. It must be one of four predefined rules, such as "show only when something is not true," or "show only when a numerical value is over a specific threshold."
2.  **The Group:** This is a simple identification number that links the specified rule to a specific category or collection of items on the screen.

**How It Works:**

The function takes the chosen Condition and the Group ID number and immediately combines them into a single, formatted instruction tag (e.g., "SHOW\_WHEN 5"). This tag is packaged and made available to the larger application. The core application then reads this instruction to govern the rendering process, ensuring that the content linked to that specific Group ID is only displayed when its required visibility Condition is perfectly met.

sha: d6bb6abcd1bf3c181328d1623158a6933d875de2c975dccd4b9b763c56e5487b 
*/
export type Props = {
    condition: 'SHOW_WHEN_NOT' | 'SHOW_WHEN_OVER' | 'SHOW_WHEN_UNDER' | 'SHOW_WHEN';
    group: number;
};

export function VisibilityGroup({ condition, group }: Props) {
    return [{ VISIBILITY_GROUP: `${condition} ${group}` }];
}
