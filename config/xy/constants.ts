/** Description:
This code defines a simple, structured list of numerical constants (fixed values) that act as an indexing system for different functional components, typically referred to as "tracks" in an application like an audio editor or sequencer.

### Basic Idea

The purpose of this file is to create a universally understood map across the entire program. Instead of having to hardcode and remember that the number `3` always refers to "Track 3," the program uses the descriptive name, `Track3`. This list simply defines the connection: `Track3` equals the number 3.

This approach makes the rest of the application much cleaner and far less prone to errors.

### Structure and Meaning

The list establishes a clear, sequential numbering scheme:

1.  **Master Control:** The `MasterTrack` is assigned the starting index, zero (0). This usually represents the main output or overall mixing channel.
2.  **Standard Channels:** Six primary audio or data channels are defined, ranging from `Track1` (1) up to `Track6` (6).
3.  **Special Channels:** The last two entries, `Sample1Track` (7) and `Sample2Track` (8), reserve specific numerical slots for dedicated functions, likely for processing sound samples or special effects separately from the main tracks.

By declaring these indices as exports, any other part of the software project can easily access these precise numerical codes using their descriptive names.

sha: 56a23af331763f64cdac67f8c5e853a2adbf7ae68a2dec5da7610e424df7d756 
*/
export const MasterTrack = 0;
export const Track1 = 1;
export const Track2 = 2;
export const Track3 = 3;
export const Track4 = 4;
export const Track5 = 5;
export const Track6 = 6;

export const Sample1Track = 7;
export const Sample2Track = 8;
