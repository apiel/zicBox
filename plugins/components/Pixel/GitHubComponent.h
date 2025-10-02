#pragma once

#define CPPHTTPLIB_OPENSSL_SUPPORT

#include "helpers/http.h"
#include "libs/httplib/httplib.h"
#include "libs/nlohmann/json.hpp"
#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"

#include <atomic>
#include <chrono>
#include <future>
#include <fstream>

/*md
## GitHub

Github components to authenticate users.
*/

class GitHubComponent : public Component {
protected:
    Color bgColor;
    Color textColor;
    Color foregroundColor;

    int fontSize = 12;
    void* font = NULL;

    int boxWidth = 20;

    std::string userCode = "--------"; // fallback until fetched
    std::chrono::steady_clock::time_point expiryTime;
    std::atomic<bool> fetching = false;

    std::string deviceCode;
    std::string tokenFile = "~/.github_token";
    std::string accessToken;
    enum class State { WaitingCode,
        LoadingToken,
        Authenticated };
    std::atomic<State> state = State::WaitingCode;

    void fetchCodeAsync()
    {
        if (fetching)
            return;
        fetching = true;

        userCode = "--------";

        std::thread([this]() {
            try {
                httplib::Client cli("https://github.com");
                cli.set_connection_timeout(5, 0); // 5s
                cli.set_read_timeout(5, 0); // 5s

                auto res = cli.Post("/login/device/code",
                    { { "Content-Type", "application/x-www-form-urlencoded" } },
                    "client_id=Ov23liVWLp79r3lJpFK2&scope=repo",
                    "application/x-www-form-urlencoded");

                if (!res || res->status != 200) {
                    throw std::runtime_error("GitHub request failed");
                }

                auto kv = parseFormUrlEncoded(res->body);

                this->userCode = kv["user_code"];
                this->deviceCode = kv["device_code"];
                int expiresIn = std::stoi(kv["expires_in"]);
                this->expiryTime = std::chrono::steady_clock::now() + std::chrono::seconds(expiresIn);

                // logDebug("GitHub: code=%s, expires in %d seconds", userCode.c_str(), expiresIn);
                renderNext();
            } catch (const std::exception& ex) {
                logError("GitHub request failed: %s", ex.what());
            }
            fetching = false;
        }).detach();
    }

    void fetchTokenAsync()
    {
        if (deviceCode.empty())
            return;
        state = State::LoadingToken;

        std::thread([this]() {
            try {
                httplib::Client cli("https://github.com");
                cli.set_connection_timeout(5, 0);
                cli.set_read_timeout(5, 0);

                std::string body = "client_id=Ov23liVWLp79r3lJpFK2"
                                   "&device_code="
                    + deviceCode + "&grant_type=urn:ietf:params:oauth:grant-type:device_code";

                for (;;) {
                    auto res = cli.Post("/login/oauth/access_token",
                        { { "Content-Type", "application/x-www-form-urlencoded" }, { "Accept", "application/json" } },
                        body,
                        "application/x-www-form-urlencoded");

                    if (!res)
                        throw std::runtime_error("GitHub token request failed");

                    auto json = nlohmann::json::parse(res->body);

                    if (json.contains("error")) {
                        std::string err = json["error"];
                        if (err == "authorization_pending") {
                            std::this_thread::sleep_for(std::chrono::seconds(5));
                            continue; // still waiting
                        } else {
                            throw std::runtime_error("GitHub error: " + err);
                        }
                    }

                    this->accessToken = json.value("access_token", "");
                    if (!accessToken.empty()) {
                        logDebug("GitHub: token=%s", accessToken.c_str());
                        std::ofstream out(tokenFile);
                        out << accessToken;
                        out.close();
                        state = State::Authenticated;
                        renderNext();
                    }
                    break;
                }
            } catch (const std::exception& ex) {
                logError("GitHub token fetch failed: %s", ex.what());
                state = State::WaitingCode;
            }
        }).detach();
    }

    bool isExpired()
    {
        return std::chrono::steady_clock::now() >= expiryTime;
    }

    bool isAuthenticated()
    {
        if (state == State::Authenticated)
            return true;

        std::ifstream in(tokenFile);
        if (in) {
            std::getline(in, accessToken);
            if (!accessToken.empty()) {
                state = State::Authenticated;
                return true;
            }
        }
        return false;
    }

public:
    GitHubComponent(ComponentInterface::Props props)
        : Component(props, [&](std::string action) {
            std::function<void(KeypadLayout::KeyMap&)> func = NULL;
            if (action == ".refresh") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        fetching = false;
                        fetchCodeAsync();
                        renderNext();
                    }
                };
            }
            if (action == ".next") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        if (state == State::WaitingCode) {
                            fetchTokenAsync();
                            renderNext();
                        }
                    }
                };
            }
            return func;
        })
        , bgColor(styles.colors.background)
        , textColor(styles.colors.text)
        , foregroundColor(lighten(styles.colors.background, 0.5))
    {
        /*md md_config:Rect */
        nlohmann::json& config = props.config;

        /// The background color.
        bgColor = draw.getColor(config["bgColor"], bgColor); //eg: "#000000"

        /// The text color.
        textColor = draw.getColor(config["textColor"], textColor); //eg: "#000000"

        /// The foreground color.
        foregroundColor = draw.getColor(config["foregroundColor"], foregroundColor); //eg: "#000000"

        /// The font of the text. Default is null.
        if (config.contains("font")) {
            font = draw.getFont(config["font"].get<std::string>().c_str()); //eg: "Sinclair_S"
            fontSize = draw.getDefaultFontSize(font);
        }

        /// The font size of the text.
        fontSize = config.value("fontSize", fontSize); //eg: 8

        /*md md_config_end */

        resize();
    }

    void resize() override
    {
        boxWidth = size.w / 9;
    }

    void render()
    {
        draw.filledRect(relativePosition, size, { bgColor });

        if (isAuthenticated()) {
            draw.textCentered({ relativePosition.x + size.w / 2, relativePosition.y + size.h / 2 },
                "Authenticated", fontSize, { textColor, .font = font });
            return;
        }

        if (state == State::LoadingToken) {
            draw.textCentered({ relativePosition.x + size.w / 2, relativePosition.y + size.h / 2 },
                "Loading token...", fontSize, { textColor, .font = font });
            return;
        }

        if (userCode == "--------" || isExpired()) {
            fetchCodeAsync();
        }

        for (int i = 0; i < 9; i++) {
            Point pos = { relativePosition.x + i * boxWidth, relativePosition.y };
            Size boxSize = { boxWidth - 2, size.h };
            int textX = pos.x + boxSize.w * 0.5;
            int textY = pos.y + (boxSize.h - fontSize) * 0.5;
            if (i == 4) {
                draw.textCentered({ textX, textY }, "-", fontSize, { textColor, .font = font });
                continue;
            }

            draw.filledRect(pos, boxSize, { foregroundColor });

            std::string letter(1, userCode.size() > i ? userCode[i] : '-');
            draw.textCentered({ textX, textY }, letter, fontSize, { textColor, .font = font });
        }
    }
};
