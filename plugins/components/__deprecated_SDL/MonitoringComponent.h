/** Description:
This component defines a specialized display element, or widget, designed to visually report the utilization of the computer’s Central Processing Unit (CPU). It is intended for inclusion in user interfaces built using graphics libraries, likely for debugging or system monitoring dashboards.

### Functionality Overview

**1. Data Acquisition:**
The component operates by retrieving raw performance data directly from the underlying operating system environment (specifically, it accesses standard system files used by Linux to track CPU time). It collects statistics on how long the CPU has been actively working versus how long it has been idle.

**2. Usage Calculation:**
To determine the current usage percentage accurately, the component measures the *change* in CPU time between two moments. It saves the previous usage totals and compares them to the new totals. This comparison calculates the precise rate of CPU utilization over that short interval.

**3. Rendering and Display:**
The calculated usage percentage is then translated into a visual output. The component draws a graphical progress bar where the filled portion dynamically adjusts based on the percentage (e.g., 50% usage fills half the bar). Additionally, the exact percentage (e.g., "cpu: 45%") is displayed as text.

**4. Efficiency and Timing:**
The component is designed to be efficient by limiting its updates. It recalculates the CPU percentage and redraws the display only once every second, preventing unnecessary strain on the system while still providing near real-time feedback.

**5. Customization:**
Users can configure the appearance of the monitor, allowing them to define the specific colors used for the background, the active usage bar, and the display text.

sha: 07a9566fd15adf01fea7a27254287ae88e80fb871a57610bd3396f1130b103fb 
*/
#ifndef _UI_COMPONENT_MONITORING_H_
#define _UI_COMPONENT_MONITORING_H_

#include "../component.h"

#include <fstream>
#include <string>
// #include <iostream>
#include <numeric>
#include <unistd.h>
#include <vector>
#include "../utils/color.h"

/*md
## SDL Monitoring

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/SDL/Monitoring.png" />

Monitoring component show the current CPU usage.
*/
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
        return Colors({ darken(color, 0.75), darken(color, 0.65), color });
    }

    const int margin;

    uint8_t fontSize = 9;

public:
    MonitoringComponent(ComponentInterface::Props props)
        : Component(props)
        , colors(getColorsFromColor({ 0x80, 0x80, 0x80, 255 }))
        , margin(styles.margin)
    {
        fontSize = props.size.h * 0.5;

        jobRendering = [this](unsigned long now) {
            if (now - lastRendering > 1000) {
                lastRendering = now;
                renderNext();
            }
        };
    }

    bool config(char* key, char* value)
    {
        /*md - `COLOR: #999999` set component color */
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
        draw.filledRect(rectPos, rectSize, { colors.background });

        float percentage = getPercentage();
        draw.filledRect(rectPos, { (int)(rectSize.w * percentage), rectSize.h }, { colors.backgroundActive });

        int pct = percentage * 100;
        int x = position.x + size.w * 0.5;
        int y = position.y + (size.h - fontSize) * 0.5;
        draw.textCentered({ x, y }, std::string("cpu: " + std::to_string(pct) + "%").c_str(), fontSize, { colors.text });
    }
};

#endif
