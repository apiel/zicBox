# Zic XY Assembly

# **Introduction**

These instructions assume you already have the PCB, everything from this [bill of materials](https://github.com/apiel/zicBox/wiki/20-Zic-XY), and that you have followed the [instructions for installing the OS on you SD card](https://github.com/apiel/zicBox/wiki/20-Zic-XY).

This assembly was made using rev.3 of the Zic XY PCB, with some silly custom silkscreen art.  Please check the [Zic XY Wiki](https://github.com/apiel/zicBox/wiki/20-Zic-XY), or the discord if you are using a different version of the board.

| <img src="images/image6.png" width="60%"> <br> *PCB Front* |  <img src="images/image16.png" width="60%"> <br> *PCB Back* |
| :---: | :---: |

NOTE: The author is not a soldering expert… Apologies in advance for strange and ugly joints.

# **Tools**

* **Soldering iron** (of course)  
* **Flush cutters** (highly recommended).    
  There are places where it will be important that the pins don't stick out through the other side of the board.  
  <img src="images/image2.png" width="40%">  <img src="images/image21.png" width="40%">    
  
  Alternatively, you can precut the legs

# **Instructions**

## **■ Solder the 5V jumper (if you're not adding your own battery power)**

On the front of the board, solder the 2 pads below.  

*NOTE: If you're adding battery power, you shouldn't do this.*  
<img src="images/image17.png" width="60%"> 

## **■ Solder jumpers on the DAC**

On the back of the DAC, Solder jumpers as pictured in the yellow square below   

| <img src="images/image22.png" width="70%">  | 1-\>L<br>2-\>L<br>3-\>H<br>4-\>L  |
| :---- | :---: |

*More info here about the function of these jumpers here:*  
[https://github.com/apiel/zicBox/wiki/05-Hardware\#audio-dac](https://github.com/apiel/zicBox/wiki/05-Hardware#audio-dac)

## **■ Fit the 4 encoders into the PCB**

It is recommended that you leave them unsoldered for now.    
<img src="images/image29.png" width="70%"> 

## **■ Fit the left 10 keyswitches, leaving the 3 bottom right keys empty for now**

It is recommended that you leave them unsoldered until the rest of the assembly is complete.  The keyswitches in particular may be fine to never solder, if you are satisfied that they are fit securely.  
<img src="images/image10.png" width="70%"> 

**NOTE**  
You might notice that there are 3 pin-holes in the PCB for each keyswitch, but your switches may only have 2 pins.  That's OK. Just insert your switch as is. The 3rd hole is present for compatibility with other kinds of switches.

## **■ Solder 660ohm resistor into the 2 holes labeled R660, onto the *back* of the PCB**

Do your best to have the resistor legs / solder joints not stick up too much.  The screen will be placed flat against the PCB on top of where these legs come out. 

| <img src="images/image3.png" width="100%">  | <span style="font-size: 400%">➧</span> |<img src="images/image19.png" width="70%">  |
| :---: | :---: | :---: |

<img src="images/image8.png" width="70%"> 
<br>
<i>This is not great. The solder joints / resistor legs are poking out too much, and will interfere with screen sitting flat against the PCB.</i>

## **■ If your screen has pre-soldered header pins, carefully remove the black plastic base from the pins**

| <img src="images/image12.png" width="100%">  | <img src="images/image4.png" width="100%">  *Carefully remove this black plastic from the pins, so the screen can sit flush against the PCB* |
| :---- | :---- |

If you skip this step, the screen will not sit flush against the PCB, and is only held up by its header pins on the right side (see the gap pictured below).  You may be able to add in some kind spacer, but if you're planning on using the 3D printed case it may no longer fit.  
<img src="images/image32.png" width="70%"> 

## **■ Solder screen to front**

| <img src="images/image24.jpg" width="100%">  *Front of board with the screen* | <img src="images/image20.png" width="70%">  <br>*Back, showing pin holes.  The author chose here to leave the black plastic piece on.* |
| :---: | :---: |

## **■ Solder 90° 2x20 female pin header to *back* of board, with holes facing up**

This will be where the RPi will be plugged in.  It should be flush to the board.  
<img src="images/image28.png" width="30%"> <img src="images/image1.png" width="40%"> 
<p align="center">The header should sit flush to the board</p>

**NOTE:**
|<img src="images/image18.png" width="60%"> <br> It is recommended that you clip the legs that come out on the front of the board so the last 3 key switches can sit flush to the PCB. | <img src="images/image30.png" width="60%"><br>*Here, the unclipped pins are preventing the switch from sitting flush. Not the end of the world, but not ideal.*  |
| :---- | :---- |

## **■ Solder the 2x20 male pin headers to the top of the RPi, with the long parts of the legs pointing away from the ports**

<img src="images/image23.png" width="40%"> 

## **■ Plug the RPi into the 90° female header**

<img src="images/image5.png" width="50%"> 

## **■ Solder a USB C socket to the back of the board here** 

This is technically optional, but you'll need to solder an additional jumper somewhere here if you decide to skip.  
<img src="images/image33.png" width="60%"> 

## **■ Be sure to clip this encoder's pins, and left tab so they don't poke out on the back of the board**

This is to allow the header in the next step to sit flush against the PCB.  This is an instance where the flush cutters are helpful.  
<br>
<p align="center"><img src="images/image27.png" width="60%"> 
<br><br>
<img src="images/image11.png" width="60%">   
<br>
*Clip these so they don't stick out*
<br>
<span style="font-size: 500%">🠿</span> 
<br>
<img src="images/image25.png" width="60%"> 
</p>

## **■ Solder a  90° 1x6 female pin header here on the back of the board for the DAC**

<img src="images/image31.png" width="60%"> 

*If you don't have a 1x6 female header handy, You can cut a longer header to size.* 

## **■ Solder a 1x6 male pin header to the *top* of the DAC.**

<img src="../../../hardware/ZicXY/assembly/image9.png" width="60%"> 

## **■ Plug DAC into the female slot from previous step**

<img src="images/image14.png" width="60%"> 

## 

## **■ (optional) Solder slide switch here**

This is only recommended if you are adding your own battery power.  
You will need to bend the pins 90 degrees to make the switch through its slot in the 3D printed case if using.

<img src="images/image7.png" width="60%">  
<img src="images/image15.png" width="60%">  
<br> *The switch's opening on the 3D printed case*

## **■ If you don't solder a slide switch, solder a jumper between these 2 holes**

!<img src="images/image34.png" width="45%"> <span style="font-size: 300%">➧</span> <img src="images/image26.png" width="40%"> 

## **■ Attach the last 3 keys, and solder the encoders in place**

<img src="images/image13.png" width="60%"> 
The keys should pretty much work without being soldered, but the encoders will likely not work well until they have been soldered.

## **■ Done\! (Hopefully)**

**Troubleshooting:**  
Not booting up?

* Have you soldered all the necessary jumpers?  
* Does your RPI have a valid SD card?   
  (Even plugged into, LEDs might not light and it will look totally dead if not)

Keys/encoders not working?

* Check for broken traces on your board (this happened to the author)

No Audio?

* Double check that you've [soldered the jumpers correctly on the DAC](#bookmark=id.mpi7b1ljqzok)