# 23 Zic 23

<img src='https://github.com/apiel/zicBox/blob/main/zic23/zic23.png?raw=true' width='700'>

Zic23 is a desktop groovebox and drum machine built for Linux, powered by the ZicBox sound engines to deliver a flexible and expressive music-making experience. It features six independent tracks, each capable of hosting a variety of advanced synthesis and sound-generation engines, giving you a wide palette to explore. Every track comes equipped with its own EQ for precise shaping, while a master multi-mode filter and compressor provide cohesive control over the overall mix. To push things further in live performance, Zic23 includes playful scatter FX designed for real-time experimentation and creative disruption. Like everything in this repository, Zic23 is a proof of concept but still fun to try.

## How to try

Zic23 depends on a few common Linux audio and multimedia libraries:
- ALSA (audio output)
- libsndfile (audio file handling)
- SFML (graphics, windowing, system — for desktop UI)

Make sure you have them installed along with pkg-config.

On Debian / Ubuntu:
```sh
sudo apt install libasound2-dev libsndfile1-dev libsfml-dev pkg-config
```

**Build & run**, from the root of the repository:
```sh
make -C zic23
```

If everything is set up correctly, the project should build and you’ll be ready to start playing with Zic23.