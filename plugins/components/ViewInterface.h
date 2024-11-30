#ifndef _VIEW_INTERFACE_COMPONENT_H_
#define _VIEW_INTERFACE_COMPONENT_H_

#include <string>
#include <vector>

class ViewInterface {
public:
    DrawInterface& draw;

    ViewInterface(DrawInterface& draw)
        : draw(draw)
    {
    }

    virtual void setGroup(int8_t index) = 0;
    virtual void* getContainer(std::string name) = 0;
    virtual void pushToRenderingQueue(void* component) = 0;
    virtual std::vector<void*>* getComponents() = 0;
};

#endif
