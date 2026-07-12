# Todo list

- scatter, when a scatter is activated, we could use encoder to modify live some values...
    - and could do Gater FX like on circuit rhytmh

- TODO do something with screenshot???

- TODO doc doc doc

- TODO live step recording ??? or capture mode?

- Preset

- wifi setting?
- freesound api...?

# Prompt

Descriptions...

Constraints:
Do not modify any other files without asking first.
Make the smallest possible changes.
Preserve existing behavior except for the new feature.
Do not do unecessary refactoring
Only change things which are needed to achieve this task
Do not run or build the application




Like on the Novation Circuit Rhythm, i would like to use the encoder to edit the scatter fx live. So when we start the scatter fx, it always use the same initial value, and then using the encoder to edit the params of the fx. Starting the scatter again will reset the params to the original state.
There are only 4 encoders, so only 4 params can be edited. Only the params from the latest activate scatter mode can be edited.

In zicXYv2/uiClips.h in draw function, under the info, we should see the params which can be edited when enable the scatter. Those params are only visible when a scatter is active.

You need to edit audio/Scatter.h to be able to make the FX editable and to reset their state when activating them.
