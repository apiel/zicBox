#ifndef _PIXEL_COMPONENTS_H_
#define _PIXEL_COMPONENTS_H_

#include "../../../viewManager.h"
#include "./KeyInfoBarComponent.h"

KeyInfoBarComponent* allocKeyInfoBar(ComponentInterface::Props props)
{
    return new KeyInfoBarComponent(props);
}

void loadPixelComponents()
{
    ViewManager& viewManager = ViewManager::get();
    // ViewManager::Plugin plugin = { "KeyInfoBar", allocKeyInfoBar };
    // ViewManager::Plugin plugin;
    // plugin.name = "KeyInfoBar";
    // plugin.allocator = (ComponentInterface * (*)(ComponentInterface::Props props))allocKeyInfoBar;
    // viewManager.plugins.push_back(plugin);

    viewManager.plugins.push_back({ "KeyInfoBar", (ComponentInterface * (*)(ComponentInterface::Props props)) allocKeyInfoBar });
}

#endif