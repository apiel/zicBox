#ifndef _UI_COMPONENT_MONITORING_H_
#define _UI_COMPONENT_MONITORING_H_

#include "component.h"

#include <fstream>
#include <string>
// #include <iostream>
#include <numeric>
#include <unistd.h>
#include <vector>

class MonitoringComponent : public Component {
protected:
    size_t previous_idle_time = 0;
    size_t previous_total_time = 0;
    unsigned long lastRendering = 0;

    std::vector<size_t> getCpuTimes()
    {
        std::ifstream proc_stat("/proc/stat");
        proc_stat.ignore(5, ' '); // Skip the 'cpu' prefix.
        std::vector<size_t> times;
        for (size_t time; proc_stat >> time; times.push_back(time))
            ;
        return times;
    }

    bool getCpuTimes(size_t& idle_time, size_t& total_time)
    {
        const std::vector<size_t> cpu_times = getCpuTimes();
        if (cpu_times.size() < 4)
            return false;
        idle_time = cpu_times[3];
        total_time = std::accumulate(cpu_times.begin(), cpu_times.end(), 0);
        return true;
    }

    float getPercentage()
    {
        size_t idle_time, total_time;
        if (!getCpuTimes(idle_time, total_time)) {
            return 0.0f;
        }
        const float idle_time_delta = idle_time - previous_idle_time;
        const float total_time_delta = total_time - previous_total_time;
        const float utilization = 1.0 - idle_time_delta / total_time_delta;
        previous_idle_time = idle_time;
        previous_total_time = total_time;

        return utilization;
    }

    struct Colors {
        Color background;
        Color backgroundActive;
        Color text;
    } colors;

    Colors getColorsFromColor(Color color)
    {
        return Colors({ draw.darken(color, 0.75), draw.darken(color, 0.65), color });
    }

    const int margin;

    uint8_t fontSize = 9;

public:
    MonitoringComponent(ComponentInterface::Props& props)
        : Component(props)
        , colors(getColorsFromColor(styles.colors.grey))
        , margin(styles.margin)
    {
        fontSize = props.size.h * 0.5;

        jobRendering = [this](unsigned long now) {
            if (now - lastRendering > 1000) {
                lastRendering = now;
                return true;
            }
            return false;
        };
    }

    bool config(char* key, char* value)
    {
        if (strcmp(key, "COLOR") == 0) {
            colors = getColorsFromColor(draw.getColor(value));
            return true;
        }

        return false;
    }

    void render()
    {
        Point rectPos = { position.x + margin, position.y + margin };
        Size rectSize = { size.w - 2 * margin, size.h - 2 * margin };
        draw.filledRect(rectPos, rectSize, colors.background);

        float percentage = getPercentage();
        draw.filledRect(rectPos, { (int)(rectSize.w * percentage), rectSize.h }, colors.backgroundActive);

        int pct = percentage * 100;
        int x = position.x + size.w * 0.5;
        int y = position.y + (size.h - fontSize) * 0.5;
        draw.textCentered({ x, y }, std::string("cpu: " + std::to_string(pct) + "%").c_str(), colors.text, fontSize);
    }
};

#endif
