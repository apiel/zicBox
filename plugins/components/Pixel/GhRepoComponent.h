/** Description:
This code defines a specialized user interface element, called a "GitHub Repository Component." Its primary function is to automatically identify and display the name of the GitHub repository associated with the current project data.

**How it Works:**

This component is designed to be highly dynamic. Instead of relying purely on application settings, it uses underlying system commands (like a terminal or command prompt) to interact with the Git version control system.

1.  **System Query:** The component runs a hidden command to ask the operating system: "What is the internet address (remote origin URL) for the files located in the project's `data` folder?"
2.  **Output Capture:** It captures the response from this command (e.g., the raw GitHub URL).
3.  **Cleanup:** It then processes this raw address, removing unnecessary technical parts like "github.com," leading slashes, and the standard ".git" ending. This ensures the user only sees a clean, readable repository name (like `username/projectname`).
4.  **Display:** Finally, the component draws a colored box and displays the detected repository name on the screen.

The appearance of this display—including background color, text color, font, and text size—is fully customizable through its settings. This component ensures that users always have immediate visual confirmation of which GitHub repository context they are currently working in.

sha: 2a73fe392e1a5dc535d215ffee409aab1b03f02fb364711d9a77f4fe9f2366f1 
*/
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
