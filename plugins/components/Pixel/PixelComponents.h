#ifndef _PIXEL_COMPONENTS_H_
#define _PIXEL_COMPONENTS_H_

#include "viewManager.h"

#define USE_COMPONENTS_SO
#ifdef USE_COMPONENTS_SO

void loadPixelComponents()
{
#ifdef IS_RPI
    std::string folder = "@/plugins/components/Pixel/build/arm/libzic_";
#else
    std::string folder = "@/plugins/components/Pixel/build/x86/libzic_";
#endif

    ViewManager& viewManager = ViewManager::get();

    viewManager.loadPlugin("TextGrid " + folder + "TextGridComponent.so");
    viewManager.loadPlugin("DrumEnvelop " + folder + "DrumEnvelopComponent.so");
    viewManager.loadPlugin("KnobValue " + folder + "KnobValueComponent.so");
    viewManager.loadPlugin("HiddenValue " + folder + "HiddenValueComponent.so");
    viewManager.loadPlugin("GraphEncoder " + folder + "GraphEncoderComponent.so");
    viewManager.loadPlugin("StepEdit " + folder + "StepEditComponent.so");
    viewManager.loadPlugin("StepEdit2 " + folder + "StepEdit2Component.so");
    viewManager.loadPlugin("StepEditDrum " + folder + "StepEditDrumComponent.so");
    viewManager.loadPlugin("StepEditMono " + folder + "StepEditMonoComponent.so");
    viewManager.loadPlugin("FmAlgo " + folder + "FmAlgoComponent.so");
    viewManager.loadPlugin("Adsr " + folder + "AdsrComponent.so");
    viewManager.loadPlugin("Text " + folder + "TextComponent.so");
    viewManager.loadPlugin("SeqBar " + folder + "SeqBarComponent.so");
    viewManager.loadPlugin("SeqProgressBar " + folder + "SeqProgressBarComponent.so");
    viewManager.loadPlugin("SeqSynthBar " + folder + "SeqSynthBarComponent.so");
    viewManager.loadPlugin("Spectrogram " + folder + "SpectrogramComponent.so");
    viewManager.loadPlugin("Value " + folder + "ValueComponent.so");
    viewManager.loadPlugin("Rect " + folder + "RectComponent.so");
    viewManager.loadPlugin("Clips " + folder + "ClipsComponent.so");
    viewManager.loadPlugin("Keyboard " + folder + "KeyboardComponent.so");
    viewManager.loadPlugin("List " + folder + "ListComponent.so");
    viewManager.loadPlugin("Workspaces " + folder + "WorkspacesComponent.so");
    viewManager.loadPlugin("Sample " + folder + "SampleComponent.so");
    viewManager.loadPlugin("Tape " + folder + "TapeComponent.so");
    viewManager.loadPlugin("SampleEditor " + folder + "SampleEditorComponent.so");
    viewManager.loadPlugin("MacroEnvelop " + folder + "MacroEnvelopComponent.so");
}

#else

#include "./AdsrComponent.h"
#include "./ClipsComponent.h"
#include "./DrumEnvelopComponent.h"
#include "./FmAlgoComponent.h"
#include "./GraphEncoderComponent.h"
#include "./HiddenValueComponent.h"
#include "./KeyboardComponent.h"
#include "./KnobValueComponent.h"
#include "./ListComponent.h"
#include "./MacroEnvelopComponent.h"
#include "./RectComponent.h"
#include "./SampleComponent.h"
#include "./SeqBarComponent.h"
#include "./SeqProgressBarComponent.h"
#include "./SeqSynthBarComponent.h"
#include "./SpectrogramComponent.h"
#include "./StepEdit2Component.h"
#include "./StepEditComponent.h"
#include "./StepEditDrumComponent.h"
#include "./StepEditMonoComponent.h"
#include "./TapeComponent.h"
#include "./SampleEditorComponent.h"
#include "./TextComponent.h"
#include "./TextGridComponent.h"
#include "./ValueComponent.h"
#include "./WorkspacesComponent.h"

void loadPixelComponents()
{
    ViewManager& viewManager = ViewManager::get();

    viewManager.plugins.push_back({ "TextGrid", [](ComponentInterface::Props props) {
                                       return new TextGridComponent(props);
                                   } });

    viewManager.plugins.push_back({ "Clips", [](ComponentInterface::Props props) {
                                       return new ClipsComponent(props);
                                   } });

    viewManager.plugins.push_back({ "DrumEnvelop", [](ComponentInterface::Props props) {
                                       return new DrumEnvelopComponent(props);
                                   } });

    viewManager.plugins.push_back({ "KnobValue", [](ComponentInterface::Props props) {
                                       return new KnobValueComponent(props);
                                   } });

    viewManager.plugins.push_back({ "HiddenValue", [](ComponentInterface::Props props) {
                                       return new HiddenValueComponent(props);
                                   } });

    viewManager.plugins.push_back({ "GraphEncoder", [](ComponentInterface::Props props) {
                                       return new GraphEncoderComponent(props);
                                   } });

    viewManager.plugins.push_back({ "StepEdit", [](ComponentInterface::Props props) {
                                       return new StepEditComponent(props);
                                   } });

    viewManager.plugins.push_back({ "StepEdit2", [](ComponentInterface::Props props) {
                                       return new StepEdit2Component(props);
                                   } });

    viewManager.plugins.push_back({ "StepEditDrum", [](ComponentInterface::Props props) {
                                       return new StepEditDrumComponent(props);
                                   } });

    viewManager.plugins.push_back({ "StepEditMono", [](ComponentInterface::Props props) {
                                       return new StepEditMonoComponent(props);
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

    viewManager.plugins.push_back({ "SeqBar", [](ComponentInterface::Props props) {
                                       return new SeqBarComponent(props);
                                   } });

    viewManager.plugins.push_back({ "SeqSynthBar", [](ComponentInterface::Props props) {
                                       return new SeqSynthBarComponent(props);
                                   } });

    viewManager.plugins.push_back({ "SeqProgressBar", [](ComponentInterface::Props props) {
                                       return new SeqProgressBarComponent(props);
                                   } });

    viewManager.plugins.push_back({ "Value", [](ComponentInterface::Props props) {
                                       return new ValueComponent(props);
                                   } });

    viewManager.plugins.push_back({ "Spectrogram", [](ComponentInterface::Props props) {
                                       return new SpectrogramComponent(props);
                                   } });

    viewManager.plugins.push_back({ "Rect", [](ComponentInterface::Props props) {
                                       return new RectComponent(props);
                                   } });

    viewManager.plugins.push_back({ "Keyboard", [](ComponentInterface::Props props) {
                                       return new KeyboardComponent(props);
                                   } });

    viewManager.plugins.push_back({ "List", [](ComponentInterface::Props props) {
                                       return new ListComponent(props);
                                   } });

    viewManager.plugins.push_back({ "Workspaces", [](ComponentInterface::Props props) {
                                       return new WorkspacesComponent(props);
                                   } });

    viewManager.plugins.push_back({ "Sample", [](ComponentInterface::Props props) {
                                       return new SampleComponent(props);
                                   } });

    viewManager.plugins.push_back({ "Tape", [](ComponentInterface::Props props) {
                                       return new TapeComponent(props);
                                   } });

    viewManager.plugins.push_back({ "SampleEditor", [](ComponentInterface::Props props) {
                                       return new SampleEditorComponent(props);
                                   } });

    viewManager.plugins.push_back({ "MacroEnvelop", [](ComponentInterface::Props props) {
                                       return new MacroEnvelopComponent(props);
                                   } });
}
#endif

#endif