// g++ -o SynthPd SynthPd.c -I/usr/include/pd -DPD

// #include "SynthPd.h"

#include <z_libpd.h>

int main(int argc, char* argv[])
{
    // SynthPd synthPd;

    libpd_init();
    // 0 is input channel but ultimately we could also use pd to process input audio
    libpd_init_audio(0, 2, 44100);

    return 0;
}
