#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <cstring> // For strerror
#include <functional>
#include <iomanip> // For std::hex, std::setw, std::setfill
#include <linux/i2c.h> // Required for i2c_msg and i2c_rdwr_ioctl_data structures

// Define a consistent delay after each I2C transaction
// This is crucial for the NeoTrellis to process commands reliably.
#define I2C_TRANSACTION_DELAY_MS 20 // 20 milliseconds

// Uncomment the line below to enable detailed I2C debugging logs
// #define DEBUG_I2C

class NeoTrellis {
public:
    // Structure to represent a key event (press or release)
    struct KeyEvent {
        uint8_t number; // The key index (0-15)
        uint8_t edge;   // The type of edge (rising or falling)
    };

private:
    int i2c_fd;          // File descriptor for the I2C bus
    uint8_t address;     // I2C address of the NeoTrellis
    
    // Seesaw module addresses (these are like "categories" of registers)
    static const uint8_t SEESAW_STATUS_BASE = 0x00;
    static const uint8_t SEESAW_GPIO_BASE = 0x01;
    static const uint8_t SEESAW_NEOPIXEL_BASE = 0x0E;
    static const uint8_t SEESAW_KEYPAD_BASE = 0x10;
    
    // Seesaw function addresses (specific registers within a module)
    static const uint8_t SEESAW_STATUS_SWRST = 0x7F;           // Software reset command
    static const uint8_t SEESAW_GPIO_DIRCLR_BULK = 0x02;       // Clear direction (set as input) for bulk GPIO
    static const uint8_t SEESAW_GPIO_PULLUP_ENSET_BULK = 0x0B; // Enable pull-up for bulk GPIO
    static const uint8_t SEESAW_NEOPIXEL_PIN = 0x01;           // Pin connected to NeoPixels
    static const uint8_t SEESAW_NEOPIXEL_SPEED = 0x02;         // NeoPixel data rate
    static const uint8_t SEESAW_NEOPIXEL_BUF_LENGTH = 0x03;    // Number of NeoPixels * 3 bytes (GRB)
    static const uint8_t SEESAW_NEOPIXEL_BUF = 0x04;           // NeoPixel color buffer
    static const uint8_t SEESAW_NEOPIXEL_SHOW = 0x05;          // Latch NeoPixel data to LEDs
    static const uint8_t SEESAW_KEYPAD_EVENT = 0x01;           // Read keypad event FIFO
    static const uint8_t SEESAW_KEYPAD_INTENSET = 0x02;        // Enable/Disable keypad interrupts
    static const uint8_t SEESAW_KEYPAD_COUNT = 0x04;           // Number of pending keypad events in FIFO
    
    // NeoTrellis specific constants
    static const uint8_t NEO_TRELLIS_NEOPIX_PIN = 3;    // The GPIO pin on ATSAMD09 connected to NeoPixels
    static const uint8_t NEO_TRELLIS_NUM_ROWS = 4;
    static const uint8_t NEO_TRELLIS_NUM_COLS = 4; 
    
    // Callbacks storage: a vector of functions, one for each key
    std::vector<std::function<void(KeyEvent)>> callbacks;
    
public: 
    static const uint8_t NEO_TRELLIS_NUM_KEYS = 16; // Moved to public scope
    
    // Edge definitions for key events (these match Adafruit's Python library)
    // Note: The raw event byte from Trellis has bits 6 and 7 indicating edge type,
    // not directly mapping to these values using a mask like `& 0x03`.
    // These are for internal use in our code/callbacks.
    static const uint8_t EDGE_HIGH = 0;    
    static const uint8_t EDGE_LOW = 1;     
    static const uint8_t EDGE_FALLING = 2; // Button release
    static const uint8_t EDGE_RISING = 3;  // Button press
    
    // Color structure for NeoPixels (GRB order expected by NeoTrellis)
    struct Color {
        uint8_t r, g, b;
        Color(uint8_t red = 0, uint8_t green = 0, uint8_t blue = 0) : r(red), g(green), b(blue) {}
    };
    
    // Static color definitions for convenience
    static const Color OFF;
    static const Color RED;
    static const Color YELLOW;
    static const Color GREEN;
    static const Color CYAN;
    static const Color BLUE;
    static const Color PURPLE;
    
    /**
     * @brief Constructor for the NeoTrellis.
     * @param i2c_device Path to the I2C device node (e.g., "/dev/i2c-1").
     * @param addr I2C address of the NeoTrellis (default 0x2E).
     */
    NeoTrellis(const char* i2c_device = "/dev/i2c-1", uint8_t addr = 0x2E) 
        : address(addr), callbacks(NEO_TRELLIS_NUM_KEYS) {
        
        // Open I2C device file
        i2c_fd = open(i2c_device, O_RDWR);
        if (i2c_fd < 0) {
            std::cerr << "Failed to open I2C device '" << i2c_device << "': " << strerror(errno) << std::endl;
            throw std::runtime_error("Failed to open I2C device");
        }
        
        // Set I2C slave address for subsequent communications
        if (ioctl(i2c_fd, I2C_SLAVE, address) < 0) {
            std::cerr << "Failed to set I2C slave address to 0x" << std::hex << (int)address << ": " << strerror(errno) << std::endl;
            close(i2c_fd);
            throw std::runtime_error("Failed to set I2C slave address");
        }
        
        // Initialize the NeoTrellis hardware
        init();
    }
    
    /**
     * @brief Destructor. Closes the I2C file descriptor.
     */
    ~NeoTrellis() {
        if (i2c_fd >= 0) {
            close(i2c_fd);
            std::cout << "I2C device closed." << std::endl;
        }
    }
    
    /**
     * @brief Initializes the NeoTrellis device, including software reset and NeoPixel setup.
     */
    void init() {
        std::cout << "Starting device initialization..." << std::endl;
        
        // Perform a software reset on the Seesaw chip
        std::cout << "Sending software reset..." << std::endl;
        try {
            // Software reset command: write 0xFF to STATUS_SWRST register
            write_register(SEESAW_STATUS_BASE, SEESAW_STATUS_SWRST, {0xFF});
            std::cout << "Software reset sent successfully." << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Failed to send software reset: " << e.what() << std::endl;
            throw; // Re-throw to indicate critical failure
        }
        
        // Wait for the device to restart after reset
        std::this_thread::sleep_for(std::chrono::milliseconds(1000)); 
        
        std::cout << "Setting up NeoPixels..." << std::endl;
        // Configure the NeoPixel pin on the Seesaw chip
        write_register(SEESAW_NEOPIXEL_BASE, SEESAW_NEOPIXEL_PIN, {NEO_TRELLIS_NEOPIX_PIN});
        // Set NeoPixel speed to 800KHz (0x01)
        write_register(SEESAW_NEOPIXEL_BASE, SEESAW_NEOPIXEL_SPEED, {0x01});
        // Set NeoPixel buffer length (16 keys * 3 bytes/pixel = 48 bytes)
        // This command takes 2 bytes: high byte then low byte for length
        write_register(SEESAW_NEOPIXEL_BASE, SEESAW_NEOPIXEL_BUF_LENGTH, {0x00, NEO_TRELLIS_NUM_KEYS * 3});
        
        // --- CRUCIAL: Configure GPIO pins for the keypad as inputs with pull-ups ---
        std::cout << "Configuring keypad GPIO pins as inputs with pull-ups..." << std::endl;
        // All 16 keys (GPIOs 0-15) need to be configured.
        // Send 0xFF, 0xFF (16 bits all set) for bulk operations.
        write_register(SEESAW_GPIO_BASE, SEESAW_GPIO_DIRCLR_BULK, {0xFF, 0xFF}); // Set all as inputs (clear direction)
        write_register(SEESAW_GPIO_BASE, SEESAW_GPIO_PULLUP_ENSET_BULK, {0xFF, 0xFF}); // Enable pull-ups
        std::cout << "Keypad GPIO setup complete." << std::endl;

        std::cout << "Device initialization complete!" << std::endl;
    }
    
    /**
     * @brief Writes data to a specific register on the Seesaw chip.
     * @param reg_high High byte of the register address (module base).
     * @param reg_low Low byte of the register address (function address).
     * @param data Vector of bytes to write.
     */
    void write_register(uint8_t reg_high, uint8_t reg_low, const std::vector<uint8_t>& data) {
        std::this_thread::sleep_for(std::chrono::milliseconds(I2C_TRANSACTION_DELAY_MS)); // Delay BEFORE transaction
        
        std::vector<uint8_t> buffer;
        buffer.push_back(reg_high); // Seesaw module address
        buffer.push_back(reg_low);  // Seesaw function address
        buffer.insert(buffer.end(), data.begin(), data.end()); // Actual data
        
        #ifdef DEBUG_I2C
        // Debugging output
        std::cerr << "I2C Write: addr=0x" << std::hex << (int)address
                  << ", reg=0x" << (int)reg_high << " 0x" << (int)reg_low << std::dec
                  << ", len=" << buffer.size() << ", data={";
        for (size_t i = 2; i < buffer.size(); ++i) { // Start from index 2 to show only actual data bytes
            std::cerr << "0x" << std::hex << std::setw(2) << std::setfill('0') << (int)buffer[i] << std::dec << " ";
        }
        std::cerr << "}" << std::endl;
        #endif

        ssize_t written = write(i2c_fd, buffer.data(), buffer.size());
        if (written != (ssize_t)buffer.size()) {
            std::cerr << "I2C write failed: wrote " << written << " bytes instead of " << buffer.size() << std::endl;
            std::cerr << "Error: " << strerror(errno) << " (errno " << errno << ")" << std::endl;
            throw std::runtime_error("I2C write failed");
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(I2C_TRANSACTION_DELAY_MS)); // Delay AFTER transaction
    }
    
    /**
     * @brief Reads data from a specific register on the Seesaw chip using I2C_RDWR ioctl.
     * This ensures an atomic write (command) then read (data) transaction.
     * @param reg_high High byte of the register address (module base).
     * @param reg_low Low byte of the register address (function address).
     * @param length Number of bytes to read.
     * @return Vector of bytes read from the register.
     */
    std::vector<uint8_t> read_register(uint8_t reg_high, uint8_t reg_low, size_t length) {
        std::this_thread::sleep_for(std::chrono::milliseconds(I2C_TRANSACTION_DELAY_MS)); // Delay BEFORE transaction

        uint8_t reg_addr[2] = {reg_high, reg_low};
        std::vector<uint8_t> data(length);

        // Prepare two I2C messages: one for writing the command, one for reading the response.
        struct i2c_msg msgs[2];

        // Message 0: Write the command bytes
        msgs[0].addr = address;
        msgs[0].flags = 0; // 0 for write
        msgs[0].len = 2;   // Command is 2 bytes (reg_high, reg_low)
        msgs[0].buf = reg_addr;

        // Message 1: Read the specified number of bytes
        msgs[1].addr = address;
        msgs[1].flags = I2C_M_RD; // I2C_M_RD for read
        msgs[1].len = length;
        msgs[1].buf = data.data();

        // Combine the messages into an i2c_rdwr_ioctl_data structure
        struct i2c_rdwr_ioctl_data ioctl_data;
        ioctl_data.msgs = msgs;
        ioctl_data.nmsgs = 2; // We are sending two messages in one atomic transaction

        #ifdef DEBUG_I2C
        std::cerr << "I2C Read (atomic): addr=0x" << std::hex << (int)address
                  << ", reg=0x" << (int)reg_high << " 0x" << (int)reg_low << std::dec
                  << ", read_len=" << length << std::endl;
        #endif

        // Perform the combined write-then-read I2C transaction
        if (ioctl(i2c_fd, I2C_RDWR, &ioctl_data) < 0) {
            std::cerr << "Failed to perform I2C_RDWR (read transaction): " << strerror(errno) << " (errno " << errno << ")" << std::endl;
            throw std::runtime_error("I2C read transaction failed");
        }
        
        #ifdef DEBUG_I2C
        // Debugging output for read data
        std::cerr << "I2C Read Data: {";
        for (uint8_t byte : data) {
            std::cerr << "0x" << std::hex << std::setw(2) << std::setfill('0') << (int)byte << std::dec << " ";
        }
        std::cerr << "}" << std::endl;
        #endif

        std::this_thread::sleep_for(std::chrono::milliseconds(I2C_TRANSACTION_DELAY_MS)); // Delay AFTER transaction
        return data;
    }
    
    /**
     * @brief Sets the color of a specific NeoPixel LED.
     * @param pixel Index of the NeoPixel (0-15).
     * @param color The Color struct containing R, G, B components.
     */
    void set_pixel_color(uint8_t pixel, const Color& color) {
        if (pixel >= NEO_TRELLIS_NUM_KEYS) return;
        
        // Corrected call: the address for NeoPixel buffer writes is 2 bytes after the command.
        // We'll pass the high byte as 0x00 and low byte as (pixel * 3)
        // The data vector needs to be G, R, B.
        write_register(SEESAW_NEOPIXEL_BASE, SEESAW_NEOPIXEL_BUF, {0x00, (uint8_t)(pixel * 3), color.g, color.r, color.b});
    }
    
    /**
     * @brief Shows the updated NeoPixel colors.
     * After calling set_pixel_color, you must call show() for changes to appear.
     */
    void show() {
        write_register(SEESAW_NEOPIXEL_BASE, SEESAW_NEOPIXEL_SHOW, {});
    }
    
    /**
     * @brief Activates event reporting for a specific key and edge type.
     * This sends the command to the NeoTrellis microcontroller.
     * @param key The key index (0-15).
     * @param edge The edge type (EDGE_RISING or EDGE_FALLING).
     */
    void activate_key(uint8_t key, uint8_t edge) {
        if (key >= NEO_TRELLIS_NUM_KEYS) return;
        
        // Command to enable/disable keypad interrupts/events is KEYPAD_INTENSET (0x02)
        // Data format: [key_number, edge_type]
        // This tells the Seesaw to report events for this key on the specified edge.
        write_register(SEESAW_KEYPAD_BASE, SEESAW_KEYPAD_INTENSET, {key, edge});
        
        std::cout << "Key " << (int)key << " activated for edge " << (int)edge << std::endl;
    }
    
    /**
     * @brief Sets a callback function to be called when a key event occurs.
     * @param key The key index (0-15).
     * @param callback The function to call, taking a KeyEvent as argument.
     */
    void set_callback(uint8_t key, std::function<void(KeyEvent)> callback) {
        if (key < NEO_TRELLIS_NUM_KEYS) {
            callbacks[key] = callback;
        }
    }
    
    /**
     * @brief Synchronizes with the NeoTrellis to read and dispatch pending key events.
     * This should be called regularly in your main loop.
     */
    void sync() {
        // Declare old_cerr here to ensure it's in scope for all conditional blocks
        std::streambuf* old_cerr = nullptr; 

        #ifndef DEBUG_I2C
        // Temporarily suppress stderr for read_register calls within sync()
        old_cerr = std::cerr.rdbuf();
        std::cerr.rdbuf(nullptr); // Redirect cerr to null
        #endif

        try {
            // Step 1: Read the number of pending key events from the FIFO
            std::vector<uint8_t> count_data;
            try {
                count_data = read_register(SEESAW_KEYPAD_BASE, SEESAW_KEYPAD_COUNT, 1);
            } catch (const std::exception& e) {
                #ifndef DEBUG_I2C
                std::cerr.rdbuf(old_cerr); // Restore cerr before logging error
                #endif
                // It's possible to get an I/O error if the device isn't ready or there's bus noise.
                // For sync(), we can often ignore these if the device is generally functioning.
                // If errors persist, enabling DEBUG_I2C will show more.
                // std::cerr << "Error reading keypad event count: " << e.what() << std::endl;
                return; // Exit sync if count read fails
            }
            
            uint8_t count = count_data[0]; // The number of events in the FIFO

            // Step 2: If there are events, read them one by one and dispatch callbacks
            for (uint8_t i = 0; i < count; ++i) {
                std::vector<uint8_t> event_bytes;
                try {
                    // Read one event byte from the FIFO
                    event_bytes = read_register(SEESAW_KEYPAD_BASE, SEESAW_KEYPAD_EVENT, 1);
                } catch (const std::exception& e) {
                    #ifndef DEBUG_I2C
                    std::cerr.rdbuf(old_cerr); // Restore cerr before logging error
                    #endif
                    std::cerr << "Error reading event byte " << (int)i << ": " << e.what() << std::endl;
                    break; // Break the loop if reading an event fails
                }

                if (event_bytes.empty()) { // Should not happen if count > 0, but safety check
                    break;
                }

                uint8_t event_byte = event_bytes[0];
                uint8_t key_num = event_byte & 0x3F; // Key number is bits 0-5
                uint8_t edge = 0; // Initialize edge

                // Determine edge type based on bits 6 and 7, as per Adafruit's library
                if (event_byte & 0x80) { // Bit 7 (0b10000000) is set for KEY_DOWN (Rising Edge)
                    edge = EDGE_RISING;
                } else if (event_byte & 0x40) { // Bit 6 (0b01000000) is set for KEY_UP (Falling Edge)
                    edge = EDGE_FALLING;
                } else {
                    // If neither bit 6 nor bit 7 is set, it's an unexpected edge type.
                    // This can happen if the FIFO is empty but the count was reported incorrectly,
                    // or a corrupted event byte.
                    std::cerr << "Invalid edge detected for raw event_byte=0x" << std::hex << (int)event_byte << std::dec << std::endl;
                    continue; // Skip this event
                }
                
                // Only dispatch if it's a known key and a valid edge type
                if (key_num < NEO_TRELLIS_NUM_KEYS && (edge == EDGE_RISING || edge == EDGE_FALLING) && callbacks[key_num]) {
                    KeyEvent event = {key_num, edge};
                    callbacks[key_num](event);
                } else {
                    std::cerr << "Received event for invalid key or no callback: key_num=" << (int)key_num
                              << ", edge=" << (int)edge << " (raw event_byte=0x"
                              << std::hex << (int)event_byte << std::dec << ")" << std::endl;
                }
            } // End for loop for events

        } catch (const std::exception& e) {
            std::cerr << "Error in sync loop: " << e.what() << std::endl;
        }

        #ifndef DEBUG_I2C
        std::cerr.rdbuf(old_cerr); // Restore cerr at the end of sync()
        #endif
    }
    
    /**
     * @brief Sets the global brightness of the NeoPixels.
     * This uses a direct I2C command specific to the NeoTrellis (not a Seesaw register).
     * @param brightness A float value from 0.0 (off) to 1.0 (full brightness).
     */
    void set_global_brightness(float brightness) {
        if (brightness < 0.0f) brightness = 0.0f;
        if (brightness > 1.0f) brightness = 1.0f;
        uint8_t value = static_cast<uint8_t>(brightness * 255);
        
        std::this_thread::sleep_for(std::chrono::milliseconds(I2C_TRANSACTION_DELAY_MS)); // Delay BEFORE transaction
        
        // NeoTrellis specific brightness command is 0x03 followed by the brightness byte
        std::vector<uint8_t> buffer = {0x03, value};
        
        #ifdef DEBUG_I2C
        std::cerr << "I2C Write Brightness: addr=0x" << std::hex << (int)address
                  << ", cmd=0x03, value=0x" << std::setw(2) << std::setfill('0') << (int)value << std::dec << std::endl;
        #endif
        
        ssize_t written = write(i2c_fd, buffer.data(), buffer.size());
        if (written != (ssize_t)buffer.size()) {
            std::cerr << "Failed to set global brightness: wrote " << written << " bytes instead of " << buffer.size() << std::endl;
            std::cerr << "Error: " << strerror(errno) << " (errno " << errno << ")" << std::endl;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(I2C_TRANSACTION_DELAY_MS)); // Delay AFTER transaction
    }
};

// Static color definitions (initialized outside the class)
const NeoTrellis::Color NeoTrellis::OFF(0, 0, 0);
const NeoTrellis::Color NeoTrellis::RED(255, 0, 0);
const NeoTrellis::Color NeoTrellis::YELLOW(255, 150, 0);
const NeoTrellis::Color NeoTrellis::GREEN(0, 255, 0);
const NeoTrellis::Color NeoTrellis::CYAN(0, 255, 255);
const NeoTrellis::Color NeoTrellis::BLUE(0, 0, 255);
const NeoTrellis::Color NeoTrellis::PURPLE(180, 0, 255);

// Global callback function (needs a reference to trellis to call its methods)
void blink(NeoTrellis::KeyEvent event, NeoTrellis& trellis) {
    if (event.edge == NeoTrellis::EDGE_RISING) {
        std::cout << "Detected event: Key " << (int)event.number << " pressed (Rising Edge)" << std::endl;
        trellis.set_pixel_color(event.number, NeoTrellis::CYAN);
        trellis.show(); // Update LEDs
    } else if (event.edge == NeoTrellis::EDGE_FALLING) {
        std::cout << "Detected event: Key " << (int)event.number << " released (Falling Edge)" << std::endl;
        trellis.set_pixel_color(event.number, NeoTrellis::OFF);
        trellis.show(); // Update LEDs
    }
}

int main() {
    try {
        std::cout << "--- NeoTrellis C++ Example ---" << std::endl;
        std::cout << "Verify NeoTrellis connection: run 'sudo i2cdetect -y 1' and look for 0x2E" << std::endl;
        
        // Create the NeoTrellis object
        NeoTrellis trellis("/dev/i2c-1", 0x2E);
        
        std::cout << "NeoTrellis object created and initialized successfully." << std::endl;

        // Set global brightness (equivalent to Python's trellis.brightness = 0.5)
        trellis.set_global_brightness(0.5f);
        
        // Set up callbacks and activate events for all 16 keys
        std::cout << "Activating key event detection and setting callbacks..." << std::endl;
        for (int i = 0; i < NeoTrellis::NEO_TRELLIS_NUM_KEYS; i++) {
            // Activate rising and falling edge events for each key
            trellis.activate_key(i, NeoTrellis::EDGE_RISING);
            trellis.activate_key(i, NeoTrellis::EDGE_FALLING);
            
            // Set callback using a lambda to capture the 'trellis' reference
            trellis.set_callback(i, [&trellis](NeoTrellis::KeyEvent event) {
                blink(event, trellis);
            });
        }
        std::cout << "Key event detection activated for all keys." << std::endl;
        
        // Startup LED cycle
        std::cout << "Starting LED cycle (Purple then Off)..." << std::endl;
        for (int i = 0; i < NeoTrellis::NEO_TRELLIS_NUM_KEYS; i++) {
            trellis.set_pixel_color(i, NeoTrellis::PURPLE);
            trellis.show(); // Latch the colors to the LEDs
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        
        for (int i = 0; i < NeoTrellis::NEO_TRELLIS_NUM_KEYS; i++) {
            trellis.set_pixel_color(i, NeoTrellis::OFF);
            trellis.show(); // Latch the colors to the LEDs
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        std::cout << "LED cycle complete." << std::endl;
        
        std::cout << "Ready! Press keys on the NeoTrellis to see lights and events." << std::endl;
        
        // Main loop: Continuously check for and process key events
        while (true) {
            trellis.sync(); // This reads keypad events and triggers callbacks
            std::this_thread::sleep_for(std::chrono::milliseconds(20)); // Polling rate (20ms recommended by Adafruit)
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Application Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
