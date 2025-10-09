#pragma once

/* WifiComponent.h */

#include "helpers/http.h" // for execCmd helper style (we reimplement execCmd below)
#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"

#include <atomic>
#include <chrono>
#include <future>
#include <regex>
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
    uint8_t contextId = 0;

    std::vector<std::string> ssids;
    int currentNetworkIndex = 0;
    bool networksLoaded = false;

    std::string password = "";
    int cursorPos = 0; // cursor within password

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

    void setContextState(float v)
    {
        setContext(contextId, v);
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
        networksLoaded = false;
        ssids.clear();
        renderNext();

        std::thread([this]() {
            try {
                std::string output = execCmd("iwlist " + getInterface() + " scan");

                std::vector<std::string> found;
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
                                found.push_back(s);
                            continue;
                        }

                        std::smatch m;
                        std::regex essidRegex("ESSID:\\\"(.*)\\\"");
                        if (std::regex_search(line, m, essidRegex)) {
                            std::string s = m[1];
                            // logDebug("Found SSID: %s", s.c_str());
                            if (!s.empty())
                                found.push_back(s);
                            continue;
                        }
                    }
                }

                for (auto& s : found) {
                    if (std::find(ssids.begin(), ssids.end(), s) == ssids.end())
                        ssids.push_back(s);
                }

                if (ssids.empty()) {
                    std::string o2 = execCmd("iw dev " + getInterface() + " scan | grep ' SSID:'");
                    std::istringstream iss2(o2);
                    std::string l2;
                    while (std::getline(iss2, l2)) {
                        size_t p = l2.find("SSID:");
                        if (p != std::string::npos) {
                            std::string s = l2.substr(p + 5);
                            s.erase(0, s.find_first_not_of(" \t\r\n"));
                            s.erase(s.find_last_not_of(" \t\r\n") + 1);
                            if (!s.empty() && std::find(ssids.begin(), ssids.end(), s) == ssids.end())
                                ssids.push_back(s);
                        }
                    }
                }

                networksLoaded = true;
                if (!ssids.empty()) {
                    std::string current = execCmd("iwgetid -r");
                    if (!current.empty()) {
                        current.erase(current.find_last_not_of(" \n\r") + 1);
                        for (size_t i = 0; i < ssids.size(); i++) {
                            if (ssids[i] == current) {
                                currentNetworkIndex = (int)i;
                                break;
                            }
                        }
                    }
                    renderNext();
                } else {
                    showMessage("No networks found", 1500);
                }
            } catch (...) {
                showMessage("Scan failed", 1000);
            }
            scanning = false;
        }).detach();
    }

    std::string getCurrentSSID()
    {
        std::string s = execCmd("iwgetid -r");
        if (!s.empty())
            s.erase(s.find_last_not_of(" \n\r") + 1);
        return s;
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
                std::string cmd = "wpa_passphrase '" + ssid + "' '" + pass + "' > /tmp/wpa_" + std::to_string(getpid()) + ".conf";
                execCmd(cmd);
                execCmd("killall wpa_supplicant 2>/dev/null || true");
                execCmd("wpa_supplicant -B -i " + iface + " -c /tmp/wpa_" + std::to_string(getpid()) + ".conf");
                execCmd("udhcpc -i " + iface + " || dhclient " + iface + " || true");
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

    void disconnectAsync()
    {
        execCmd("killall wpa_supplicant 2>/dev/null || true");
        execCmd("killall udhcpc 2>/dev/null || true");
        execCmd("ip link set " + getInterface() + " down 2>/dev/null || true");
        showMessage("Disconnected", 800);
        renderNext();
    }

public:
    WifiComponent(ComponentInterface::Props props)
        : Component(props, [&](std::string action) {
            std::function<void(KeypadLayout::KeyMap&)> func = NULL;
            if (action == ".scan") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        scanNetworksAsync();
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
            if (action == ".disconnect") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        disconnectAsync();
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
                        if (!password.empty()) {
                            password.erase(password.end() - 1);
                            cursorPos = password.size();
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
        contextId = config.value("contextId", contextId);

        scanNetworksAsync();
    }

    void render() override
    {
        draw.filledRect(relativePosition, size, { bgColor });
        if (!lastMessage.empty()) {
            int textY = relativePosition.y + (size.h - fontSize) * 0.5;
            draw.textCentered({ relativePosition.x + size.w / 2, textY }, lastMessage, fontSize, { textColor, .font = font });
            return;
        }

        int sep = 4;
        int boxH = (size.h - sep) / 2;
        Point topPos = { relativePosition.x, relativePosition.y };
        Size topSize = { size.w, boxH };
        Point botPos = { relativePosition.x, relativePosition.y + boxH + sep };
        Size botSize = { size.w, boxH };

        draw.filledRect(topPos, topSize, { foregroundColor });
        int textY = topPos.y + (boxH - fontSize) * 0.5;
        std::string ssidText = "-";
        if (networksLoaded && !ssids.empty())
            ssidText = ssids[currentNetworkIndex];
        else if (scanning)
            ssidText = "Scanning...";
        draw.text({ topPos.x + 4, textY }, ssidText, fontSize, { textColor, .font = font });

        std::string cur = getCurrentSSID();
        if (!cur.empty() && cur == ssidText)
            draw.textRight({ topPos.x + topSize.w - 4, textY }, "Connected", fontSize, { activeColor, .font = font });

        draw.filledRect(botPos, botSize, { foregroundColor });
        int textYb = botPos.y + (boxH - fontSize) * 0.5;
        // draw.text({ botPos.x + 4, textYb }, password, fontSize, { textColor, .font = font });

        // split text in 3 parts
        std::string left, mid, right;

        if (cursorPos > 0)
            left = password.substr(0, cursorPos);

        if (cursorPos < (int)password.size())
            mid = password.substr(cursorPos, 1);

        if (cursorPos + 1 < (int)password.size())
            right = password.substr(cursorPos + 1);

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

    void onEncoder(int id, int8_t direction) override
    {
        if (id == ssidEncoderId) {
            if (networksLoaded && !ssids.empty()) {
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
