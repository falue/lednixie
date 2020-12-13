# Introduction

Use this to manually display any positive integer number on a LED Nixie array made by led-genial.de or similar:

https://www.led-genial.de/LED-Nixie-L-6-stelliger-Bausatz-inkl-LED-Basic-Controller

Usually, these types of products are for displaying the current time. If you want to display any other number, hook up the circuit boards to your arduino and modify this script to your needs.

The LEDs used here are controlled by WS2812 and the code is written for that.

My purpose was to have the possibility to "count up" and "count down" to a target number, entered via serial monitor. I added randomness to this animation, to have a more natural flow of numbers, as if it were a counter of something in the real world.

# Video
![gif](./readme/example.gif)

*Example without randomness*

# How to
You can send integers and other commands via serial monitor to the arduino, which in turn displays the target number on your LED Nixie.

There are a few settings to determine speed, style and colors.

The smallest possible number to display is 0 (no negative numbers).


# Serial monitor input and actions

| Input  | Action taken |
| :----- | :------------ |
| 1234   | Display 1234, use count up/count down animation from current number |
| (empty)| Skip count up/count down animation |
| -5     | Subtract 5 from current number, use animation |
| +5     | Add 5 to current number, use animation |
| abc    | Any other character(s): Add +1 to current number |