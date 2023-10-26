#ifndef _OSC_CONTROLLER_H_
#define _OSC_CONTROLLER_H_

#include "controllerInterface.h"

#include <lo/lo.h>
#include <vector>
#include <stdio.h>

lo_server_thread serverThread;

void oscError(int num, const char *msg, const char *path)
{
    printf("liblo server error %d in path %s: %s\n", num, path, msg);
    fflush(stdout);
}

int midiOscHandler(const char *path, const char *types, lo_arg **argv, int argc, lo_message data, void *user_data);
int encoderOscHandler(const char *path, const char *types, lo_arg **argv, int argc, lo_message data, void *user_data);

class OscController : public ControllerInterface
{
protected:
    void start(const char *port)
    {
        if (serverThread)
        {
            lo_server_thread_free(serverThread);
        }

        serverThread = lo_server_thread_new(port, oscError);

        printf("OSC server listening on port %s\n", port);

        lo_server_thread_add_method(serverThread, "/midi", NULL, midiOscHandler, this);
        lo_server_thread_add_method(serverThread, "/encoder", "ii", encoderOscHandler, this);

        lo_server_thread_start(serverThread);
    }

public:
    OscController(Props &props) : ControllerInterface(props)
    {
        // printf("OSC server props : %d %d\n", props.midi, props.encoder);
        start("8888");
    }
};

int midiOscHandler(const char *path, const char *types, lo_arg **argv, int argc, lo_message data, void *user_data)
{
    std::vector<unsigned char> msg;
    for (int i = 0; i < argc; i++)
    {
        msg.push_back(argv[i]->c);
    }

    OscController *plugin = (OscController *)user_data;
    plugin->midi(&msg);

    return 0;
}

int encoderOscHandler(const char *path, const char *types, lo_arg **argv, int argc, lo_message data, void *user_data)
{
    OscController *plugin = (OscController *)user_data;
    plugin->encoder(argv[0]->i, argv[1]->i);

    return 0;
}

#endif