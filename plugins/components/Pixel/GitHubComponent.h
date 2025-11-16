#pragma once

/* Toggle HTTP implementation: 1 = httplib, 0 = wget */
#define USE_HTTPLIB 0

#if USE_HTTPLIB
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "libs/httplib/httplib.h"
#endif

#include "helpers/http.h"
#include "host/constants.h"
#include "libs/nlohmann/json.hpp"
#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <future>
#include <string>
#include <thread>

/*md
## GitHub

Github components to authenticate users.
*/

class GitHubComponent : public Component {
protected:
    Color bgColor;
    Color textColor;
    Color foregroundColor;
    Color scrollColor;
    Color activeColor;

    int fontSize = 12;
    void* font = NULL;

    int8_t encoderId = -1;
    uint8_t contextId = 0;

    std::string localeName = "default";

    int boxWidth = 20;

    std::string userCode = "--------"; // fallback until fetched
    std::chrono::steady_clock::time_point expiryTime;
    std::atomic<bool> fetching = false;

    std::string deviceCode;
    std::string tokenFile = "../.github_token";
    std::string accessToken;
    enum class State {
        WaitingCode,
        LoadingToken,
        Authenticated,
    };
    std::atomic<State> state = State::WaitingCode;

    std::vector<std::string> repos;
    int currentRepoIndex = 0;
    bool reposLoaded = false;

    void setState(State newState)
    {
        setContext(contextId, (float)newState);
        state = newState;
        renderNext();
    }

    std::string fetchData(
        const std::string& url,
        const std::string& postData = "",
        const std::vector<std::pair<std::string, std::string>>& headers = {})
    {
#if USE_HTTPLIB
        try {
            httplib::Client cli(url.substr(0, url.find('/', 8))); // extract host
            cli.set_connection_timeout(5, 0);
            cli.set_read_timeout(5, 0);
            // cli.enable_server_certificate_verification(false);

            std::string path = url.substr(url.find('/', 8));
            httplib::Headers hdrs;
            for (auto& h : headers)
                hdrs[h.first] = h.second;

            httplib::Result res;
            if (postData.empty())
                res = cli.Get(path.c_str(), hdrs);
            else
                res = cli.Post(path.c_str(), hdrs, postData, "application/x-www-form-urlencoded");

            if (!res || res->status != 200)
                throw std::runtime_error("HTTP request failed");

            return res->body;
        } catch (const std::exception& ex) {
            logError("HTTP fetch failed: %s", ex.what());
            return "";
        }
#else
        // Build wget command
        // std::string cmd = "wget --quiet --output-document=- --no-check-certificate ";
        std::string cmd = "wget --quiet --output-document=- ";
        for (auto& h : headers)
            cmd += "--header=\"" + h.first + ": " + h.second + "\" ";
        if (!postData.empty())
            cmd += "--post-data=\"" + postData + "\" ";
        cmd += url;

        return execCmd(cmd);
#endif
    }

    void fetchCodeAsync()
    {
        if (fetching)
            return;
        fetching = true;
        userCode = "--------";
        setState(State::WaitingCode);

        std::thread([this]() {
            try {
                std::string body = "client_id=Ov23liVWLp79r3lJpFK2&scope=repo";
                std::string result = fetchData(
                    "https://github.com/login/device/code",
                    body,
                    { { "Content-Type", "application/x-www-form-urlencoded" } });

                if (result.empty())
                    throw std::runtime_error("GitHub request failed");

                auto kv = parseFormUrlEncoded(result);
                this->userCode = kv["user_code"];
                this->deviceCode = kv["device_code"];
                int expiresIn = std::stoi(kv["expires_in"]);
                this->expiryTime = std::chrono::steady_clock::now() + std::chrono::seconds(expiresIn);

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
        setState(State::LoadingToken);

        std::thread([this]() {
            try {
                std::string body = "client_id=Ov23liVWLp79r3lJpFK2"
                                   "&device_code="
                    + deviceCode + "&grant_type=urn:ietf:params:oauth:grant-type:device_code";

                for (;;) {
                    std::string result = fetchData(
                        "https://github.com/login/oauth/access_token",
                        body,
                        { { "Content-Type", "application/x-www-form-urlencoded" },
                            { "Accept", "application/json" } });

                    if (result.empty())
                        throw std::runtime_error("GitHub token request failed");

                    auto json = nlohmann::json::parse(result);
                    if (json.contains("error")) {
                        std::string err = json["error"];
                        if (err == "authorization_pending") {
                            std::this_thread::sleep_for(std::chrono::seconds(5));
                            continue;
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

                        fetchReposAsync();
                        setState(State::Authenticated);
                    }
                    break;
                }
            } catch (const std::exception& ex) {
                logError("GitHub token fetch failed: %s", ex.what());
                setState(State::WaitingCode);
            }
        }).detach();
    }

    void fetchReposAsync()
    {
        if (accessToken.empty() || reposLoaded)
            return;

        std::thread([this]() {
            try {
                repos.clear();

                currentRepoIndex = 0;
                std::string activeRepo = getActiveRepo();
                int page = 1;

                for (int i = 0; i < 20; i++) { // max 20 pages = 2000 repos
                    std::string url = "https://api.github.com/user/repos?per_page=100&page=" + std::to_string(page);

                    std::string result = fetchData(
                        url,
                        "", // GET request → no POST data
                        {
                            { "Authorization", "token " + accessToken },
                            { "User-Agent", "zicbox-client" },
                            { "Accept", "application/vnd.github+json" } });

                    if (result.empty()) {
                        throw std::runtime_error("GitHub repos fetch failed");
                    }

                    auto json = nlohmann::json::parse(result);
                    if (json.empty())
                        break;

                    for (auto& repo : json) {
                        if (repo.contains("full_name")) {
                            repos.push_back(repo["full_name"]);
                            if (repo["full_name"] == activeRepo)
                                currentRepoIndex = repos.size() - 1;
                        }
                    }

                    if ((int)json.size() < 100)
                        break; // last page
                    page++;
                }

                if (!repos.empty()) {
                    reposLoaded = true;
                    renderNext();
                }

            } catch (const std::exception& ex) {
                logError("GitHub repos fetch failed: %s", ex.what());
            }
        }).detach();
    }

    void loadSelectedRepoAsync()
    {
        if (!isAuthenticated() || !reposLoaded || repos.empty()) {
            showMessage("No repo selected");
            return;
        }

        std::string repoName = repos[currentRepoIndex];
        if (repoName.empty() || repoName == "Local") {
            showMessage("Invalid repo");
            return;
        }

        std::thread([this, repoName]() {
            try {
                namespace fs = std::filesystem;
                showMessage("Loading repo...");

                std::string tmpDir = "/tmp/github_data_clone_" + std::to_string(getpid());
                // logDebug("GitHub: tmpDir=%s", tmpDir.c_str());

                // Clean tmp if exists
                execCmd("rm -rf " + tmpDir);

                // Prepare clone URL with token
                std::string cloneUrl = "https://x-access-token:" + accessToken + "@github.com/" + repoName + ".git";

                // Clone repo
                // std::string cloneCmd = "git -c http.sslVerify=false clone " + cloneUrl + " " + tmpDir + " 2>&1";
                std::string cloneCmd = "git clone " + cloneUrl + " " + tmpDir + " 2>&1";
                // logDebug("GitHub: cloneCmd=%s", cloneCmd.c_str());
                std::string output = execCmd(cloneCmd);

                if (output.find("denied") != std::string::npos || output.find("error") != std::string::npos || output.find("fatal") != std::string::npos) {
                    logError("Git clone failed: %s", output.c_str());
                    showMessage("Permission denied.");
                    execCmd("rm -rf " + tmpDir);
                    return;
                }

                // Remove credentials from .git/config to avoid saving token
                execCmd("git -C " + tmpDir + " remote set-url origin https://github.com/" + repoName + ".git");

                // Prepare data folders
                fs::path dataDir = "data";
                fs::path backupDir = "data_backup";

                if (fs::exists(backupDir)) {
                    execCmd("rm -rf data_backup");
                }

                if (fs::exists(dataDir)) {
                    fs::rename(dataDir, backupDir);
                }

                fs::rename(tmpDir, dataDir);

                showMessage("Loaded successfully");

                // Reload active repo
                fetchReposAsync();

            } catch (const std::exception& ex) {
                logError("Repo load failed: %s", ex.what());
                showMessage("Load failed");
            }
        }).detach();
    }

    // --- Helper to execute a shell command and capture stdout/stderr ---
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

    std::string lastMessage;
    void showMessage(const std::string& msg, int durationMs = 1000)
    {
        lastMessage = msg;
        renderNext(); // show message immediately

        std::thread([this, durationMs]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(durationMs));
            lastMessage = "";
            renderNext(); // trigger re-render when duration is over
        }).detach();
    }

    std::filesystem::path repoPath = "data";
    void pushToRepoAsync()
    {
        if (!isAuthenticated()) {
            showMessage("Not authenticated");
            return;
        }

        std::thread([this]() {
            try {
                namespace fs = std::filesystem;
                // Check if it's a git repo
                if (!fs::exists(repoPath / ".git")) {
                    showMessage("No git repo found");
                    return;
                }

                // Get current remote URL
                std::string remoteUrl = execCmd("git -C data remote get-url origin");
                remoteUrl.erase(remoteUrl.find_last_not_of(" \n\r\t") + 1); // trim whitespace/newline

                if (remoteUrl.empty()) {
                    showMessage("No remote URL");
                    return;
                }

                // Convert SSH or HTTPS URL to HTTPS with token
                if (remoteUrl.rfind("git@", 0) == 0) {
                    // git@github.com:user/repo.git -> https://github.com/user/repo.git
                    size_t colon = remoteUrl.find(':');
                    if (colon == std::string::npos)
                        throw std::runtime_error("Invalid remote URL");
                    remoteUrl = "https://github.com/" + remoteUrl.substr(colon + 1);
                }

                showMessage("Saving to " + remoteUrl.substr(19));

                // Inject token
                std::string pushUrl = "https://x-access-token:" + accessToken + "@" + remoteUrl.substr(8); // skip https://

                // Run add, commit, push
                // std::string cmd = "cd data && git add . && git commit -m 'Sync' --allow-empty && git -c http.sslVerify=false push " + pushUrl + " HEAD 2>&1";
                std::string cmd = "cd data && git add . && git commit -m 'Sync' --allow-empty && git push " + pushUrl + " HEAD 2>&1";
                std::string output = execCmd(cmd);

                if (output.find("denied") != std::string::npos || output.find("error") != std::string::npos || output.find("fatal") != std::string::npos) {
                    logError("Git push failed: %s", output.c_str());
                    showMessage("Permission denied.");
                } else {
                    showMessage("Pushed successfully");
                }

            } catch (const std::exception& ex) {
                logError("Push failed: %s", ex.what());
                showMessage("Push failed");
            }
        }).detach();
    }

    std::string getActiveRepo()
    {
        namespace fs = std::filesystem;
        if (!fs::exists(repoPath / ".git")) {
            return "";
        }

        // Get current remote URL
        std::string remoteUrl = execCmd("git -C data remote get-url origin");
        remoteUrl.erase(remoteUrl.find_last_not_of(" \n\r\t") + 1); // trim whitespace/newline

        if (remoteUrl.empty()) {
            return "";
        }

        if (remoteUrl.rfind("git@", 0) == 0) {
            // git@github.com:user/repo.git -> user/repo.git
            size_t colon = remoteUrl.find(':');
            if (colon == std::string::npos)
                return "";
            remoteUrl = remoteUrl.substr(colon + 1);
        } else {
            remoteUrl = remoteUrl.substr(19);
        }

        return remoteUrl.substr(0, remoteUrl.size() - 4); // - .git
    }

    bool isExpired()
    {
        return std::chrono::steady_clock::now() >= expiryTime;
    }

    bool isAuthenticated()
    {
        if (state == State::Authenticated && !accessToken.empty())
            return true;

        std::ifstream in(tokenFile);
        if (in) {
            std::getline(in, accessToken);
            if (!accessToken.empty()) {
                if (!reposLoaded) {
                    fetchReposAsync();
                }
                setState(State::Authenticated);
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
                        if (state != State::Authenticated) {
                            fetching = false;
                            fetchCodeAsync();
                            renderNext();
                        }
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
            if (action == ".save") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        pushToRepoAsync();
                    }
                };
            }
            if (action == ".load") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        loadSelectedRepoAsync();
                    }
                };
            }
            return func;
        })
        , bgColor(styles.colors.background)
        , textColor(styles.colors.text)
        , foregroundColor(lighten(styles.colors.background, 0.5))
        , scrollColor(lighten(styles.colors.background, 1.0))
        , activeColor(rgba(24, 141, 59, 1))
    {
        /*md md_config:Rect */
        nlohmann::json& config = props.config;

        /// The background color.
        bgColor = draw.getColor(config["bgColor"], bgColor); //eg: "#000000"

        /// The text color.
        textColor = draw.getColor(config["textColor"], textColor); //eg: "#000000"

        /// The foreground color.
        foregroundColor = draw.getColor(config["foregroundColor"], foregroundColor); //eg: "#000000"

        /// The scroll color.
        scrollColor = draw.getColor(config["scrollColor"], scrollColor); //eg: "#000000"

        /// The active color.
        activeColor = draw.getColor(config["activeColor"], activeColor); //eg: "#000000"

        /// The font of the text. Default is null.
        if (config.contains("font")) {
            font = draw.getFont(config["font"].get<std::string>().c_str()); //eg: "Sinclair_S"
            fontSize = draw.getDefaultFontSize(font);
        }

        /// The font size of the text.
        fontSize = config.value("fontSize", fontSize); //eg: 8

        /// The encoder id that will interract with this component.
        /*md   encoderId={0} */
        encoderId = config.value("encoderId", encoderId);

        /// Set context id shared with other components to share the state.
        contextId = config.value("contextId", contextId); //eg: 10

        /// Name of the default locale repo.
        localeName = config.value("default", localeName);

        /*md md_config_end */

        resize();
        if (isAuthenticated()) {
            setState(State::Authenticated);
        }
    }

    void resize() override
    {
        boxWidth = size.w / 9;
    }

    void render()
    {
        draw.filledRect(relativePosition, size, { bgColor });

        int textY = relativePosition.y + (size.h - fontSize) * 0.5;

        if (!lastMessage.empty()) {
            draw.textCentered({ relativePosition.x + size.w / 2, textY }, lastMessage, fontSize, { textColor, .font = font });
            return;
        }

        if (isAuthenticated()) {
            if (reposLoaded) {
                draw.filledRect(relativePosition, size, { foregroundColor });
                int scrollW = size.w * ((float)currentRepoIndex / (float)repos.size());
                draw.filledRect(relativePosition, { scrollW, 2 }, { scrollColor });
                std::string repoName = repos[currentRepoIndex];
                draw.text({ relativePosition.x + 4, textY }, repoName, fontSize, { textColor, .font = font });

                std::string currentRepo = getActiveRepo();
                // logDebug("currentRepo: %s vs repoName: %s", currentRepo.c_str(), repoName.c_str());
                if (repoName == currentRepo) {
                    draw.textRight({ relativePosition.x + size.w - 4, textY }, "Active", fontSize, { activeColor, .font = font });
                }
                return;
            }
            draw.textCentered({ relativePosition.x + size.w / 2, textY }, "Loading repos...", fontSize, { textColor, .font = font });
            return;
        }

        if (state == State::LoadingToken) {
            draw.textCentered({ relativePosition.x + size.w / 2, textY }, "Loading token...", fontSize, { textColor, .font = font });
            return;
        }

        if (userCode == "--------" || isExpired()) {
            fetchCodeAsync();
        }

        for (int i = 0; i < 9; i++) {
            Point pos = { relativePosition.x + i * boxWidth, relativePosition.y };
            Size boxSize = { boxWidth - 2, size.h };
            int textX = pos.x + boxSize.w * 0.5;
            if (i == 4) {
                draw.textCentered({ textX, textY }, "-", fontSize, { textColor, .font = font });
                continue;
            }

            draw.filledRect(pos, boxSize, { foregroundColor });

            std::string letter(1, userCode.size() > i ? userCode[i] : '-');
            draw.textCentered({ textX, textY }, letter, fontSize, { textColor, .font = font });
        }
    }

    void onEncoder(int8_t id, int8_t direction)
    {
        if (id == encoderId && reposLoaded && !repos.empty()) {
            currentRepoIndex += direction;
            if (currentRepoIndex < 0)
                currentRepoIndex = 0;
            if (currentRepoIndex >= (int)repos.size())
                currentRepoIndex = repos.size() - 1;
            renderNext();
        }
    }
};
