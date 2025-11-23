/** Description:
## Analysis of Audio Track Identification Constants

This small, foundational piece of code serves as a crucial dictionary for an audio or music software application. Its purpose is to assign permanent, easy-to-read names to the unique identification numbers (IDs) of various sound channels, commonly referred to as "tracks."

### Basic Functionality

In programming, relying solely on raw numbers can lead to confusion (e.g., what does the number '5' represent?). This code solves that problem by using **constants**. A constant is a fixed value that never changes once defined. This allows the rest of the program to reference meaningful names, improving clarity and preventing errors.

Instead of a developer having to remember that the main output channel is number 0, they can simply use the name `MasterTrack`.

### Track Structure

The assigned numerical IDs are organized into clear, logical groups, setting the basic infrastructure for the sound mixing engine:

1.  **Master Track (ID 0):** This is the primary channel responsible for the overall volume and output of all sounds combined.
2.  **Drum Tracks (IDs 1–4):** Four dedicated channels are reserved for different percussion elements, allowing fine control over rhythm sections.
3.  **Synthesizer Tracks (IDs 5–7):** Three channels are allocated for computer-generated tones and instrumental parts.
4.  **Sample Tracks (IDs 8–10):** Three channels are used specifically for playing short, pre-recorded audio snippets or sound effects.

By providing these fixed numerical identifiers paired with descriptive names, the code ensures that the structure of the audio project is consistent, reliable, and straightforward for any programmer working on the system.

sha: a51fd023754f880e4628668b986939d94835a5b4db2626cc056829788731d6c9 
*/
export const MasterTrack = 0;
export const Drum1Track = 1;
export const Drum2Track = 2;
export const Drum3Track = 3;
export const Drum4Track = 4;

export const Synth1Track = 5;
export const Synth2Track = 6;
export const Synth3Track = 7;

export const Sample1Track = 8;
export const Sample2Track = 9;
export const Sample3Track = 10;
