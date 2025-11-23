/** Description:
This specialized piece of code acts as a standardized preparation utility for handling simple text messages, often used in systems that require highly structured input, such as logging tools or action pipelines.

### Structure and Purpose

The code defines two primary components:

1.  **The Input Rule:** This sets a strict requirement for the data the utility accepts. It ensures that any input must be an object containing exactly one piece of information: a string (a simple message or sentence) labeled `text`.

2.  **The Packaging Function (`Log`):** This function processes the incoming message based on the rule.

### How It Works

Imagine you have a single letter you need to send through a very specific mail system.

1.  **Receiving the Letter:** The system first checks that the input adheres to the rule—it must be just the `text` of the letter.
2.  **Standardizing the Container:** The function takes this raw text and places it inside a specially labeled internal container. This container is tagged with the label `print`.
3.  **Final Delivery Format:** Finally, it takes this single container and puts it inside a list (an array) before returning it.

In summary, this utility receives a simple message, wraps it in a standardized structure (labeled `print`), and returns it as the single item in a list. This guarantees that the message is formatted correctly for whatever system is next in line to process or display the output.

sha: 9a56eac58bac95c4ca0c9d13109d4fab4157aeab51638d1e1ee3e93b00b0fe5d 
*/
export type Props = {
    text: string;
};

export function Log({ text }: Props) {
    return [{ print: text }];
}
