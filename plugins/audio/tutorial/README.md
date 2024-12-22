# 90 Music programming tutorial

In this tutorial, I will explain how to approach music programming based on my journey since I first started learning about it. My goal is to keep the content simple and approachable for most developers. Please note that I’m not an expert, so there’s a chance I might get a few things wrong. However, I’ll do my best to ensure the information is accurate and useful.

## Which programming language

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

