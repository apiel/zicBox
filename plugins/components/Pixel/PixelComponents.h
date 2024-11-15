#ifndef _PIXEL_COMPONENTS_H_
#define _PIXEL_COMPONENTS_H_

#include "../../../viewManager.h"
#include "./KeyInfoBarComponent.h"

void loadPixelComponents()
{
    ViewManager& viewManager = ViewManager::get();
    viewManager.plugins.push_back({ "KeyInfoBar", [](ComponentInterface::Props props) {
        return new KeyInfoBarComponent(props);
    } });
}

#endif