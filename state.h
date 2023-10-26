#ifndef _UI_STATE_H_
#define _UI_STATE_H_

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *texture = NULL;

// TODO make this configurable
uint8_t encoderCount = 4;
int8_t group = 0;

void setGroup(int8_t index)
{
    group = index == -1 ? 0 : index;
}

#endif