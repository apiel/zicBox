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

Let’s begin with the tricky part. Many audio programming examples use the Q31 fixed-point format, which can be confusing for beginners. This is how I started, and it made understanding what was happening extremely difficult (an example using fixed point format is the famous [Teensy Audio Library]([link](https://github.com/PaulStoffregen/Audio))). For those just starting out in audio programming, I strongly recommend avoiding fixed-point formats like Q31 and sticking to floating-point numbers instead. It’s far more intuitive and easier to work with.

Fixed-point is a way of representing numbers where a specific number of bits are reserved for the integer part and the rest for the fractional part. Unlike floating-point, which can represent a wide range of values with dynamic precision, fixed-point numbers have a fixed number of decimal places, making them more efficient for certain applications, especially in hardware with limited processing power. For example, in Q31 format, 31 bits are used for the fractional part, and 1 bit is for the sign, allowing for precise representation of values between -1 and 1.

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

Compile it with:
```sh
g++ 01.cpp -o 01.bin -I../../.. -pthread -D_REENTRANT -lpulse-simple -lpulse -pthread && ./01.bin
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