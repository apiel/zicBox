// g++ -o alsa_midi_example alsa_midi_example.cpp -lasound
#include <alsa/asoundlib.h>
#include <stdio.h>
#include <stdlib.h>

void list_midi_devices()
{
    snd_ctl_t* handle;
    snd_rawmidi_info_t* info;
    int card = -1;
    int err;

    snd_rawmidi_info_alloca(&info);

    printf("Available MIDI devices:\n");
    while (snd_card_next(&card) >= 0 && card >= 0) {
        char name[32];
        sprintf(name, "hw:%d", card);

        if ((err = snd_ctl_open(&handle, name, 0)) < 0) {
            fprintf(stderr, "Control open error: %s\n", snd_strerror(err));
            continue;
        }

        int device = -1;
        while (snd_ctl_rawmidi_next_device(handle, &device) >= 0 && device >= 0) {
            snd_rawmidi_info_set_device(info, device);
            snd_rawmidi_info_set_subdevice(info, 0);
            snd_rawmidi_info_set_stream(info, SND_RAWMIDI_STREAM_INPUT);
            if ((err = snd_ctl_rawmidi_info(handle, info)) < 0) {
                fprintf(stderr, "Control rawmidi info error: %s\n", snd_strerror(err));
                continue;
            }
            printf("Input MIDI Device: hw:%d,%d - %s\n", card, device, snd_rawmidi_info_get_name(info));

            snd_rawmidi_info_set_stream(info, SND_RAWMIDI_STREAM_OUTPUT);
            if ((err = snd_ctl_rawmidi_info(handle, info)) < 0) {
                fprintf(stderr, "Control rawmidi info error: %s\n", snd_strerror(err));
                continue;
            }
            printf("Output MIDI Device: hw:%d,%d - %s\n", card, device, snd_rawmidi_info_get_name(info));
        }

        snd_ctl_close(handle);
    }
    printf("____________________\n");
}

void read_midi_input(const char* device_name)
{
    snd_rawmidi_t* handle;
    if (snd_rawmidi_open(&handle, NULL, device_name, SND_RAWMIDI_NONBLOCK) < 0) {
        fprintf(stderr, "Error opening MIDI input device %s\n", device_name);
        return;
    }

    while (1) {
        unsigned char buffer[3];
        int n = snd_rawmidi_read(handle, buffer, sizeof(buffer));
        if (n > 0) {
            // Process the MIDI event (buffer contains the raw MIDI data)
            printf("Received MIDI event: %02x %02x %02x\n", buffer[0], buffer[1], buffer[2]);
        }
    }

    snd_rawmidi_close(handle);
}

void write_midi_output(const char* device_name, unsigned char* message, size_t length)
{
    snd_rawmidi_t* handle;

    if (snd_rawmidi_open(NULL, &handle, device_name, 0) < 0) {
        fprintf(stderr, "Error opening MIDI output device %s\n", device_name);
        return;
    }

    snd_rawmidi_write(handle, message, length);
    snd_rawmidi_drain(handle);
    snd_rawmidi_close(handle);
}

int main()
{
    list_midi_devices();

    // Example: Read from input device "hw:1,0"
    read_midi_input("hw:1,0");
    // read_midi_input("Arduino Leonardo");

    // Example: Write to output device "hw:1,0"
    unsigned char midi_message[] = { 0x90, 0x3C, 0x7F }; // Note On, Middle C, Full Velocity
    write_midi_output("hw:1,0", midi_message, sizeof(midi_message));

    return 0;
}
