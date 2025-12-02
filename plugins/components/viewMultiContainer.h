/** Description:
This C++ structure defines the `ViewMultiContainer` class, which serves as a central manager for arranging and controlling complex screen layouts. Unlike a simple display, this class specializes in handling multiple independent display areas, known as "Containers," within a single view.

Its primary function is coordination: when the application runs, it initializes and activates all the contained display sections. When it's time to draw the screen, or when the user provides input (such as pressing keys, using touch controls, or turning physical knobs/encoders), the `ViewMultiContainer` acts as the hub, directing the specific event or command to the correct internal Container for processing.

A crucial feature is its dynamic layout management. The class manages responsive design by calculating the exact vertical size and position of each visible Container. It intelligently divides the available screen space based on whether a Container demands a fixed height or a specific percentage of the remaining area, ensuring the entire view is filled correctly.

The class also includes robust mechanisms, like internal software locks, to ensure that user inputs and layout changes are handled safely, even in concurrent processing environments, preventing errors during rapid operation.

Tags: C++, Header, GUI, UserInterface, UI Framework, ComponentModel, ViewManagement, ContainerManagement, CompositePattern, Hierarchy, EventHandling, InputProcessing, LayoutManagement, ResponsiveDesign, Rendering, Concurrency, ThreadSafety, Mutex, EncoderInput, HardwareInterface, LibraryComponent, Frontend.
sha: f21582a74a452c9cd5d365edf5bf2e710b8a74311d02bc709cb7b1bc827cc1f4 
*/
#pragma once

#include "helpers/getTicks.h"
#include "plugins/components/container.h"
#include "plugins/components/view.h"

#include <mutex>
#include <vector>

class ViewMultiContainer : public View {
protected:
    std::vector<Container> containers;
    std::mutex m2;

    // there should be about 4 to 12 encoders, however with 256 we are sure to not be out of bounds
    uint64_t lastEncoderTick[256] = { 0 };

public:
    ViewMultiContainer(DrawInterface& draw,
        std::function<void(std::string)> setView,
        float* contextVar)
        : View(draw, setView, contextVar)
    {
    }

    void init() override
    {
        for (auto& c : containers) {
            c.init();
        }
    }

    void activate() override
    {
        for (auto& c : containers)
            c.activate();
    }

    std::vector<ComponentInterface*>& getComponents() override
    {
        static std::vector<ComponentInterface*> merged;
        merged.clear();

        for (auto& c : containers) {
            if (!c.isVisible()) continue; // <-------------- ??
            for (auto* comp : c.components)
                merged.push_back(comp);
        }
        return merged;
    }

    Container* addContainer(std::string& name, Point position, std::string height) override
    {
        containers.push_back(Container(draw, setView, contextVar, name, position, height));
        return &containers.back();
    }

    void addComponent(ComponentInterface* component, Container* container) override
    {
        if (container) {
            container->addComponent(component);
        }
    }

    void pushToRenderingQueue(void* component) override
    {
        for (auto& c : containers) {
            c.pushToRenderingQueue(component);
        }
    }

    void renderComponents(unsigned long now) override
    {
        for (auto& c : containers) {
            c.renderComponents(now);
        }

        draw.triggerRendering();
    }

    void onContext(uint8_t index, float value) override
    {
        bool needResize = false;
        for (auto& c : containers) {
            if (c.onContext(index, value) && c.needResize) {
                needResize = true;
                c.needResize = false;
            };
        }

        if (needResize) {
            resize();
        }
    }

    void onMotion(MotionInterface& motion) override
    {
        for (auto& c : containers) {
            c.onMotion(motion);
        }
    }

    void onMotionRelease(MotionInterface& motion) override
    {
        for (auto& c : containers) {
            c.onMotionRelease(motion);
        }
    }

    void onEncoder(int8_t id, int8_t direction, uint64_t tick) override
    {
        m2.lock();
        if (tick > 0) {
            direction = encGetScaledDirection(direction, tick, lastEncoderTick[id]);
        }
        lastEncoderTick[id] = tick;

        for (auto& c : containers) {
            c.onEncoder(id, direction, tick);
        }

        m2.unlock();
    }

    void onKey(uint16_t id, int key, int8_t state) override
    {
        unsigned long now = getTicks();
        m2.lock();
        for (auto& c : containers) {
            c.onKey(id, key, state, now);
        }
        m2.unlock();
    }

    void resize() override
    {
        m2.lock();

        int totalFixed = 0;
        float totalPercent = 0.f;

        for (auto& c : containers) {
            if (c.isVisible()) {
                if (c.fixedHeight > 0) {
                    totalFixed += c.fixedHeight;
                } else {
                    totalPercent += c.percentHeight;
                }
            } else {
                c.needResize = true;
            }
        }

        int available = draw.getScreenSize().h - totalFixed;
        float xFactor = draw.getxFactor();
        int currentY = 0;
        for (auto& c : containers) {
            if (!c.isVisible()) continue; // <----- TODO need to fix this, actually even if not visible, should still be resized
            int h = c.fixedHeight;
            if (c.percentHeight > 0.f) {
                if (totalPercent > 0)
                    h = int((c.percentHeight / totalPercent) * available);
                else
                    h = 0;
            }

            c.position.y = currentY;
            if (c.originalHeight == 0) c.originalHeight = h; // First time, initialize original height for responsive height using percent
            float yFactor = h / (float)c.originalHeight;
            c.resize(xFactor, yFactor);
            currentY += h;
        }

        m2.unlock();
        draw.renderNext();
    }

    const std::vector<EventInterface::EncoderPosition> getEncoderPositions() override
    {
        static std::vector<EventInterface::EncoderPosition> merged;
        merged.clear();

        for (auto& c : containers) {
            if (!c.isVisible()) continue; // <-------------- ?
            auto list = c.getEncoderPositions();
            merged.insert(merged.end(), list.begin(), list.end());
        }
        return merged;
    }

    int8_t getEncoderId(int32_t x, int32_t y, bool isMotion = false) override
    {
        for (auto& c : containers) {
            int8_t id = c.getEncoderId(x, y, isMotion);
            if (id > -2)
                return id;
        }
        return -2;
    }
};
