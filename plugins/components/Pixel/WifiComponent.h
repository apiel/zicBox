/** Description:
This header file defines the **WifiComponent**, a specialized software module designed to handle all aspects of wireless network management, typically within an embedded system or device dashboard.

**Purpose and Features**

The primary function of the WifiComponent is to provide a user interface for connecting to Wi-Fi networks. It manages:

1.  **Network Scanning:** It actively searches for nearby Wi-Fi networks (SSIDs) and maintains a list of available options.
2.  **Password Input:** It provides a mechanism for entering and editing the security password, including visual cues for the cursor position and the option to mask the characters.
3.  **Connection Management:** It saves the network credentials to the operating system's configuration files and executes the necessary commands to establish and maintain a connection.
4.  **Status Display:** It shows messages to the user regarding the scanning process, connection attempts, and the final connection status.

**How It Works**

The component operates by running essential system commands in the background. When a user requests a network scan or connection, the component launches a separate, dedicated background task. This allows the main application interface to remain responsive while waiting for the operating system to find networks or negotiate a connection.

The component is highly interactive, designed to respond to physical input devices called "encoders" (like rotary dials). These encoders allow the user to quickly navigate the list of discovered networks, move the cursor within the password field, and cycle through characters for input.

It tracks various settings, including custom colors, font styles, and connection details, ensuring the visual appearance integrates smoothly into the larger application environment.

sha: d654c4e9c099dbce82b3ab5639c7405ea06ef620593bc1d67229b488491f6bd9 
*/
#pragma once

/* WifiComponent.h */

#include "helpers/http.h" // for execCmd helper style (we reimplement execCmd below)
#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"

#include <atomic>
#include <chrono>
#include <fstream>
#include <future>
#include <regex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

class WifiComponent : public Component {
protected:
    Color bgColor;
    Color textColor;
    Color textEditColor;
    Color foregroundColor;
    Color activeColor;

    int fontSize = 12;
    void* font = NULL;

    int8_t ssidEncoderId = -1; // encoder used to navigate SSID list
    int8_t passwordEncoderId = -1; // encoder used to edit password / move cursor
    int8_t cursorEncoderId = -1; // encoder used to move cursor

    std::vector<std::string> ssids;
    int currentNetworkIndex = 0;
    bool initialized = false;

    std::string password = "";
    int cursorPos = 0; // cursor within
    bool masked = false;

    std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()-_=+[]{};:,.?/<> ";

    std::atomic<bool> scanning { false };
    std::atomic<bool> connecting { false };
    std::string lastMessage;

    // Helper to execute shell command and capture output
    static std::string execCmd(const std::string& cmd)
    {
        std::array<char, 256> buffer;
        std::string result;
        FILE* pipe = popen((cmd + " 2>&1").c_str(), "r");
        if (!pipe)
            return "";
        while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
            result += buffer.data();
        pclose(pipe);
        return result;
    }

    // Helper to get Wi-Fi interface name from environment (default: wlan0)
    std::string getInterface() const
    {
        const char* env = getenv("WIFI_IFACE");
        if (env && *env)
            return env;
        return "wlan0";
    }

    void showMessage(const std::string& msg, int durationMs = 1000)
    {
        lastMessage = msg;
        renderNext();
        std::thread([this, durationMs]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(durationMs));
            lastMessage = "";
            renderNext();
        }).detach();
    }

    void scanNetworksAsync()
    {
        if (scanning)
            return;
        scanning = true;
        ssids.clear();
        // showMessage("Scanning...", 500);
        renderNext();

        std::thread([this]() {
            try {
                logDebug("Scanning networks...");
                std::string output = execCmd("iwlist " + getInterface() + " scan");

                if (!output.empty()) {
                    std::istringstream iss(output);
                    std::string line;
                    while (std::getline(iss, line)) {
                        line.erase(0, line.find_first_not_of(" \t\r\n"));
                        line.erase(line.find_last_not_of(" \t\r\n") + 1);
                        if (line.empty())
                            continue;

                        if (line.find("SSID:") == 0) {
                            std::string s = line.substr(5);
                            if (!s.empty())
                                ssids.push_back(s);
                            continue;
                        }

                        std::smatch m;
                        std::regex essidRegex("ESSID:\\\"(.*)\\\"");
                        if (std::regex_search(line, m, essidRegex)) {
                            std::string s = m[1];
                            // logDebug("Found SSID: %s", s.c_str());
                            if (!s.empty())
                                ssids.push_back(s);
                            continue;
                        }
                    }
                }

                logDebug("Found %d networks", ssids.size());

                if (ssids.empty()) {
                    showMessage("No networks found", 1500);
                } else {
                    std::string current = getCurrentSSID();
                    for (size_t i = 0; i < ssids.size(); i++) {
                        if (ssids[i] == current) {
                            currentNetworkIndex = i;
                            break;
                        }
                    }
                    showMessage("Found " + std::to_string(ssids.size()) + " networks", 1500);
                }
            } catch (...) {
                showMessage("Scan failed", 1000);
            }
            scanning = false;
        }).detach();
    }

    std::string currentSSID = "";
    std::string& getCurrentSSID()
    {
        if (!currentSSID.empty())
            return currentSSID;

        std::string s = execCmd("iwgetid -r");
        if (!s.empty()) {
            s.erase(s.find_last_not_of(" \n\r") + 1);
            currentSSID = s;
            logDebug("Current SSID: %s", currentSSID.c_str());
        }
        return currentSSID;
    }

    void saveNetworkToWpaSupplicant(const std::string& ssid, const std::string& pass)
    {
        std::ofstream file("/etc/wpa_supplicant.conf", std::ios::app);
        if (!file.is_open()) {
            logDebug("Failed to open /etc/wpa_supplicant.conf for writing");
            return;
        }

        file << "\nnetwork={\n";
        file << "    ssid=\"" << ssid << "\"\n";
        file << "    psk=\"" << pass << "\"\n";
        file << "}\n";

        file.close();
        logDebug("Saved network %s to wpa_supplicant.conf", ssid.c_str());
    }

    void connectAsync(const std::string& ssid, const std::string& pass)
    {
        if (connecting)
            return;
        connecting = true;
        showMessage("Connecting...", 500);

        std::thread([this, ssid, pass]() {
            try {
                std::string iface = getInterface();
                saveNetworkToWpaSupplicant(ssid, pass); // Step 1
                execCmd("killall wpa_supplicant 2>/dev/null || true"); // Step 2
                execCmd("wpa_supplicant -B -i " + iface + " -c /etc/wpa_supplicant.conf"); // Step 3
                execCmd("udhcpc -i " + iface + " || dhclient " + iface + " || true"); // Step 4
                std::this_thread::sleep_for(std::chrono::seconds(2));
                std::string current = getCurrentSSID();
                if (!current.empty() && current == ssid)
                    showMessage("Connected", 1200);
                else
                    showMessage("Connect failed", 1500);
            } catch (...) {
                showMessage("Connect error", 1200);
            }
            connecting = false;
            renderNext();
        }).detach();
    }

    std::string getSavedPassword()
    {
        // logDebug("Reading wpa_supplicant.conf");
        std::ifstream file("/etc/wpa_supplicant.conf");
        if (!file.is_open()) {
            logDebug("Failed to open wpa_supplicant.conf");
            return "";
        }

        std::string line;

        logDebug("Reading wpa_supplicant.conf");

        while (std::getline(file, line)) {
            // remove leading/trailing spaces
            line.erase(0, line.find_first_not_of(" \t\r\n"));
            line.erase(line.find_last_not_of(" \t\r\n") + 1);
            if (line.empty() || line[0] == '#')
                continue;

            if (line.find("psk=") == 0) {
                std::string s = line.substr(4);
                if (!s.empty() && s.front() == '"' && s.back() == '"')
                    s = s.substr(1, s.size() - 2);
                return s;
            }
        }

        logDebug("Password not found in wpa_supplicant.conf");
        return ""; // not found
    }

public:
    WifiComponent(ComponentInterface::Props props)
        : Component(props, [&](std::string action) {
            std::function<void(KeypadLayout::KeyMap&)> func = NULL;
            if (action == ".scan") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        scanNetworksAsync();
                        renderNext();
                    }
                };
            }
            if (action == ".connect") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        if (!ssids.empty()) {
                            connectAsync(ssids[currentNetworkIndex], password);
                        } else {
                            showMessage("No SSID selected", 800);
                        }
                    }
                };
            }
            if (action == ".clear") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        password = "";
                        cursorPos = 0;
                        renderNext();
                    }
                };
            }
            if (action == ".backspace") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        if (!password.empty() && cursorPos > 0) {
                            password.erase(cursorPos - 1, 1); // remove 1 character at cursorPos - 1
                            cursorPos--; // move cursor back
                            renderNext();
                        }
                    }
                };
            }
            return func;
        })
        , bgColor(styles.colors.background)
        , textColor(styles.colors.text)
        , textEditColor(darken(styles.colors.text, 0.5))
        , foregroundColor(lighten(styles.colors.background, 0.5))
        , activeColor(rgba(24, 141, 59, 1))
    {
        nlohmann::json& config = props.config;
        bgColor = draw.getColor(config["bgColor"], bgColor);
        textColor = draw.getColor(config["textColor"], textColor);
        foregroundColor = draw.getColor(config["foregroundColor"], foregroundColor);
        activeColor = draw.getColor(config["activeColor"], activeColor);

        if (config.contains("font")) {
            font = draw.getFont(config["font"].get<std::string>().c_str());
            fontSize = draw.getDefaultFontSize(font);
        }
        fontSize = config.value("fontSize", fontSize);

        ssidEncoderId = config.value("ssidEncoderId", ssidEncoderId);
        passwordEncoderId = config.value("passwordEncoderId", passwordEncoderId);
        cursorEncoderId = config.value("cursorEncoderId", cursorEncoderId);
        masked = config.value("masked", masked);

        // scanNetworksAsync();

        password = getSavedPassword();
        cursorPos = password.length();
    }

    void render() override
    {
        if (!initialized) {
            scanNetworksAsync();
            initialized = true;
        }

        draw.filledRect(relativePosition, size, { bgColor });

        int sep = 4;
        int boxH = (size.h - sep) / 2;
        Point topPos = { relativePosition.x, relativePosition.y };
        Size topSize = { size.w, boxH };
        Point botPos = { relativePosition.x, relativePosition.y + boxH + sep };
        Size botSize = { size.w, boxH };

        draw.filledRect(topPos, topSize, { foregroundColor });
        int textY = topPos.y + (boxH - fontSize) * 0.5;
        std::string ssidText = "-";

        if (!lastMessage.empty())
            ssidText = lastMessage;
        else if (scanning)
            ssidText = "Scanning...";
        else if (!ssids.empty())
            ssidText = ssids[currentNetworkIndex];
        else
            ssidText = "Disconnected";
        draw.text({ topPos.x + 4, textY }, ssidText, fontSize, { textColor, .font = font });

        if (lastMessage.empty()) {
            std::string cur = getCurrentSSID();
            if (!cur.empty() && cur == ssidText)
                draw.textRight({ topPos.x + topSize.w - 4, textY }, "Connected", fontSize, { activeColor, .font = font });
        }

        draw.filledRect(botPos, botSize, { foregroundColor });
        int textYb = botPos.y + (boxH - fontSize) * 0.5;
        // draw.text({ botPos.x + 4, textYb }, password, fontSize, { textColor, .font = font });

        // split text in 3 parts
        std::string left, mid, right;

        if (cursorPos > 0)
            left = masked ? std::string(cursorPos, '*') : password.substr(0, cursorPos);
        if (cursorPos < (int)password.size())
            mid = password.substr(cursorPos, 1);
        if (cursorPos + 1 < (int)password.size())
            right = masked ? std::string(password.size() - cursorPos - 1, '*') : password.substr(cursorPos + 1);

        // Draw each part with correct color
        int x = botPos.x + 4;
        if (!left.empty())
            x = draw.text({ x, textYb }, left, fontSize, { textEditColor, .font = font });

        int cursorY = botPos.y + boxH - 1;
        if (!mid.empty()) {
            int prevX = x;
            x = draw.text({ x + 2, textYb }, mid, fontSize, { textColor, .font = font });
            draw.line({ prevX, cursorY }, { x, cursorY }, { textEditColor });
        } else {
            draw.line({ x, cursorY }, { x + 8, cursorY }, { textEditColor });
        }

        if (!right.empty())
            draw.text({ x + 2, textYb }, right, fontSize, { textEditColor, .font = font });
    }

    void onEncoder(int8_t id, int8_t direction) override
    {
        if (id == ssidEncoderId) {
            if (!ssids.empty()) {
                currentNetworkIndex += direction;
                if (currentNetworkIndex < 0)
                    currentNetworkIndex = 0;
                if (currentNetworkIndex >= (int)ssids.size())
                    currentNetworkIndex = ssids.size() - 1;
                renderNext();
            }
            return;
        }

        if (id == cursorEncoderId) {
            cursorPos += direction;
            if (cursorPos < 0)
                cursorPos = 0;
            if (cursorPos > (int)password.size())
                cursorPos = password.size();
            renderNext();
            return;
        }

        if (id == passwordEncoderId) {
            if (cursorPos == (int)password.size()) {
                int idx = (direction > 0) ? 0 : (int)charset.size() - 1;
                password.push_back(charset[idx]);
            } else if (!password.empty()) {
                char c = password[cursorPos];
                size_t pos = charset.find(c);
                int newPos = 0;
                if (pos == std::string::npos)
                    newPos = (direction > 0) ? 0 : (int)charset.size() - 1;
                else {
                    int cand = (int)pos + direction;
                    if (cand < 0)
                        cand = (int)charset.size() - 1;
                    if (cand >= (int)charset.size())
                        cand = 0;
                    newPos = cand;
                }
                password[cursorPos] = charset[newPos];
            }
            renderNext();
        }
    }
};
