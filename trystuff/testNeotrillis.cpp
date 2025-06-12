#include <iostream>   // For input/output operations (e.g., std::cout, std::cerr)
#include <vector>     // For dynamic arrays (e.g., std::vector<uint8_t>)
#include <string>     // For string manipulation (e.g., const char*)
#include <fcntl.h>    // For file control options (e.g., open, O_RDWR)
#include <unistd.h>   // For POSIX operating system API (e.g., close, usleep)
#include <sys/ioctl.h>// For device-specific control operations (e.g., ioctl)
#include <linux/i2c-dev.h> // Linux I2C device header, providing I2C_SLAVE
#include <linux/i2c.h> // Required for I2C_RDWR message structures
#include <iomanip>    // For std::hex, std::setw, std::setfill for debugging output
#include <cstdlib>    // For system() command
#include <errno.h>    // For errno, to get specific error codes

// --- Constants for NeoTrellis I2C Communication ---

// The default I2C address for the Adafruit NeoTrellis board.
#define NEO_TRELLIS_ADDR 0x2E

// NeoTrellis Commands (based on Adafruit's CircuitPython/Arduino libraries)
// These commands are sent to the ATSAMD09 microcontroller on the Trellis.
#define NEO_TRELLIS_COMMAND_READ_EVENTS_FIFO 0x05 // Read button press/release events from an internal FIFO
#define NEO_TRELLIS_COMMAND_SET_BRIGHTNESS 0x03   // Set the global brightness of the NeoPixels
#define NEO_TRELLIS_COMMAND_SET_KEY_EVENT 0x02    // Configure a key to trigger events on rising/falling edges
#define NEO_TRELLIS_COMMAND_SET_PIXEL_COLOR 0x04  // Set the color of a specific NeoPixel LED

// Edge types for key event configuration
// These define when a button press should generate an event.
#define NEO_TRELLIS_EDGE_RISING 0x01  // Event triggered when button is pressed (goes from high to low)
#define NEO_TRELLIS_EDGE_FALLING 0x02 // Event triggered when button is released (goes from low to high)
#define NEO_TRELLIS_EDGE_BOTH 0x03    // Event triggered on both press and release (not used in this example)

// A small delay (in microseconds) to wait before AND after each I2C transaction.
// This gives the NeoTrellis time to process commands.
#define I2C_COMMAND_DELAY_US 20000 // 20 milliseconds (increased from 5ms)

// --- Structure to hold Button Event Data ---
// This mimics the 'event' object in the Python example.
struct ButtonEvent {
    uint8_t key_number; // The index of the button (0-15) that triggered the event
    uint8_t edge;       // The type of edge detected (NEO_TRELLIS_EDGE_RISING or NEO_TRELLIS_EDGE_FALLING)
};

// --- NeoTrellis Class Definition ---
// Encapsulates the I2C communication and specific NeoTrellis operations.
class NeoTrellis {
public:
    /**
     * @brief Constructor for the NeoTrellis class.
     * Initializes the I2C bus connection.
     * @param i2c_device_path The path to the I2C device file (e.g., "/dev/i2c-1" for RPi 4).
     */
    NeoTrellis(const char* i2c_device_path);

    /**
     * @brief Destructor for the NeoTrellis class.
     * Closes the I2C bus file descriptor.
     */
    ~NeoTrellis();

    /**
     * @brief Sets the global brightness for all NeoPixel LEDs on the Trellis.
     * @param brightness A value from 0 (off) to 255 (full brightness).
     */
    void setBrightness(uint8_t brightness);

    /**
     * @brief Sets the color of a specific NeoPixel LED on the Trellis.
     * @param pixel The index of the NeoPixel (0-15).
     * @param r The red component of the color (0-255).
     * @param g The green component of the color (0-255).
     * @param b The blue component of the color (0-255).
     */
    void setPixelColor(uint8_t pixel, uint8_t r, uint8_t g, uint8_t b);

    /**
     * @brief Activates or deactivates event generation for a specific key.
     * @param key_number The index of the key (0-15).
     * @param edge_type The type of edge to activate (NEO_TRELLIS_EDGE_RISING, NEO_TRELLIS_EDGE_FALLING, or NEO_TRELLIS_EDGE_BOTH).
     */
    void activateKey(uint8_t key_number, uint8_t edge_type);

    /**
     * @brief Reads all available button events from the Trellis's internal FIFO buffer.
     * This is equivalent to the 'trellis.sync()' function in the Python library,
     * which reads and processes pending events.
     * @return A vector of ButtonEvent structs, each representing a key press or release.
     */
    std::vector<ButtonEvent> getEvents();

private:
    int i2c_fd;          // File descriptor for the opened I2C bus
    int trellis_addr;    // The I2C address of the NeoTrellis
    
    /**
     * @brief Helper function to perform an I2C write transaction using I2C_RDWR ioctl.
     * This ensures a single atomic write operation for command + data.
     * A delay is applied before and after the transaction.
     * @param command The single-byte command to send.
     * @param data A vector of bytes representing the data payload for the command.
     * @return true if the write operation was successful, false otherwise.
     */
    bool i2cWriteTransaction(uint8_t command, const std::vector<uint8_t>& data = {});

    /**
     * @brief Helper function to send a command and then read bytes from the I2C device
     * using a repeated start condition. This is crucial for devices that expect a
     * single transaction for register reads (write command, then read data without stopping).
     * A delay is applied before and after the transaction.
     * @param command The single-byte command to send before reading.
     * @param buffer A reference to a vector where the read bytes will be stored.
     * @param num_bytes The number of bytes expected to be read.
     * @return true if the read operation was successful, false otherwise.
     */
    bool i2cReadTransaction(uint8_t command, std::vector<uint8_t>& buffer, size_t num_bytes);
};

// --- NeoTrellis Class Implementation ---

NeoTrellis::NeoTrellis(const char* i2c_device_path) : trellis_addr(NEO_TRELLIS_ADDR) {
    // Open the I2C device file in read/write mode.
    i2c_fd = open(i2c_device_path, O_RDWR);
    if (i2c_fd < 0) {
        // If opening fails, print an error and exit the program.
        perror("Failed to open the I2C bus");
        exit(1);
    }

    // Set the I2C slave address. This tells the I2C driver which device to communicate with.
    // This ioctl should be successful if the device is found and accessible.
    if (ioctl(i2c_fd, I2C_SLAVE, trellis_addr) < 0) {
        // If setting the slave address fails, print an error, close the file descriptor, and exit.
        perror("Failed to acquire I2C bus access and/or talk to slave. Check I2C address, wiring, and power.");
        close(i2c_fd);
        exit(1);
    }
    std::cout << "NeoTrellis initialized successfully on " << i2c_device_path << std::endl;
}

NeoTrellis::~NeoTrellis() {
    // Close the I2C file descriptor if it's open.
    if (i2c_fd >= 0) {
        close(i2c_fd);
        std::cout << "I2C bus closed." << std::endl;
    }
}

bool NeoTrellis::i2cWriteTransaction(uint8_t command, const std::vector<uint8_t>& data) {
    usleep(I2C_COMMAND_DELAY_US); // Delay BEFORE the transaction

    // Create a buffer to hold the command and its data.
    std::vector<uint8_t> buffer;
    buffer.push_back(command); // Add the command byte first
    // Append the data bytes to the buffer
    buffer.insert(buffer.end(), data.begin(), data.end());

    // Prepare a single I2C message for writing the command and data
    struct i2c_msg msg;
    msg.addr = trellis_addr;
    msg.flags = 0; // 0 for write
    msg.len = buffer.size();
    msg.buf = buffer.data();

    struct i2c_rdwr_ioctl_data ioctl_data;
    ioctl_data.msgs = &msg;
    ioctl_data.nmsgs = 1; // We are sending one message (write)

    std::cerr << "Attempting to write " << buffer.size() << " bytes (cmd 0x"
              << std::hex << std::setw(2) << std::setfill('0') << (int)command << std::dec << "), data: ";
    for (uint8_t byte : data) {
        std::cerr << std::hex << std::setw(2) << std::setfill('0') << (int)byte << " ";
    }
    std::cerr << std::dec << std::endl;

    if (ioctl(i2c_fd, I2C_RDWR, &ioctl_data) < 0) {
        perror("Failed to perform I2C_RDWR (write transaction)");
        std::cerr << "Errno: " << errno << std::endl; // Report specific errno
        usleep(I2C_COMMAND_DELAY_US); // Delay AFTER the transaction, even on failure
        return false;
    }
    std::cerr << "Successfully wrote " << buffer.size() << " bytes." << std::endl;
    usleep(I2C_COMMAND_DELAY_US); // Delay AFTER the transaction
    return true;
}

bool NeoTrellis::i2cReadTransaction(uint8_t command, std::vector<uint8_t>& buffer, size_t num_bytes) {
    usleep(I2C_COMMAND_DELAY_US); // Delay BEFORE the transaction

    // Prepare two I2C messages: one for writing the command, one for reading the response.
    struct i2c_msg msgs[2];
    uint8_t cmd_byte = command;

    // Message 0: Write the command byte
    msgs[0].addr = trellis_addr;
    msgs[0].flags = 0; // 0 for write
    msgs[0].len = 1;
    msgs[0].buf = &cmd_byte;

    // Message 1: Read the specified number of bytes
    buffer.resize(num_bytes); // Ensure buffer has enough space
    msgs[1].addr = trellis_addr;
    msgs[1].flags = I2C_M_RD; // I2C_M_RD for read
    msgs[1].len = num_bytes;
    msgs[1].buf = buffer.data();

    // Combine the messages into an i2c_rdwr_ioctl_data structure
    struct i2c_rdwr_ioctl_data ioctl_data;
    ioctl_data.msgs = msgs;
    ioctl_data.nmsgs = 2; // We are sending two messages

    std::cerr << "Attempting to read " << num_bytes << " bytes after sending command 0x"
              << std::hex << std::setw(2) << std::setfill('0') << (int)command << std::dec << std::endl;

    // Perform the combined write-then-read I2C transaction
    if (ioctl(i2c_fd, I2C_RDWR, &ioctl_data) < 0) {
        perror("Failed to perform I2C_RDWR (read transaction)");
        std::cerr << "Errno: " << errno << std::endl; // Report specific errno
        usleep(I2C_COMMAND_DELAY_US); // Delay AFTER the transaction, even on failure
        return false;
    }
    std::cerr << "Successfully read " << num_bytes << " bytes." << std::endl;
    usleep(I2C_COMMAND_DELAY_US); // Delay AFTER the transaction
    return true;
}

void NeoTrellis::setBrightness(uint8_t brightness) {
    if (!i2cWriteTransaction(NEO_TRELLIS_COMMAND_SET_BRIGHTNESS, {brightness})) {
        std::cerr << "Error setting brightness." << std::endl;
    }
}

void NeoTrellis::setPixelColor(uint8_t pixel, uint8_t r, uint8_t g, uint8_t b) {
    if (!i2cWriteTransaction(NEO_TRELLIS_COMMAND_SET_PIXEL_COLOR, {pixel, r, g, b})) {
        std::cerr << "Error setting pixel color for key " << (int)pixel << std::endl;
    }
}

void NeoTrellis::activateKey(uint8_t key_number, uint8_t edge_type) {
    if (!i2cWriteTransaction(NEO_TRELLIS_COMMAND_SET_KEY_EVENT, {key_number, edge_type})) {
        std::cerr << "Error activating key " << (int)key_number << std::endl;
    }
}

std::vector<ButtonEvent> NeoTrellis::getEvents() {
    std::vector<ButtonEvent> events;
    std::vector<uint8_t> buffer;

    // The Trellis's event FIFO can hold multiple events.
    // Each event is 2 bytes: (key_number, edge_type).
    // The device sends (0xFF, 0xFF) when the FIFO is empty.
    while (true) {
        // Use i2cReadTransaction for the FIFO, which expects a repeated start condition
        if (!i2cReadTransaction(NEO_TRELLIS_COMMAND_READ_EVENTS_FIFO, buffer, 2)) {
            // If read fails, it might mean the FIFO is empty or a genuine I2C issue.
            // For event reading, failure often implies no more events, so we break.
            break;
        }

        uint8_t key_number = buffer[0];
        uint8_t edge = buffer[1];

        // Check for the end-of-FIFO marker (0xFF, 0xFF).
        if (key_number == 0xFF && edge == 0xFF) {
            break; // FIFO is empty, stop reading.
        }

        // If it's a valid event, add it to our list.
        events.push_back({key_number, edge});
    }
    return events;
}

// --- Main Program Logic ---
int main() {
    // Optional: Run i2cdetect to verify I2C device presence.
    // std::cout << "Running i2cdetect to verify NeoTrellis presence..." << std::endl;
    // system("sudo i2cdetect -y 1"); // Uncomment this line if you want to run i2cdetect at startup.
    // std::cout << "i2cdetect complete. Continuing with Trellis initialization." << std::endl;


    // Initialize the NeoTrellis object.
    // On Raspberry Pi 4, the primary I2C bus is typically "/dev/i2c-1".
    NeoTrellis trellis("/dev/i2c-1");

    // Set the brightness. 128 is roughly 50% of the maximum (255),
    // similar to the 0.5 value in the Python example.
    trellis.setBrightness(128);
    fflush(stdout); // Ensure messages are printed immediately
    fflush(stderr);

    // Define colors using std::vector<uint8_t> to hold RGB components.
    const std::vector<uint8_t> OFF = {0, 0, 0};       // Black
    const std::vector<uint8_t> PURPLE = {180, 0, 255}; // Purple
    const std::vector<uint8_t> CYAN = {0, 255, 255};   // Cyan

    // Activate rising and falling edge events for all 16 keys (0-15).
    // This tells the Trellis to report both button presses and releases.
    std::cout << "Activating keys for event detection..." << std::endl;
    for (uint8_t i = 0; i < 16; ++i) {
        trellis.activateKey(i, NEO_TRELLIS_EDGE_RISING);
        trellis.activateKey(i, NEO_TRELLIS_EDGE_FALLING);
        fflush(stdout); // Ensure messages are printed immediately
        fflush(stderr);
    }
    std::cout << "Keys activated." << std::endl;

    // Cycle the LEDs with a purple color on startup, then turn them off.
    std::cout << "Cycling LEDs..." << std::endl;
    fflush(stdout);
    for (uint8_t i = 0; i < 16; ++i) {
        trellis.setPixelColor(i, PURPLE[0], PURPLE[1], PURPLE[2]);
        usleep(50000); // Wait for 50 milliseconds (50,000 microseconds)
        fflush(stdout);
        fflush(stderr);
    }

    for (uint8_t i = 0; i < 16; ++i) {
        trellis.setPixelColor(i, OFF[0], OFF[1], OFF[2]);
        usleep(50000); // Wait for 50 milliseconds
        fflush(stdout);
        fflush(stderr);
    }
    std::cout << "LED cycle complete." << std::endl;
    std::cout << "Trellis ready. Press buttons to see lights!" << std::endl;
    fflush(stdout);

    // Main loop: Continuously read button events and update LEDs.
    while (true) {
        // Get all pending button events from the Trellis FIFO.
        std::vector<ButtonEvent> events = trellis.getEvents();

        // Iterate through each received event and process it.
        for (const auto& event : events) {
            // Check if the event is a rising edge (button press).
            if (event.edge == NEO_TRELLIS_EDGE_RISING) {
                std::cout << "Key " << (int)event.key_number << " pressed (Rising Edge)" << std::endl;
                // Turn the corresponding LED to CYAN.
                trellis.setPixelColor(event.key_number, CYAN[0], CYAN[1], CYAN[2]);
            }
            // Check if the event is a falling edge (button release).
            else if (event.edge == NEO_TRELLIS_EDGE_FALLING) {
                std::cout << "Key " << (int)event.key_number << " released (Falling Edge)" << std::endl;
                // Turn the corresponding LED OFF.
                trellis.setPixelColor(event.key_number, OFF[0], OFF[1], OFF[2]);
            }
            fflush(stdout); // Ensure messages are printed immediately
            fflush(stderr);
        }
        usleep(20000); // Wait for 20 milliseconds before checking for new events (similar to Python's 0.02s sleep)
    }

    return 0; // Program exits (though in this case, it's an infinite loop)
}
