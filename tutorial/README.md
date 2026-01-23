# 90 Music programming tutorial

In this tutorial, I will explain how to approach music programming based on my journey since I first started learning about it. My goal is to keep the content simple and approachable for most developers. Please note that I’m not an expert, so there’s a chance I might get a few things wrong. However, I’ll do my best to ensure the information is accurate and useful.

## Which programming language

> [!TIP]
> Use `C++`.
> 
> If you’re not interested in the details, no worries—just skip ahead to the next step and use C++.

First, let's speak about the programming language. Using **C or C++** for audio programming makes sense for several reasons. Audio programming often involves working with low-level data processing, high performance, and real-time constraints, all of which align well with the strengths of these languages.

1. **High Performance**: Audio programming requires processing large amounts of data in real time, often within milliseconds. C and C++ are compiled languages that produce highly optimized machine code, enabling developers to achieve the low latency and high throughput needed for audio applications.

2. **Low-Level Access**: Audio programming often involves direct interaction with hardware (like audio drivers) or precise control over memory and data structures. C and C++ provide low-level access to memory and system resources, which is critical for tasks like:

3. **Real-Time Capabilities**: Real-time audio processing requires deterministic behavior. C and C++ allow fine-tuned control over execution time, enabling developers to avoid unpredictable overhead caused by garbage collection or other runtime management features found in higher-level languages.

4. **Extensive Ecosystem**: The C/C++ ecosystem includes numerous libraries for DSP (Digital Signal Processing), audio synthesis, and interfacing with MIDI devices. Developers can leverage these libraries to save time and implement advanced functionality.

8. **Availability of Examples and Resources**: Most examples, tutorials, and resources for audio programming that you’ll find online are written in C or C++. Learning C++ ensures that you can follow along with these materials, understand code snippets, and take advantage of the wealth of knowledge already available in the field.

While other languages like Python, JavaScript, or Java may be suitable for prototyping or specific applications, C and C++ provide the performance, flexibility, and control required for professional-grade audio programming.

**Rust, Ada, and others** low level programming languages also offer unique strengths for audio programming, but they cater to specific needs and scenarios. **Rust** has gained popularity for systems programming due to its modern features, safety guarantees, and performance. **Ada** is a niche language often associated with safety-critical systems (e.g., aviation, military, medical). It has features that could benefit audio programming, but its usage is less common in this domain.

**Why C++ is Still Dominant**

Despite the benefits of languages like Rust or Ada, C++ remains the go-to for audio programming due to its maturity, extensive libraries, and widespread industry adoption.

**Conclusion**:

Rust and Ada can be viable options for audio programming under specific circumstances, but C++ continues to be the best choice for most developers due to its balance of performance, control, and ecosystem support. However, as Rust’s ecosystem grows, it may emerge as a stronger contender in the future.

## Fixed point vs floating point

> [!TIP]
> Use `float`.
> 
> If you’re not interested in the details, no worries—just skip ahead to the next step and use floating-point numbers. 🙂

Let’s begin with the tricky part. Many audio programming examples use the Q fixed-point format, which can be confusing for beginners. This is how I started, and it made understanding what was happening extremely difficult (an example using fixed point format is the famous [Teensy Audio Library]([link](https://github.com/PaulStoffregen/Audio))). For those just starting out in audio programming, I strongly recommend avoiding fixed-point formats and sticking to floating-point numbers instead. It’s far more intuitive and easier to work with.

Fixed-point is a way of representing numbers where a specific number of bits are reserved for the integer part and the rest for the fractional part. Unlike floating-point, which can represent a wide range of values with dynamic precision, fixed-point numbers have a fixed number of decimal places, making them more efficient for certain applications, especially in hardware with limited processing power. For example, in Q31 format, 31 bits are used for the fractional part, and 1 bit is for the sign, allowing for precise representation of values between -1 and 1.

> For more details about fixed-point arithmetic, check out the **Wikipedia** page on the **Q number format**. It provides a clear and detailed explanation: [Q (number format)](https://en.wikipedia.org/wiki/Q_(number_format))

Today, with modern CPUs and MCUs that have good support for floating-point operations, using floating-point numbers (like float or double) often makes more sense than using fixed-point. Here's why:

1. **Hardware Support for Floating-Point Operations**: Many recent CPUs and microcontrollers come with dedicated hardware for floating-point arithmetic (FPU - Floating Point Unit). This means floating-point operations are handled efficiently and quickly, often without any noticeable performance hit. This significantly reduces the need for fixed-point math, which was historically used to overcome the lack of hardware support.

2. **Simplicity and Readability**: Floating-point numbers are easier to work with because they allow you to express values in a more intuitive way (e.g., 0.5, 3.14159, etc.). This makes the code simpler and more readable, reducing the mental overhead required to manage fixed-point conversions or manually scaling values.

3. **Precision and Range**: Floating-point numbers have a much larger dynamic range and precision compared to fixed-point numbers. For most audio applications, especially when working with continuous values (like sound waves, audio effects, etc.), floating-point provides sufficient precision and avoids the need to carefully manage fixed-point limits or overflow issues.

4. **Development Speed**: With floating-point, you don't have to worry about the complications of manually converting between fixed-point and floating-point formats, scaling values, or dealing with precision loss. This can significantly speed up development, especially for beginners, and allow for quicker prototyping.

5. **Modern Tools and Libraries**: Many audio programming libraries and frameworks today, including those used in professional DAWs, synths, and embedded systems, are built with floating-point in mind. The availability of powerful and optimized floating-point libraries makes it easier to work with audio data without needing to dive into low-level fixed-point math.

**Conclusion**:

While fixed-point arithmetic still has its place in certain resource-constrained systems (especially with older MCUs or when extreme performance optimization is required), modern hardware’s efficient support for floating-point arithmetic makes float the more practical choice for most audio programming today. It allows developers to focus on the creative and algorithmic aspects of their work rather than worrying about low-level number representation.

## Let's make some noise

The easiest way to get started is by literally producing noise. Noise is simply a random value sent to the audio buffer. In this example—and all the examples that follow—we’ll use my audio library to send audio to the output buffer. The library is built on PulseAudio. I’m working on Linux, but I believe it should also be compatible with Windows and macOS (though I haven’t tested it yet).

> If it doesn’t work for you, here’s a friendly tip: switch to Linux! 😄

```cpp
// 01.cpp
#include "plugins/audio/AudioOutputPulse.h"

int main(int argc, char* argv[])
{
    // Initialize audio properties with default settings
    AudioPlugin::Props props = defaultAudioProps;

    // Create an AudioOutputPulse instance to handle audio output
    // The name "zicAudioOutputPulse" is passed as an identifier
    AudioOutputPulse audioOutput(props, (char*)"zicAudioOutputPulse");

    // Create an audio buffer to hold samples
    // Each sample corresponds to a track; initialize to silence (0.0f)
    float buffer[props.maxTracks] = { 0.0f };

    // Main loop to generate and play audio
    while (1) {
        // Assign a random float value to track 0
        // rand() generates a random integer, which is normalized to the range [0.0, 1.0]
        buffer[0] = rand() / (float)RAND_MAX;

        // Send the sample from track 0 to the audio output
        // By default, the audio output processes track 0
        audioOutput.sample(buffer);
    }

    return 0;
}

```

Code example are available here: https://github.com/apiel/zicBox/tree/main/plugins/audio/tutorial

```sh
git clone https://github.com/apiel/zicBox.git
cd plugins/audio/tutorial
```

Compile it with:
```sh
g++ 01.cpp -o 01.bin -I../../.. -lpulse-simple -lpulse && ./01.bin
```


1. **Audio Library Overview**: The code uses the AudioOutputPulse class from your audio library to interact with PulseAudio, a widely used sound server on Linux. This class handles the connection to the audio server and streams the audio data to the output device.

2. **Audio Buffer**: An audio buffer is an array used to store audio samples temporarily before they are sent to the audio output. In this example:
The buffer array has a size equal to the maximum number of tracks defined in props.maxTracks.
Each track in the buffer corresponds to a separate audio channel .

3. **Random Noise Generation**: To produce noise, a random value is generated using the rand() function, which outputs a pseudo-random integer. This value is then normalized to a floating-point number between 0.0 and 1.0 by dividing by RAND_MAX, the maximum possible value of rand().

4. **Real-Time Audio Output**: The audioOutput.sample(buffer) function sends the buffer to the audio output device. In this implementation: The function is called repeatedly in an infinite loop. Each iteration processes one audio frame (a single sample for each track). This ensures continuous real-time audio playback.

**Takeaways for Beginners**:

This example highlights the fundamental structure of real-time audio programming: prepare audio data, write it to a buffer, and send it to the audio output.
Starting with noise is a great way to understand how audio buffers and output work without dealing with complex audio synthesis or processing.
From here, you can experiment with generating other types of audio signals, such as sine waves, and explore how to expand to multiple tracks or stereo audio.

## Audio chunk

In the previous example, you can see that each sample is sent one at a time to the output. However, behind the scenes, `AudioOutputPulse` is accumulating these samples in an internal buffer. This internal buffer groups the samples into chunks, which are then sent to PulseAudio for more efficient processing.

Chunks are used in audio processing for efficiency and practicality. Here's why:

1. **Minimized Overhead**: Sending audio samples one at a time to the audio server (e.g., PulseAudio) would involve constant communication, causing significant overhead. This includes function calls, context switching, and potentially network or inter-process communication if the server is running separately. By grouping samples into chunks and sending them in batches, the number of communications is drastically reduced, improving efficiency.

2. **Compatibility with Audio Hardware**: Most audio hardware and APIs are designed to process audio data in blocks or frames rather than individual samples. These blocks are predefined sizes of audio data that align with the hardware’s internal processing or buffering capabilities. Chunks ensure that the data provided to the hardware matches its expected format.

3. **Smooth Playback**: Real-time audio playback requires a constant and uninterrupted flow of data. By buffering samples into chunks, the system has a reserve of audio data ready to be processed. This helps prevent playback issues like stuttering or glitches that could occur if the system momentarily runs out of samples.

In our example—and in most parts of zicBox—I handle audio processing sample by sample. This approach is much easier to understand and, in most cases, doesn’t make a significant difference in performance.

The only time it becomes important to use chunks is when generating audio across multiple threads. In such cases, synchronizing and communicating between threads for each individual sample is too costly. That’s where processing audio in chunks makes sense.

Multi-threaded audio generation is far away... For now, let’s stick to sample-by-sample processing for its simplicity!

As you might have noted, chunks should be used to enable efficient communication between two entities, such as threads, processes, or hardware. They reduce the overhead of transferring one sample at a time, facilitate synchronization, and balance latency with computational efficiency in real-time audio systems.

**Why Sample-by-Sample Processing is Simpler**:
1. **Straightforward Logic**: Processing audio one sample at a time aligns naturally with how sound is represented—a continuous stream of individual values. There’s no need to worry about managing multiple samples at once, so the logic is easier to follow and debug.

2. **Reduced Buffer Management**: Handling chunks requires maintaining intermediate buffers, ensuring they’re the correct size, and managing the flow of data between buffers. By working sample by sample, you avoid these complexities entirely.

3. **Great for Learning**: For beginners or simple projects, working with one sample at a time makes it easier to understand how audio data is processed and manipulated without introducing additional layers of complexity.

**Why Chunks are Necessary in Multi-Threaded Processing**
1. **High Cost of Per-Sample Synchronization**: In multi-threaded scenarios, threads need to communicate to ensure the generated audio is synchronized. Doing this for each individual sample introduces a lot of overhead, as thread synchronization mechanisms (e.g., mutexes, locks) are expensive to execute repeatedly.

2. **Batch Processing Reduces Overhead**: By working with chunks, threads can generate multiple samples at once and communicate only when an entire chunk is ready. This drastically reduces the number of synchronization events and improves efficiency.

**Why Start with Sample-by-Sample**

Before introducing the complexity of multi-threading and chunk management, it’s beneficial to master sample-by-sample processing. This approach builds a strong foundation, helping you understand the basics of audio synthesis, manipulation, and playback without worrying about advanced optimizations.

When you’re ready to scale up to multi-threading or more demanding performance scenarios, you’ll have the knowledge to adapt your processing to chunks efficiently.

In 99% of my personal projects, I stick to sample-by-sample processing. I only switch to chunk-based processing when it’s absolutely necessary.

## Sine wave

Here's a complete example for generating and playing a sine wave:

```cpp
// 02.cpp

#include "plugins/audio/AudioOutputPulse.h"
#include <cmath> // For sin()

#define TWO_PI 6.283185307179586

int main(int argc, char* argv[])
{
    // Initialize audio properties with default settings
    AudioPlugin::Props props = defaultAudioProps;

    // Create an AudioOutputPulse instance to handle audio output
    AudioOutputPulse audioOutput(props, (char*)"zicAudioOutputPulse");

    // Create an audio buffer to hold samples (one per track)
    float buffer[props.maxTracks] = { 0.0f };

    // Frequency of the sine wave (in Hz)
    float frequency = 440.0f; // Standard A4 note

    // Phase accumulator for the sine wave
    float phase = 0.0f;

    // Phase increment per sample
    float phaseIncrement = TWO_PI * frequency / props.sampleRate;

    // Main loop to generate and play the sine wave
    while (1) {
        // Calculate the sine value for the current phase
        buffer[0] = sin(phase);

        // Increment the phase
        phase += phaseIncrement;

        // Wrap the phase back into the range [0, TWO_PI]
        if (phase >= TWO_PI) {
            phase -= TWO_PI;
        }

        // Send the generated sample to the audio output
        audioOutput.sample(buffer);
    }

    return 0;
}
```

> [!WARNING]
> It generates a continuous, unpleasant beep sound.

Compile it with:
```sh
g++ 02.cpp -o 02.bin -I../../.. -lpulse-simple -lpulse && ./02.bin
```

1. **Frequency and Sample Rate**: The sine wave's frequency is set to 440 Hz (the pitch of the A4 musical note). The sampling rate is fetched from the audio properties (props.sampleRate).

2. **Phase and Phase Increment**: The phase represents the current position in the sine wave cycle. The phaseIncrement determines how much the phase progresses for each sample, based on the frequency and sampling rate. The formula TWO_PI * frequency / sampleRate ensures the correct frequency is achieved.
   
3. **Sine Wave Generation**: The sine value is calculated for the current phase using sin(phase) and assigned to buffer[0].

4. **Phase Wrapping**: After each sample, the phase is incremented. When the phase exceeds 2𝜋, it’s wrapped back into the range of [0,2𝜋].

5. **Output the Sample**: The generated sine wave sample is sent to the audio output using audioOutput.sample(buffer).

This example generates a pure sine wave at 440 Hz and outputs it continuously. You can adjust the frequency variable to produce different tones.

## Amplitude envelop

Let’s introduce a new function to generate an amplitude envelope. One of the simplest types is the ASR (Attack, Sustain, Release) envelope. To implement this, take a look at the following function:

```cpp
// Function to calculate the envelope value at a given sample index
float calculateEnvelope(int sampleIndex, int totalSamples, int rampSamples)
{
    if (sampleIndex < rampSamples) {
        // Attack: Ramp-up
        return (float)sampleIndex / rampSamples;
    } else if (sampleIndex >= totalSamples - rampSamples) {
        // Release: Ramp-down
        return (float)(totalSamples - sampleIndex) / rampSamples;
    } else {
        // Sustain
        return 1.0f;
    }
}
```

**Envelope Creation**: Added a linear ramp-up for the start of the beep and a ramp-down for the end. The envelope smoothly increases the amplitude from 0 to 1 (ramp-up) and decreases it back to 0 (ramp-down).

```cpp
// Envelope configuration
float rampDuration = 0.4f; // Ramp-up and ramp-down duration in seconds
float beepDuration = 1.5f; // Total beep duration (including ramp-up and ramp-down) in seconds
float breakDuration = 0.5f; // Duration of the silence between beeps in seconds

// Calculate sample counts
int rampSamples = rampDuration * props.sampleRate;
int beepSamples = beepDuration * props.sampleRate;
int breakSamples = breakDuration * props.sampleRate;
```
We calculate how many samples each steps need.

1. **Sample Count Calculations**: Converted durations (in seconds) into sample counts using the sampling rate (sampleRate).
2. **Break Between Beeps**: After the beep ends, silence is generated for a short duration (breakDuration).

Finally, we apply the modulation to the waveform:

```cpp
// Generate the sine wave sample with envelope
buffer[0] = amplitude * sin(phase);
```
Applied the envelope to the sine wave by multiplying the amplitude during the ramp-up and ramp-down sections.

Here is the whole code:
```cpp
// 03.cpp
#include "plugins/audio/AudioOutputPulse.h"
#include <cmath> // For sin()

#define TWO_PI 6.283185307179586

// Function to calculate the envelope value at a given sample index
float calculateEnvelope(int sampleIndex, int totalSamples, int rampSamples)
{
    if (sampleIndex < rampSamples) {
        // Attack: Ramp-up
        return (float)sampleIndex / rampSamples;
    } else if (sampleIndex >= totalSamples - rampSamples) {
        // Release: Ramp-down
        return (float)(totalSamples - sampleIndex) / rampSamples;
    } else {
        // Sustain
        return 1.0f;
    }
}

int main(int argc, char* argv[])
{
    // Initialize audio properties with default settings
    AudioPlugin::Props props = defaultAudioProps;

    // Create an AudioOutputPulse instance to handle audio output
    AudioOutputPulse audioOutput(props, (char*)"zicAudioOutputPulse");

    // Create an audio buffer to hold samples (one per track)
    float buffer[props.maxTracks] = { 0.0f };

    // Frequency of the sine wave (in Hz)
    float frequency = 440.0f; // Standard A4 note

    // Phase accumulator for the sine wave
    float phase = 0.0f;

    // Phase increment per sample
    float phaseIncrement = TWO_PI * frequency / props.sampleRate;

    // Envelope configuration
    float rampDuration = 0.4f; // Ramp-up and ramp-down duration in seconds
    float beepDuration = 1.5f; // Total beep duration (including ramp-up and ramp-down) in seconds
    float breakDuration = 0.5f; // Duration of the silence between beeps in seconds

    // Calculate sample counts
    int rampSamples = rampDuration * props.sampleRate;
    int beepSamples = beepDuration * props.sampleRate;
    int breakSamples = breakDuration * props.sampleRate;

    // Main loop to generate and play the sine wave with envelope
    while (1) {
        // Generate the beep with ramp-up and ramp-down
        for (int i = 0; i < beepSamples; ++i) {
            // Calculate the current amplitude using the envelope function
            float amplitude = calculateEnvelope(i, beepSamples, rampSamples);

            // Generate the sine wave sample with envelope
            buffer[0] = amplitude * sin(phase);

            // Increment the phase
            phase += phaseIncrement;

            // Wrap the phase back into the range [0, TWO_PI]
            if (phase >= TWO_PI) {
                phase -= TWO_PI;
            }

            // Send the generated sample to the audio output
            audioOutput.sample(buffer);
        }

        // Generate silence for the break
        for (int i = 0; i < breakSamples; ++i) {
            buffer[0] = 0.0f; // Silence
            audioOutput.sample(buffer);
        }
    }

    return 0;
}
```

Compile it with:
```sh
g++ 03.cpp -o 03.bin -I../../.. -lpulse-simple -lpulse && ./03.bin
```

**Important**: In audio processing, counting time is based on the sample rate instead of using the computer's system clock. It is critical for ensuring accuracy, consistency, and synchronization.

As we have seen in the previous section, in audio processing, we send audio data in chunks (blocks of samples) to the hardware audio interface. The hardware ensures these samples are sent to the audio buffer at the correct time, based on the defined sample rate. If we were to rely on the computer's system clock instead of the sample rate, it could become out of sync with the audio hardware, leading to timing mismatches, glitches, or drift in audio playback or processing. Using the sample rate ensures perfect synchronization with the hardware buffer and consistent timing.

## Lookup table

A Lookup Table (LUT) is a data structure, typically an array or table, that stores precomputed values. Instead of performing a computation repeatedly, the program retrieves the results directly from the table, offering faster access than recalculating the values on the fly.

**How Does a LUT Work?**

1. **Precompute Values:**
Values (such as sine wave points, logarithms, etc.) are precomputed and stored in the table. These values are indexed by a specific key, often an integer or floating-point value.

1. **Indexing:**
To retrieve a value, you use an index. For example, if you want to get the sine of 30 degrees, you look up the corresponding precomputed value from the table using the index for that angle.

1. **Direct Access:**
Instead of computing the sine of each angle in real-time (which can be computationally expensive), you directly access the value from the table. This is much faster, as memory access (loading a value) is typically faster than performing mathematical operations.

**Example of a Simple LUT**

Suppose we want to generate the sine wave for 360 degrees. Instead of calculating sin(x) every time we need it, we could create a LUT with precomputed sine values for 0 to 360 degrees (or even a higher resolution, like every 0.5 degree).

**LUT Example:**

| Index (degrees) | Sine Value |
|----|----|
| 0	| 0.0 |
| 1	| 0.017452 |
| 2	| 0.034899 |
| 3	| 0.052336 |
| ... |	... |
| 360 |	0.0 |

If you need the sine of 45 degrees, instead of recalculating sin(45), you simply look up the value stored at index 45 in the table.

```cpp
float sineTable[361];  // LUT for sine values

// Populate the table with sine values
for (int i = 0; i <= 360; ++i) {
    sineTable[i] = sin(i * M_PI / 180.0f);  // Convert degrees to radians
}

// Access the sine of 45 degrees directly
float sineValue = sineTable[45];
```

Using a lookup table instead of calculating the sine wave on the fly can provide significant performance benefits, especially in real-time audio applications. 

1. **Performance Efficiency**
- **Sine Calculation is Expensive:**
  Computing the sine of a number is a relatively expensive operation for a CPU. It involves floating-point calculations that can take several CPU cycles.

- **Lookup Table is Faster:**
  By precomputing the sine values and storing them in a table, you can simply access the correct value using an array index. This is a much faster operation—essentially just loading a value from memory (which takes only a few cycles), instead of recalculating it every time.

2. **Real-Time Constraints**
- **Real-Time Audio Processing:**
  Audio processing often occurs in real time, where the system needs to process audio data and send it to the output at a fixed rate (e.g., 44.1 kHz). If your audio generation code is constantly recalculating sine waves, it may not be able to keep up with the required output rate, causing delays or glitches.

- **Reduced Latency:**
  Using a lookup table can reduce the amount of time spent on computation, helping to maintain the required processing speed and minimize latency. This is crucial in real-time systems like audio playback.

3. **Reusability and Consistency**
- **Consistent Values:**
  A lookup table guarantees that the sine values are consistent and accurate each time they’re accessed. You don’t have to worry about rounding errors or inaccuracies that may arise in real-time floating-point computations. This is important when generating precise waveforms in audio applications.

- **Reusability Across Multiple Generations:**
  Once the table is created, it can be reused for generating multiple sine waves with different frequencies. The only thing you need to do is adjust the table index, based on the desired frequency. This saves on repeated computation.

4. **Reduced CPU Load**
- **Less CPU Usage:**
  In systems with limited CPU resources (such as embedded systems or low-power devices), reducing the number of computations required per sample can allow more resources to be dedicated to other parts of the application, such as mixing, effects, or handling multiple audio channels.

**When to Use Lookup Tables**

While lookup tables are very efficient, they do come with some trade-offs:

- **Memory Usage:** Lookup tables take up memory to store the precomputed values. Depending on the precision required and the number of different waveforms, the table size can be quite large. For sine waves, this may not be a major concern, but in systems with limited memory, this could be a factor.

- **Precision:** The resolution of the lookup table depends on how many values you precompute. A higher resolution table gives more accurate results, but it also takes up more memory. For many applications, a balance between performance and precision is required.

> [!CAUTION]
> While lookup tables can be an efficient way to store and retrieve precomputed values, they are not always the optimal choice.

After conducting benchmarks, I discovered that lookup tables (LUTs) are not always more efficient. For example, while implementing a drum snare, I benchmarked tone generation on my Raspberry Pi Zero using both a LUT and `sinf`. Surprisingly, the **LUT approach consumed 5% more CPU** than using `sinf`. This highlights the importance of testing optimizations in the context of specific use cases and hardware.

It may seem surprising that `sinf` performs better than the lookup table (LUT), but this behavior can be explained by several factors:

1. **Efficient Floating-Point Math**: The `sinf` function on modern CPUs, including ARM-based processors like those in the Raspberry Pi, is highly optimized. These processors often include hardware acceleration or highly efficient implementations for trigonometric functions in their standard math libraries. For small and predictable workloads, sinf can be faster than fetching values from memory, especially for a highly repetitive task.

2. **Memory Access Costs**: Using a lookup table introduces memory access overhead. Each access to the LUT involves an index calculation and memory fetch. On a constrained system like the Raspberry Pi, memory latency and cache misses can significantly impact performance.
   
3. **Pipeline and SIMD Optimization**: Modern CPUs can execute floating-point operations in parallel using vectorized instructions (like SIMD), but accessing LUTs involves sequential memory operations, which cannot benefit from such optimizations.

However, in contrast to `sinf`, I found that using a lookup table for `tanh` is more efficient than directly calling `tanh`. This can be explained by the fact that `sinf` is computationally simpler than `tanh`, and on the RPi0, `sinf` is likely optimized for performance, whereas `tanh` involves more complex computations that benefit more from precomputed values.

- `sinf`: The sine function is typically implemented using efficient approximations, such as Taylor series, polynomial approximations (like Chebyshev), or CORDIC algorithms. Modern libraries optimize `sinf` heavily for performance, especially on common architectures like ARM (used in the RPi0).

- `tanh`: The hyperbolic tangent (tanh) is computationally more complex than `sinf`. It involves exponentials in its definition. Computing  is costly on hardware without dedicated acceleration. Consequently, `tanh` is more expensive to compute directly than `sinf`.

- `sinf` function is likely optimized in the RPi0's math library (e.g., GNU C Library or newlib). These libraries use platform-specific optimizations like vectorization or approximation algorithms tailored to the ARM architecture.

- `tanh` being less common in many applications, may not be as optimized. This lack of optimization makes the LUT approach more favorable for `tanh`.

**Conclusion**

In audio programming, using a lookup table for wave generation can significantly enhance performance, reduce CPU/MCU load, and help meet real-time constraints. By trading a small amount of memory for faster and consistent calculations, this technique is a common optimization in many real-time audio systems. However, this approach is not universally ideal and should be applied cautiously depending on your hardware. Always perform benchmarking to ensure it benefit to your application.


Full example:
```cpp

#include "plugins/audio/AudioOutputPulse.h"
#include <cmath> // For sin()

#define TWO_PI 6.283185307179586

// Size of the lookup table
#define LUT_SIZE 1024

// Precompute the sine values and store them in a lookup table
float sineLUT[LUT_SIZE];

// Function to initialize the sine lookup table
void initializeSineLUT() {
    for (int i = 0; i < LUT_SIZE; ++i) {
        sineLUT[i] = sin(TWO_PI * i / LUT_SIZE);
    }
}

float calculateEnvelope(int sampleIndex, int totalSamples, int rampSamples)
{
    if (sampleIndex < rampSamples) {
        // Ramp-up
        return (float)sampleIndex / rampSamples;
    } else if (sampleIndex >= totalSamples - rampSamples) {
        // Ramp-down
        return (float)(totalSamples - sampleIndex) / rampSamples;
    } else {
        // Sustain
        return 1.0f;
    }
}

int main(int argc, char* argv[])
{
    // Initialize audio properties with default settings
    AudioPlugin::Props props = defaultAudioProps;

    // Create an AudioOutputPulse instance to handle audio output
    AudioOutputPulse audioOutput(props, (char*)"zicAudioOutputPulse");

    // Create an audio buffer to hold samples (one per track)
    float buffer[props.maxTracks] = { 0.0f };

    // Frequency of the sine wave (in Hz)
    float frequency = 440.0f; // Standard A4 note

    // Phase accumulator for the sine wave
    float phase = 0.0f;

    // Phase increment per sample based on the desired frequency
    float phaseIncrement = frequency * LUT_SIZE / props.sampleRate;

    // Initialize the sine lookup table
    initializeSineLUT();

    // Envelope configuration
    float rampDuration = 0.1f; // Ramp-up and ramp-down duration in seconds
    float beepDuration = 0.5f; // Total beep duration (including ramp-up and ramp-down) in seconds
    float breakDuration = 0.2f; // Duration of the silence between beeps

    // Calculate sample counts
    int rampSamples = rampDuration * props.sampleRate;
    int beepSamples = beepDuration * props.sampleRate;
    int breakSamples = breakDuration * props.sampleRate;

    // Main loop to generate and play the sine wave with envelope
    while (1) {
        // Generate the beep with ramp-up and ramp-down
        for (int i = 0; i < beepSamples; ++i) {
            // Calculate the current amplitude using the envelope function
            float amplitude = calculateEnvelope(i, beepSamples, rampSamples);

            // Get the sine value from the lookup table
            int index = static_cast<int>(phase) % LUT_SIZE;
            buffer[0] = amplitude * sineLUT[index];

            // Increment the phase
            phase += phaseIncrement;

            // Wrap the phase to stay within the LUT size
            if (phase >= LUT_SIZE) {
                phase -= LUT_SIZE;
            }

            // Send the generated sample to the audio output
            audioOutput.sample(buffer);
        }

        // Generate silence for the break
        for (int i = 0; i < breakSamples; ++i) {
            buffer[0] = 0.0f; // Silence
            audioOutput.sample(buffer);
        }
    }

    return 0;
}
```

## Linear interpolation

Linear interpolation is a way of estimating a value that falls between two known values. Imagine you’re looking at a straight line connecting two points on a graph. Linear interpolation is like drawing that line and picking a value somewhere between those two points. It assumes everything changes at a steady rate between the points.

**Why Linear Interpolation Matters in a Lookup Table (LUT)**

LUTs can’t store every possible value but only some of them (depending on the lookup table size). For any value not in the table, you need a way to estimate it. This is where linear interpolation becomes important.

- **It Fills the Gaps**: Since a LUT only has values at specific points, linear interpolation helps estimate values for the points in between.
- **It Keeps Things Smooth**: Using interpolation makes the transitions between the stored values look natural and continuous, rather than jumping abruptly.
- **It Saves Space**: Instead of storing every possible value in the table (which would require a lot of memory), you can store just a few key points and use interpolation to fill in the rest.

```cpp
float linearInterpolation(float index, uint16_t lutSize, float* lut)
{
    // Linear Interpolation to get smoother transitions between discrete LUT values
    
    // Step 1: Scale the normalized index (0.0 to 1.0) to the LUT range (0 to lutSize - 1).
    // This maps the input index to a position within the LUT.
    float lutIndex = index * (lutSize - 1);
    
    // Step 2: Find the lower neighboring index in the LUT.
    // Convert the floating-point position to an integer index.
    uint16_t index1 = (uint16_t)lutIndex;

    // Step 3: Find the upper neighboring index in the LUT.
    // Wrap around using modulo in case we are at the last LUT element.
    uint16_t index2 = (index1 + 1) % lutSize;

    // Step 4: Calculate the fractional distance between index1 and the exact position.
    // This determines how close the position is to the next LUT value (index2).
    float fractional = lutIndex - index1;

    // Step 5: Perform linear interpolation between the two neighboring LUT values.
    // The result is a weighted average of the values at index1 and index2.
    // The weights are determined by the fractional distance.
    return lut[index1] * (1.0f - fractional) + lut[index2] * fractional;
}
```

## Wavetable

Now that we understand lookup tables (LUTs), we can introduce **wavetables**, which are closely related. A wavetable is essentially a specialized form of a lookup table designed for audio synthesis. While a LUT stores precomputed values of a mathematical function or data for efficient retrieval, a wavetable specifically stores samples of one or more waveforms.

The primary difference is that a wavetable can contain multiple waveform representations within the same table, allowing it to store various types of periodic signals like sine waves, square waves, sawtooth waves, or custom waveforms. These waveforms can be accessed, interpolated, and combined to generate dynamic and evolving sounds in audio synthesis.

**What is a Wavetable?**

A wavetable is a precomputed array (or set of arrays) of values that represent one or more cycles of a waveform. These values are stored in memory and accessed during sound synthesis to efficiently produce continuous audio signals. Wavetables are widely used in music production and digital audio applications because they enable efficient and versatile sound generation.

Key characteristics of a wavetable:

1. **Waveform Representation**: It contains sampled points of a waveform. For example, a sine wave might be divided into 1024 evenly spaced samples.
2. **Multiple Waveforms**: A single wavetable can hold several different waveforms, which can be selected or interpolated for creative effects.
3. **Efficient Playback**: Rather than recalculating waveform values in real time (e.g., using trigonometric functions), the wavetable allows rapid access to precomputed values.

In short, using a wavetable is just like using a lookup table (LUT), but instead of storing a single set of values, you store multiple waveforms. These can then be accessed and interpolated as needed for audio synthesis.

```cpp
#include "plugins/audio/AudioOutputPulse.h"
#include <cmath> // For sin()

// Define the size of the wavetable and the number of waveforms
const uint16_t WAVE_SIZE = 256; // Number of samples per waveform
const uint16_t NUM_WAVES = 3;   // Number of waveforms in the wavetable

// Generate a simple wavetable with multiple waveforms
void generateWavetable(float wavetable[WAVE_SIZE * NUM_WAVES]) {
    for (uint8_t wave = 0; wave < NUM_WAVES; ++wave) {
        for (int16_t i = 0; i < WAVE_SIZE; ++i) {
            float phase = static_cast<float>(i) / WAVE_SIZE;
            if (wave == 0) {
                wavetable[wave * WAVE_SIZE + i] = std::sin(2.0f * M_PI * phase); // Sine wave
            } else if (wave == 1) {
                wavetable[wave * WAVE_SIZE + i] = phase < 0.5f ? 1.0f : -1.0f;    // Square wave
            } else if (wave == 2) {
                wavetable[wave * WAVE_SIZE + i] = 2.0f * (phase - 0.5f);         // Sawtooth wave
            }
        }
    }
}

int main() {
    // Initialize audio properties with default settings
    AudioPlugin::Props props = defaultAudioProps;

    // Create an AudioOutputPulse instance to handle audio output
    AudioOutputPulse audioOutput(props, (char*)"zicAudioOutputPulse");

    // Create an audio buffer to hold samples (one per track)
    float buffer[props.maxTracks] = { 0.0f };

    // Create a flat array to store the wavetable
    float wavetable[WAVE_SIZE * NUM_WAVES];

    // Generate waveforms
    generateWavetable(wavetable);

    uint8_t waveform = 0; // Sine wave
    int16_t index = 0;
    // Main loop to generate and play the sine wave
    while (1) {
        // Calculate the sine value for the current phase
        buffer[0] = wavetable[waveform * WAVE_SIZE + index];

        index++;
        while (index >= WAVE_SIZE) {
            index -= WAVE_SIZE;
        }

        // Send the generated sample to the audio output
        audioOutput.sample(buffer);
    }

    return 0;
}
```

You can create your own wavetable from scratch, but there are also plenty of ready-made options available online. For instance, [WaveEdit](https://synthtech.com/waveedit/) is a fantastic tool that allows you to design custom wavetables and provides access to a large library of shared wavetables created by the community. It's a great resource for both beginners and advanced users looking to explore or expand their wavetable collection.
