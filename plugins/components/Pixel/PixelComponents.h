#ifndef _PIXEL_COMPONENTS_H_
#define _PIXEL_COMPONENTS_H_

#include "viewManager.h"

#include "./KeyInfoBarComponent.h"
#include "./DrumEnvelopComponent.h"
#include "./Encoder3Component.h"

void loadPixelComponents()
{
    ViewManager& viewManager = ViewManager::get();
    viewManager.plugins.push_back({ "KeyInfoBar", [](ComponentInterface::Props props) {
        return new KeyInfoBarComponent(props);
    } });

    viewManager.plugins.push_back({ "DrumEnvelop", [](ComponentInterface::Props props) {
        return new DrumEnvelopComponent(props);
    } });

    viewManager.plugins.push_back({ "Encoder3", [](ComponentInterface::Props props) {
        return new Encoder3Component(props);
    } });
}

#endif