#include "audio/engines/Kick32v5.h"
#include "audio/engines/Acid32.h"
#include "audio/engines/Fm4.h"
#include "studio/zicStudioKick/emu/Sequencer.h"
#include "studio/zicStudioAcid/emu/AcidSequencer.h"
#include "studio/zicStudioFm4/emu/Fm4Sequencer.h"
#include "audio/effects/StudioFx.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>

int main() {
    // Kick
    {
        Kick32v5 e(44100.0f);
        e.noteOn(60,1.0f);
        float peak=0; bool finite=true;
        for (int i=0;i<44100;i++){ float s=e.sample(); if(!std::isfinite(s)) finite=false; peak=std::max(peak,std::fabs(s)); }
        std::cout<<"Kick peak "<<peak<<"\n";
        assert(finite && peak>0.001f);
    }
    // Acid
    {
        Acid32 e(44100.0f);
        e.noteOn(36,0.9f,false,true);
        float peak=0; bool finite=true;
        for (int i=0;i<44100;i++){ float s=e.sample(); if(!std::isfinite(s)) finite=false; peak=std::max(peak,std::fabs(s)); }
        std::cout<<"Acid peak "<<peak<<"\n";
        assert(finite && peak>0.001f);
    }
    // Fm4
    {
        Fm4 e(44100.0f);
        e.noteOn(60,1.0f);
        float peak=0; bool finite=true;
        for (int i=0;i<44100;i++){ float s=e.sample(); if(!std::isfinite(s)) finite=false; peak=std::max(peak,std::fabs(s)); }
        std::cout<<"Fm4 peak "<<peak<<"\n";
        assert(finite && peak>0.001f);
    }
    // Sequencers
    {
        Sequencer s(44100.0f); s.play();
        int fired=0; int visited[64]={};
        for (int i=0;i<44100*2;i++){ int n=0; if (s.tick(n)) fired++; int cs=s.current.load(); if (cs>=0 && cs<64) visited[cs]=1; }
        int v=0; for (int i=0;i<64;i++) v+=visited[i];
        std::cout<<"Kick seq fired "<<fired<<" visited "<<v<<"\n";
        assert(fired>0);
    }
    {
        AcidSequencer s(44100.0f); s.play();
        int on=0,steps=0; int vis[32]={};
        for (int i=0;i<44100*2;i++){ auto ev=s.tick(); if (ev.noteOn) on++; int cs=s.currentStep.load(); if (cs>=0&&cs<32) vis[cs]=1; }
        for (int i=0;i<32;i++) steps+=vis[i];
        std::cout<<"Acid seq on "<<on<<" steps "<<steps<<"\n";
        assert(on>0 && steps>0);
    }
    {
        Fm4Sequencer s(44100.0f); s.play();
        int on=0,steps=0; int vis[32]={};
        for (int i=0;i<44100*2;i++){ auto ev=s.tick(); if (ev.noteOn) on++; int cs=s.currentStep.load(); if (cs>=0&&cs<32) vis[cs]=1; }
        for (int i=0;i<32;i++) steps+=vis[i];
        std::cout<<"Fm4 seq on "<<on<<" steps "<<steps<<"\n";
        assert(on>0 && steps>0);
    }
    // Master FX bus: a delay tap must appear exactly delaySamples later
    {
        StudioFx fx;
        fx.delaySamples = 100;
        fx.delayLevel = 1.0f;
        fx.delayFeedback = 0.0f;
        fx.reverbAmount = 0.0f;
        float before = 0.0f, tap = 0.0f;
        for (int i = 0; i < 200; ++i) {
            float w = fx.process(i == 0 ? 0.5f : 0.0f);
            if (i == 99) before = w;
            if (i == 100) tap = w;
        }
        std::cout << "Delay tap " << tap << " (before " << before << ")\n";
        assert(std::fabs(before) < 1e-6f);
        assert(std::fabs(tap - 0.5f) < 1e-4f);
    }
    // Reverb + 8 track style mix: all finite, audible and bounded
    {
        Kick32v5 k(44100.0f);
        Acid32 a(44100.0f);
        Fm4 f(44100.0f);
        k.noteOn(60, 1.0f);
        a.noteOn(36, 0.9f, false, true);
        f.noteOn(60, 1.0f);

        StudioFx delay, reverb;
        delay.delaySamples = 3300;
        delay.delayFeedback = 0.35f;
        delay.delayLevel = 0.5f;
        reverb.reverbAmount = 0.55f;
        reverb.reverbLevel = 0.4f;

        float peak = 0.0f;
        bool finite = true;
        for (int i = 0; i < 44100; ++i) {
            float mono = (k.sample() + a.sample() * 0.55f + f.sample() * 0.55f) * 0.35f;
            float wet = delay.process(mono * 0.2f) + reverb.process(mono * 0.12f);
            float out = (mono + wet) * 0.8f;
            if (!std::isfinite(out)) finite = false;
            peak = std::max(peak, std::fabs(out));
        }
        std::cout << "Master mix peak " << peak << "\n";
        assert(finite && peak > 0.001f && peak < 4.0f);
    }
    std::cout<<"zicStudio DSP tests passed!\n";
    return 0;
}