ZicBoxXY2-MidiMIX changes nothing about the functionality of ZicBoxXYv2. 

I made these changes to enable it to run on a Raspberry Pi 4B 4GB with the MHS 3.5" SPI touchscreen at 480x320 resolution with touch functionality enabled.

For control, this version uses the Akai MidiMix controller with default settings. 

Top row knobs 1-4: Knob 1-4
Mute buttons 1-5: View, Up, Down, Mute, Menu
Rec Arm 1-8: Second row menu and track selection
Bank Left: Play/Stop
Bank Right: Shift

To use with any other controller, the CC assignments are all on channel 1:
Knobs: CC 16, 20, 24, 28. 
Top row menu buttons: Note 1, 4, 7, 10, 13
Bottom row buttons: Note 3, 6, 9, 12, 15, 18, 21, 24
Play/Stop: Note 26
Shift: Note 25

For audio, a cheap USB-Audio converter was used. 

Ensure both of these are connected at boot time.

You may need to do some troubleshooting on the controller assignments as I don't know whether the addressing changes between devices.

ALSA needs to be changed to use the USB-Audio converter instead of the default.

Startup script included.

To Install: Download ZicBoxXYv2, replace the included headers and build normally.

