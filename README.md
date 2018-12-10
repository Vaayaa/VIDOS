# VIDOS: Shader Based Video Synthesizer

![VidBoi 1](https://github.com/teafella/VIDOS/blob/master/images/IMGP1873.jpg?raw=true)
## Overview

The VIDOS is a flexible Raspberry Pi based video synthesizer with OSC & CV inputs. At its core the device is built around shaders (video card code) and uses them to generate the visuals. VIDOS is a great way to interface the world of shader visuals with analog CV equipment such as Eurorack synthesizers or anything that happens to speak OSC.

Turn the knobs and explore or plug in your eurorack to create your own audio/visual universe!

Features:
- Runs on a Pi Zero
- Bipolar -5V to +5V CV inputs (Eurorack friendly!)
- Instantly Updating Shader Code (Live Coding Encouraged)
- OSC support
	- SHAPEFINDER Layout for Lemur & TouchOSC
- Open Source

Future Features:
- A Shader Specialized Eurorack Module
- MIDI Support

## OSC Control
Do you have one of those addicting pocket thingies?! Send packets with an app like TouchOSC or Lemur and control that shader! Send stuff to /vidos/0 ... 7 to control the corresponding CVs in the .frag

You can find Layouts in the /OSCLayouts Folder

## SHAPEFINDER 
The VIDOS comes with the SHAPEFINDER shader and Layout as a starting point for your own creations

This shader should start automatically when you run the main.out

## Writing Your Own Shaders

VIDOS is basically just a Raspberry Pi in a convenient package that is ready to be programmed.

###OVER WIFI using Sublime Text editor
1. Get Sublime here: https://www.sublimetext.com/
2. Use Sublime's SFTP package: https://wbond.net/sublime_packages/sftp
3. Follow the directions to edit /VIDOS/MyShader0.frag


## Learning About Shaders
Sound intimidating? It isn't! Shader Language is only slightly different from regular C based coding and is super fun!

Check out: https://thebookofshaders.com/ and you will be creating your own craziness in a matter of minutes.

When you're ready to take the show on the road and/or open your creations up to the world of modular synthesizers, take that same code and plug it into the VIDOS.

## Hardware CV Control
The hardware device comes with bipolar CV inputs that can be used to control each of these knobs with voltage rather than turning them by hand. When an input is present the position of the knob represents the center or 0V state of the parameter.

Pair VIDOS with an external LFO for slow morphing changes or patch in your modular sequencer for rhythmic frames in sync with the rest of your patch.

# Installation
## Software
0. flash rasbian
8. git clone https://github.com/teafella/VIDOS
59. sudo apt-get install libsoil-dev libasound2-dev
68. cd VIDOS/
89. ./main.out

## Controlling the Shader (OSC SETUP)
1. Make sure your phone/pi are connected to thesame wifi network
2. Open A Lemur/TouchOSC layout on your phone
3. Point Lemur/TouchOSC at the pi (raspberrypi.local @ port 7000)

# OPTIONAL Setup
### Optional Installation (You probably already have this in default rasbian)
5. install wiringPI: http://wiringpi.com/download-and-install/

## Creating a Headless Raspberry Pi Zero (Ez Dev Ready Rig)

### Gear
- Pi Zero
- Micro USB cable
- Yurr Laptop

### Steps
1. OTG that thang:
		Guide: https://gist.github.com/gbaman/975e2db164b3ca2b51ae11e45e8fd40a 
2. Make sure you are sharing your wifi Connection: 
		OSX Guide: https://stevegrunwell.com/blog/raspberry-pi-zero-share-internet/
3. Log in with VNC: 
		Instructions: https://raspberrypi.stackexchange.com/questions/13986/how-to-have-remote-desktop-on-macbook-with-raspberry-pi

# Hardware (Dev Notes)

### MCP3008 Pinout
![VMCP3008](https://github.com/teafella/VIDOS/blob/master/images/MCP3008Pinout.gif)


## About The Creator
Ronald Sardarian is a musician, creative programmer and modular synthesizer enthusiast.

Check out [sardarian.wordpress.com](sardarian.wordpress.com) for more projects and [instagram.com/teafela/](instagram.com/teafela) for electronic music and visuals.


