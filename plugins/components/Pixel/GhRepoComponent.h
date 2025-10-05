#pragma once

#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"

#include <array>
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>

/*md
## Repo

Displays the GitHub repository currently used in `data/`.
*/

class GhRepoComponent : public Component {
    Color bgColor;
    Color color;

    int fontSize = 12;
    void* font = NULL;

    std::string repoPath = "No repo";

    // --- Simple helper to run shell commands and capture output ---
    static std::string execCmd(const char* cmd)
    {
        std::array<char, 256> buffer;
        std::string result;

        FILE* pipe = popen(cmd, "r");
        if (!pipe)
            return "";
        while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
            result += buffer.data();
        }
        pclose(pipe);

        // Trim trailing newlines and spaces
        while (!result.empty() && (result.back() == '\n' || result.back() == '\r' || result.back() == ' '))
            result.pop_back();

        return result;
    }

    // --- Extract clean repo name ---
    void detectRepo()
    {
        // Get remote origin URL
        std::string cmd = "cd data 2>/dev/null && git remote get-url origin 2>/dev/null";
        std::string url = execCmd(cmd.c_str());

        if (url.empty()) {
            repoPath = "No repo";
            return;
        }

        // Normalize to "user/repo"
        size_t pos = url.find("github.com");
        if (pos != std::string::npos) {
            url = url.substr(pos + strlen("github.com"));
        }

        // Remove leading ":" or "/"
        if (!url.empty() && (url[0] == ':' || url[0] == '/'))
            url.erase(0, 1);

        // Remove .git suffix
        if (url.size() > 4 && url.substr(url.size() - 4) == ".git")
            url = url.substr(0, url.size() - 4);

        repoPath = url;
    }

public:
    GhRepoComponent(ComponentInterface::Props props)
        : Component(props)
        , bgColor(styles.colors.background)
        , color(styles.colors.text)
    {
        nlohmann::json& config = props.config;

        bgColor = draw.getColor(config["bgColor"], bgColor);
        color = draw.getColor(config["color"], color);
        fontSize = config.value("fontSize", fontSize);

        if (config.contains("font")) {
            font = draw.getFont(config["font"].get<std::string>().c_str());
        }
    }

    void render() override
    {
        detectRepo();
        draw.filledRect(relativePosition, size, { bgColor });
        draw.text({ relativePosition.x, relativePosition.y }, repoPath, fontSize, { color, .font = font });
    }
};
