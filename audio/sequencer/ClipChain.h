#pragma once

#include <vector>

inline void addClipToChain(std::vector<int>& chain, int clipIdx)
{
    chain.push_back(clipIdx);
}

inline void removeClipFromChain(std::vector<int>& chain, int index)
{
    if (index >= 0 && index < (int)chain.size()) {
        chain.erase(chain.begin() + index);
    }
}

inline void clearChain(std::vector<int>& chain)
{
    chain.clear();
}

inline void toggleChainPlaying(bool& chainPlaying, int& chainActiveIdx)
{
    chainPlaying = !chainPlaying;
    chainActiveIdx = chainPlaying ? 0 : -1;
}
