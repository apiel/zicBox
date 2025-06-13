#include <chrono>
#include <cstring> // For strerror
#include <fcntl.h>
#include <functional>
#include <iomanip> // For std::hex, std::setw, std::setfill
#include <iostream>
#include <linux/i2c-dev.h>
#include <linux/i2c.h> // Required for i2c_msg and i2c_rdwr_ioctl_data structures
#include <sys/ioctl.h>
#include <thread>
#include <unistd.h>
#include <vector>

#define NEO_TRELLIS_KEY(x) (((x) / 4) * 8 + ((x) % 4))
#define NEO_TRELLIS_SEESAW_KEY(x) (((x) / 8) * 4 + ((x) % 8))

#define I2C_TRANSACTION_DELAY_MS 20 // 20 milliseconds

// Enable detailed I2C debugging logs for troubleshooting
// #define DEBUG_I2C
#define DEBUG_KEYPAD

enum {
    SEESAW_KEYPAD_EDGE_HIGH = 0,
    SEESAW_KEYPAD_EDGE_LOW,
    SEESAW_KEYPAD_EDGE_FALLING,
    SEESAW_KEYPAD_EDGE_RISING,
};

class NeoTrellis {
public:
    // Structure to represent a key event (press or release)
    struct KeyEvent {
        uint8_t number; // The key index (0-15)
        uint8_t edge; // The type of edge (rising or falling)
    };

private:
    int i2c_fd; // File descriptor for the I2C bus
    uint8_t address; // I2C address of the NeoTrellis

    // Seesaw module addresses (these are like "categories" of registers)
    static const uint8_t SEESAW_STATUS_BASE = 0x00;
    static const uint8_t SEESAW_GPIO_BASE = 0x01;
    static const uint8_t SEESAW_NEOPIXEL_BASE = 0x0E;
    static const uint8_t SEESAW_KEYPAD_BASE = 0x10;
    static const uint8_t SEESAW_KEYPAD_FIFO = 0x10;

    // Seesaw function addresses (specific registers within a module)
    static const uint8_t SEESAW_STATUS_SWRST = 0x7F; // Software reset command
    static const uint8_t SEESAW_STATUS_HW_ID = 0x01; // Hardware ID register
    static const uint8_t SEESAW_GPIO_DIRCLR_BULK = 0x02; // Clear direction (set as input) for bulk GPIO
    static const uint8_t SEESAW_GPIO_PULLUP_ENSET_BULK = 0x0B; // Enable pull-up for bulk GPIO
    static const uint8_t SEESAW_NEOPIXEL_PIN = 0x01; // Pin connected to NeoPixels
    static const uint8_t SEESAW_NEOPIXEL_SPEED = 0x02; // NeoPixel data rate
    static const uint8_t SEESAW_NEOPIXEL_BUF_LENGTH = 0x03; // Number of NeoPixels * 3 bytes (GRB)
    static const uint8_t SEESAW_NEOPIXEL_BUF = 0x04; // NeoPixel color buffer
    static const uint8_t SEESAW_NEOPIXEL_SHOW = 0x05; // Latch NeoPixel data to LEDs
    static const uint8_t SEESAW_KEYPAD_EVENT = 0x01; // Read keypad event FIFO
    static const uint8_t SEESAW_KEYPAD_INTENSET = 0x02; // Enable/Disable keypad interrupts
    static const uint8_t SEESAW_KEYPAD_COUNT = 0x04; // Number of pending keypad events in FIFO

    // NeoTrellis specific constants
    static const uint8_t NEO_TRELLIS_NEOPIX_PIN = 3; // The GPIO pin on ATSAMD09 connected to NeoPixels
    static const uint8_t NEO_TRELLIS_NUM_ROWS = 4;
    static const uint8_t NEO_TRELLIS_NUM_COLS = 4;

    // Callbacks storage: a vector of functions, one for each key
    std::vector<std::function<void(KeyEvent)>> callbacks;

public:
    static const uint8_t NEO_TRELLIS_NUM_KEYS = 16; // Moved to public scope

    // Edge definitions for key events (these match Adafruit's Python library)
    static const uint8_t EDGE_HIGH = 0;
    static const uint8_t EDGE_LOW = 1;
    static const uint8_t EDGE_FALLING = 2; // Button release
    static const uint8_t EDGE_RISING = 3; // Button press

    struct Color {
        uint8_t r, g, b;
        Color(uint8_t red = 0, uint8_t green = 0, uint8_t blue = 0)
            : r(red)
            , g(green)
            , b(blue)
        {
        }
    };

    static const Color OFF;
    static const Color RED;
    static const Color YELLOW;
    static const Color GREEN;
    static const Color CYAN;
    static const Color BLUE;
    static const Color PURPLE;

    NeoTrellis(const char* i2c_device = "/dev/i2c-1", uint8_t addr = 0x2E)
        : address(addr)
        , callbacks(NEO_TRELLIS_NUM_KEYS)
    {
        i2c_fd = open(i2c_device, O_RDWR);
        if (i2c_fd < 0) {
            std::cerr << "Failed to open I2C device '" << i2c_device << "': " << strerror(errno) << std::endl;
            throw std::runtime_error("Failed to open I2C device");
        }

        if (ioctl(i2c_fd, I2C_SLAVE, address) < 0) {
            std::cerr << "Failed to set I2C slave address to 0x" << std::hex << (int)address << ": " << strerror(errno) << std::endl;
            close(i2c_fd);
            throw std::runtime_error("Failed to set I2C slave address");
        }

        init();
    }

    ~NeoTrellis()
    {
        if (i2c_fd >= 0) {
            close(i2c_fd);
            std::cout << "I2C device closed." << std::endl;
        }
    }

    // bool test_communication()
    // {
    //     try {
    //         std::cout << "Testing I2C communication..." << std::endl;
    //         auto hw_id = read_register(SEESAW_STATUS_BASE, SEESAW_STATUS_HW_ID, 1);
    //         std::cout << "Hardware ID: 0x" << std::hex << (int)hw_id[0] << std::dec << std::endl;
    //         return hw_id[0] == 0x55; // Expected hardware ID for Seesaw
    //     } catch (const std::exception& e) {
    //         std::cerr << "Communication test failed: " << e.what() << std::endl;
    //         return false;
    //     }
    // }

    void init()
    {
        std::cout << "Starting device initialization..." << std::endl;

        // if (!test_communication()) {
        //     std::cerr << "Warning: Communication test failed, proceeding anyway..." << std::endl;
        // }

        // Perform a software reset on the Seesaw chip
        std::cout << "Sending software reset..." << std::endl;
        try {
            // Software reset command: write 0xFF to STATUS_SWRST register
            write_register(SEESAW_STATUS_BASE, SEESAW_STATUS_SWRST, { 0xFF });
            std::cout << "Software reset sent successfully." << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Failed to send software reset: " << e.what() << std::endl;
            throw; // Re-throw to indicate critical failure
        }

        // Wait for the device to restart after reset
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        std::cout << "Setting up NeoPixels..." << std::endl;
        // Configure the NeoPixel pin on the Seesaw chip
        write_register(SEESAW_NEOPIXEL_BASE, SEESAW_NEOPIXEL_PIN, { NEO_TRELLIS_NEOPIX_PIN });
        // Set NeoPixel speed to 800KHz (0x01)
        write_register(SEESAW_NEOPIXEL_BASE, SEESAW_NEOPIXEL_SPEED, { 0x01 });
        // Set NeoPixel buffer length (16 keys * 3 bytes/pixel = 48 bytes)
        // This command takes 2 bytes: high byte then low byte for length
        write_register(SEESAW_NEOPIXEL_BASE, SEESAW_NEOPIXEL_BUF_LENGTH, { 0x00, NEO_TRELLIS_NUM_KEYS * 3 });

        // --- CRUCIAL: Configure GPIO pins for the keypad as inputs with pull-ups ---
        std::cout << "Configuring keypad GPIO pins as inputs with pull-ups..." << std::endl;
        // All 16 keys (GPIOs 0-15) need to be configured.
        // Send 0xFF, 0xFF (16 bits all set) for bulk operations.
        write_register(SEESAW_GPIO_BASE, SEESAW_GPIO_DIRCLR_BULK, { 0xFF, 0xFF }); // Set all as inputs (clear direction)
        write_register(SEESAW_GPIO_BASE, SEESAW_GPIO_PULLUP_ENSET_BULK, { 0xFF, 0xFF }); // Enable pull-ups
        std::cout << "Keypad GPIO setup complete." << std::endl;

        std::cout << "Device initialization complete!" << std::endl;
    }

    void write_register(uint8_t reg_high, uint8_t reg_low, const std::vector<uint8_t>& data)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(I2C_TRANSACTION_DELAY_MS)); // Delay BEFORE transaction

        std::vector<uint8_t> buffer;
        buffer.push_back(reg_high); // Seesaw module address
        buffer.push_back(reg_low); // Seesaw function address
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

    union keyState {
        struct {
            uint8_t STATE : 1; ///< the current state of the key
            uint8_t ACTIVE : 4; ///< the registered events for that key
        } bit; ///< bitfield format
        uint8_t reg; ///< register format
    };

    void activateKey(uint8_t key, uint8_t edge, bool enable = true)
    {
        setKeypadEvent(NEO_TRELLIS_KEY(key), edge, enable);

        // key = NEO_TRELLIS_KEY(key);
        // keyState ks;
        // ks.bit.STATE = enable;
        // ks.bit.ACTIVE = (1 << edge);
        // write_register(SEESAW_KEYPAD_BASE, SEESAW_KEYPAD_EVENT, { key, ks.reg });
    }

    void setKeypadEvent(uint8_t key, uint8_t edge, bool enable)
    {
        keyState ks;
        ks.bit.STATE = enable;
        ks.bit.ACTIVE = (1 << edge);
        write_register(SEESAW_KEYPAD_BASE, SEESAW_KEYPAD_EVENT, { key, ks.reg });
    }

    std::vector<uint8_t> read_register(uint8_t reg_high, uint8_t reg_low, size_t length)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(I2C_TRANSACTION_DELAY_MS)); // Delay BEFORE transaction

        uint8_t reg_addr[2] = { reg_high, reg_low };
        std::vector<uint8_t> data(length);

        // Prepare two I2C messages: one for writing the command, one for reading the response.
        struct i2c_msg msgs[2];

        // Message 0: Write the command bytes
        msgs[0].addr = address;
        msgs[0].flags = 0; // 0 for write
        msgs[0].len = 2; // Command is 2 bytes (reg_high, reg_low)
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

    uint8_t getKeypadCount()
    {
        return read_register(SEESAW_KEYPAD_BASE, SEESAW_KEYPAD_COUNT, 1)[0];
    }

    union keyEventRaw {
        struct {
            uint8_t EDGE : 2; ///< the edge that was triggered
            uint8_t NUM : 6; ///< the event number
        } bit; ///< bitfield format
        uint8_t reg; ///< register format
    };

    void readKeys(bool polling = true)
    {
        uint8_t count = getKeypadCount();
        std::cout << "Keys count: " << (int)count << std::endl;
        std::this_thread::sleep_for(std::chrono::microseconds(500));
        if (count > 0 && count < 255) {
            // std::vector<uint8_t> data = read_register(SEESAW_KEYPAD_BASE, SEESAW_KEYPAD_FIFO, sizeof(keyEventRaw));
            // keyEventRaw* e = (keyEventRaw*)data.data();
            // std::cout << "Event: " << (int)e->bit.NUM << " " << (int)e->bit.EDGE << std::endl;

            std::vector<uint8_t> data = read_register(SEESAW_KEYPAD_BASE, SEESAW_KEYPAD_FIFO, 3);
            std::cout << "Event: " << (int)data[0] << " " << (int)data[1] << " " << (int)data[2] << std::endl;

            // std::vector<uint8_t> data = read_register(SEESAW_KEYPAD_BASE, SEESAW_KEYPAD_FIFO, count + 2);
            // for (int i = 0; i < count; i++) {
            //     keyEventRaw e = { data[i] };
            //     std::cout << i << ". Event: " << (int)e.bit.NUM << " " << (int)e.bit.EDGE << std::endl;
            // }

            // if (polling) {
            //     count = count + 2;
            // }
            // keyEventRaw e[count];
            // readKeypad(e, count);
            // for (int i = 0; i < count; i++) {
            //     // call any callbacks associated with the key
            //     e[i].bit.NUM = NEO_TRELLIS_SEESAW_KEY(e[i].bit.NUM);
            //     // if (e[i].bit.NUM < NEO_TRELLIS_NUM_KEYS && _callbacks[e[i].bit.NUM] != NULL) {
            //     //     keyEvent evt = { e[i].bit.EDGE, e[i].bit.NUM };
            //     //     _callbacks[e[i].bit.NUM](evt);
            //     // }
            //     if (e[i].bit.NUM < NEO_TRELLIS_NUM_KEYS) {
            //         std::cout << "Key " << (int)e[i].bit.NUM << " " << (int)e[i].bit.EDGE << std::endl;
            //     }
            // }
        }
    }

    bool readKeypad(keyEventRaw* buf, uint8_t count)
    {
        std::vector<uint8_t> data = read_register(SEESAW_KEYPAD_BASE, SEESAW_KEYPAD_FIFO, count * sizeof(keyEventRaw));
        memcpy(buf, data.data(), count * sizeof(keyEventRaw));
        return true;
    }

    void set_pixel_color(uint8_t pixel, const Color& color)
    {
        if (pixel >= NEO_TRELLIS_NUM_KEYS)
            return;

        // Corrected call: the address for NeoPixel buffer writes is 2 bytes after the command.
        // We'll pass the high byte as 0x00 and low byte as (pixel * 3)
        // The data vector needs to be G, R, B.
        write_register(SEESAW_NEOPIXEL_BASE, SEESAW_NEOPIXEL_BUF, { 0x00, (uint8_t)(pixel * 3), color.g, color.r, color.b });
    }

    void show()
    {
        write_register(SEESAW_NEOPIXEL_BASE, SEESAW_NEOPIXEL_SHOW, {});
    }

    void set_global_brightness(float brightness)
    {
        if (brightness < 0.0f)
            brightness = 0.0f;
        if (brightness > 1.0f)
            brightness = 1.0f;
        uint8_t value = static_cast<uint8_t>(brightness * 255);

        std::this_thread::sleep_for(std::chrono::milliseconds(I2C_TRANSACTION_DELAY_MS)); // Delay BEFORE transaction

        // NeoTrellis specific brightness command is 0x03 followed by the brightness byte
        std::vector<uint8_t> buffer = { 0x03, value };

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

int main()
{
    try {
        std::cout << "--- NeoTrellis C++ Debug Version ---" << std::endl;
        std::cout << "Verify NeoTrellis connection: run 'sudo i2cdetect -y 1' and look for 0x2E" << std::endl;

        // Create the NeoTrellis object
        NeoTrellis trellis("/dev/i2c-1", 0x2E);

        std::cout << "NeoTrellis object created and initialized successfully." << std::endl;

        for (int i = 0; i < NeoTrellis::NEO_TRELLIS_NUM_KEYS; i++) {
            trellis.activateKey(i, SEESAW_KEYPAD_EDGE_RISING);
            trellis.activateKey(i, SEESAW_KEYPAD_EDGE_FALLING);
        }

        // Set global brightness (equivalent to Python's trellis.brightness = 0.5)
        trellis.set_global_brightness(0.5f);

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

        while (true) {
            trellis.readKeys();

            // std::this_thread::sleep_for(std::chrono::milliseconds(20)); // Polling rate (20ms recommended by Adafruit)
            std::this_thread::sleep_for(std::chrono::milliseconds(2000)); // Polling rate (20ms recommended by Adafruit)
        }

    } catch (const std::exception& e) {
        std::cerr << "Application Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
