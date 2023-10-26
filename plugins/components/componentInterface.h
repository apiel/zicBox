#ifndef _UI_COMPONENT_INTERFACE_H_
#define _UI_COMPONENT_INTERFACE_H_

#include "drawInterface.h"
#include "valueInterface.h"
#include "motionInterface.h"

#include <vector>

class ComponentInterface
{
protected:
    DrawInterface &draw;
    Styles &styles;
    virtual void render() = 0;

    AudioPlugin &(*getPlugin)(const char *name);
    void (*setGroup)(int8_t index);

public:
    std::vector<ValueInterface *> values;
    Point position;
    const Size size;
    bool needRendering = true;
    int8_t group = -1;

    struct Props
    {
        Point position;
        Size size;
        DrawInterface &draw;
        AudioPlugin &(*getPlugin)(const char *name);
        void (*setGroup)(int8_t index);
    };

    ComponentInterface(Props &props)
        : draw(props.draw),
          styles(props.draw.styles),
          getPlugin(props.getPlugin),
          setGroup(props.setGroup),
          position(props.position),
          size(props.size)
    {
    }

    virtual void triggerRenderer() = 0;
    virtual void renderNext() = 0;
    virtual void onMotion(MotionInterface &motion) = 0;
    virtual void handleMotion(MotionInterface &motion) = 0;
    virtual void onMotionRelease(MotionInterface &motion) = 0;
    virtual void handleMotionRelease(MotionInterface &motion) = 0;
    virtual void onEncoder(int id, int8_t direction) = 0;
    virtual bool config(char *key, char *value) = 0;
    virtual bool baseConfig(char *key, char *value) = 0;
    virtual void onGroupChanged(int8_t index) = 0;
    virtual void onUpdate(ValueInterface *value) = 0;
};

#endif
