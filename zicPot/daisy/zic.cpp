#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>

#include "audio/engines/PotKick.h"
#include "zicPot/sequenceBrain.h"
#include "zicPot/zicApp.h"
#include "zicPot/runtimeHardware.h"

using namespace daisy;

PotKick potKick(44100.0f);
SequenceBrain brain(44100.0f);
ZicApp app(brain, potKick);
HardwareDaisy hwDaisy;

void sendMidiByte(uint8_t byte)
{
    hwDaisy.sendMidiByte(byte);
}

// Audio Callback
static void AudioCallback(AudioHandle::InterleavingInputBuffer in,
                           AudioHandle::InterleavingOutputBuffer out,
                           size_t size)
{
    for (size_t i = 0; i < size; i += 2) {
        brain.processSample(potKick, sendMidiByte);

        float sampleVal = potKick.sample() * app.masterVolume;
        out[i] = sampleVal;
        out[i + 1] = sampleVal;
    }
}

int main(void)
{
    hwDaisy.init();
    hwDaisy.hw.StartAudio(AudioCallback);

    hwDaisy.processPots(app);
    hwDaisy.renderDisplay(app);

    while (1) {
        hwDaisy.encoder.Debounce();

        int32_t inc = hwDaisy.encoder.Increment();
        if (inc != 0) {
            app.handleEncoderTurn(inc > 0 ? 1 : -1);
            hwDaisy.renderDisplay(app);
        }

        if (hwDaisy.encoder.RisingEdge()) {
            app.handleEncoderClick(sendMidiByte);
            hwDaisy.renderDisplay(app);
        }

        hwDaisy.processPots(app);

        if (app.potOverlayTimer > 0) {
            app.potOverlayTimer -= 2;
            if (app.potOverlayTimer <= 0) {
                app.potOverlayTimer = 0;
                hwDaisy.renderDisplay(app);
            }
        }

        System::Delay(2);
    }
}
