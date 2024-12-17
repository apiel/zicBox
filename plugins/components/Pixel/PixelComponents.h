#ifndef _PIXEL_COMPONENTS_H_
#define _PIXEL_COMPONENTS_H_

#include "viewManager.h"

#include "./KeyInfoBarComponent.h"
#include "./DrumEnvelopComponent.h"
#include "./Encoder3Component.h"
#include "./GraphComponent.h"
#include "./StepEditComponent.h"
#include "./HiddenEncoderComponent.h"
#include "./FmAlgoComponent.h"
#include "./AdsrComponent.h"
#include "./TextComponent.h"
#include "./SeqSynthBarComponent.h"

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

    viewManager.plugins.push_back({ "HiddenEncoder", [](ComponentInterface::Props props) {
        return new HiddenEncoderComponent(props);
    } });

    viewManager.plugins.push_back({ "Graph", [](ComponentInterface::Props props) {
        return new GraphComponent(props);
    } });

    viewManager.plugins.push_back({ "StepEdit", [](ComponentInterface::Props props) {
        return new StepEditComponent(props);
    } });

    viewManager.plugins.push_back({ "FmAlgo", [](ComponentInterface::Props props) {
        return new FmAlgoComponent(props);
    } });

    viewManager.plugins.push_back({ "Adsr", [](ComponentInterface::Props props) {
        return new AdsrComponent(props);
    } });

    viewManager.plugins.push_back({ "Text", [](ComponentInterface::Props props) {
        return new TextComponent(props);
    } });

    viewManager.plugins.push_back({ "SeqSynthBar", [](ComponentInterface::Props props) {
        return new SeqSynthBarComponent(props);
    } });
}

#endif