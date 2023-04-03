# Motorola 6847P Replacement #
## Background ##
The 6847P is fairly ubiquitous in the history of 70s/80s home computers and while it provides a simple solution to 
video signal generation although with fairly basic capabilities in comparison with the custom video solutions 
of other 80s home computers.

In terms of availability, the chip is easily obtained and due to reliability, rarely needs replacing. Compared
with the Sinclair ULA or Commodore VIC chips this is a bulletproof system *but* reliability does not equate to
desirability.

The PAL version of the output generated has a very limited palette when in higher resolution modes, the NTSC
version of the output utilises video artifacting to provide a wider capability but is still well behind its
competitors.

## Building a better 6847 ##
There are a number of things that can be done to improve on the design and the availability of high performance,
cheap microcontrollers means it is fairly simple to build a custom VDG that exhibits the same external behaviour
while operating in a completely custom manner internally.

Custom fonts are a basic starting point, custom palettes are equally simple but there is scope in the system to
incorporate custom modes of operation provided the same addressing and timing is retained.

The essential requirement is to replace the 6847P in the Dragon range of home computers - this significantly
narrows the functional requirements of the substitute. In this scenario all addressing is performed by the
accompanying SAM (MC6883) address multiplexing chip. The system also binds pins to further simplify
implementation - INV and DD6 are combined, as are ~A/S and DD7, and ~INT/EXT with GM0. Without this there are
simply not enough GPIO pins available on the pre-built RP Pico board (as opposed to the RP2040 chip itself).

## Objectives ##
1. MVP is to reproduce the existing 6847 as per the original Motorola datasheet using the RP2040 microcontroller
2. Add custom font definition
3. Modify palette definitions per mode
4. Create custom video modes following original timing
5. Create custom video modes with novel screen construction and timing

## Signal Output ##
The 6847 provides 13 lanes of address bus to reference memory - this maps to the first 8k of the regular 16bit
address space. If used in conjunction with the MC6883 SAM chip only the first bit is used to provided a clock
signal to an internal address counter. This allows the use of paged video memory - all on 512 byte boundaries, 
potentially anywhere in the full 64k address space.

In addition the chip provides a horizontal sync signal (consumed by the SAM)

Fully implemented the data output requires 14 gpio pins. Simplifying the implementation in conjunction with a
SAM reduces the pin count to just 2.

## Signal Input ##
The 6847 requires a clock input (CLK), 8 bits of data bus (DD0-DD7) and 8 bits of mode control:
* ~A/G = Alpha or Graphics
* ~A/S = Alpha or Semigraphics
* ~INT/EXT = Internal or External alphanumerics
* GM0 = Graphic Mode LSB
* GM1 = Graphic Mode Bit 1
* GM2 = Graphic Mode Bit 2 (MSB)
* CSS
* INV = Inverted Alphanumerics

~A/G, ~A/S, CSS and INV can be controlled on a character by character basis.

CSS selects between two possible alphanumeric colours - the effect 
will vary depending on the graphic mode selected

| ~A/G | ~A/S | ~INT/EXT | INV | GM2 | GM1 | GM0 | Mode                           | Colours | Page Size |
|:----:|:----:|:--------:|:---:|:---:|:---:|:---:|:-------------------------------|:-------:|:---------:|
|  0   |  0   |    0     |  0  |  X  |  X  |  X  | Internal Alphanumeric          |    2    |    512    |
|  0   |  0   |    0     |  1  |  X  |  X  |  X  | Internal Alphanumeric Inverted |    2    |    512    |
|  0   |  0   |    1     |  0  |  X  |  X  |  X  | External Alphanumeric          |    2    |    512    |
|  0   |  0   |    1     |  1  |  X  |  X  |  X  | External Alphanumeric Inverted |    2    |    512    |
|  0   |  1   |    0     |  X  |  X  |  X  |  X  | Semigraphics 4                 |    8    |   1024    |
|  0   |  1   |    1     |  X  |  X  |  X  |  X  | Semigraphics 6                 |    8    |   2048    |
|  1   |  X   |    X     |  X  |  0  |  0  |  0  | 64x64 Graphics                 |    4    |   1024    |
|  1   |  X   |    X     |  X  |  0  |  0  |  1  | 128x64 Graphics                |    2    |   1024    |
|  1   |  X   |    X     |  X  |  0  |  1  |  0  | 128x64 Graphics                |    4    |   2048    |
|  1   |  X   |    X     |  X  |  0  |  1  |  1  | 128x96 Graphics                |    2    |   1536    |
|  1   |  X   |    X     |  X  |  1  |  0  |  0  | 128x96 Graphics                |    4    |   3072    |
|  1   |  X   |    X     |  X  |  1  |  0  |  1  | 128x192 Graphics               |    2    |   3072    |
|  1   |  X   |    X     |  X  |  1  |  1  |  0  | 128x192 Graphics               |    4    |   6144    |
|  1   |  X   |    X     |  X  |  1  |  1  |  1  | 256x192 Graphics               |    2    |   6144    |
 
The control input consumes 15 GPIO pins.

## Video Output ##
The 6847 has three output lines - Luminance (Y), ∅ A and ∅ B.

The three signals combine to provide 9 colours - black, green, yellow, blue, red, buff (white), cyan, magenta
and orange.

Y is a 6 level analogue signal, A is a 3 level analogue signal, B is a 4 level analogue signal. Not all signal
combinations are valid for the primary display area. For the main display only 4 levels are needed, the other
two levels are used for signal blanking and sync.

| Colour  | Y |  A  | B |  
|:--------|:-:|:---:|:-:|  
| Black   | 5 | 0/1 | 2 |  
| Green   | 1 |  0  | 0 |  
| Yellow  | 0 |  1  | 0 |  
| Blue    | 2 |  1  | 3 |  
| Red     | 2 |  2  | 2 |  
| Buff    | 0 |  1  | 2 |  
| Cyan    | 1 |  0  | 2 |  
| Magenta | 1 |  2  | 3 |  
| Orange  | 1 |  2  | 0 |  

Using the original datasheet for reference the default output is for NTSC and requires a timing phase 
difference between the Y, A and B signals. Y leads the trio, B follows, then A. The Y-B delay is half 
the rise/fall time of Y, while the B-A delay is fully the sum of Y-B and the rise/fall time of B.

To achieve an analogue signal output from the microcontroller the signal must be synthesized from a 
group of digital GPIO pins.

For the 6 level Y output 3 pins are needed (actually achieves 8 levels). For A and B where 3 or 4 levels are 
required then just 2 gpio pins each is needed. This consumes a total of 7 GPIO pins.

## Total GPIO Pin Requirements ##

Data input needs 14 or 2 GPIO pins - for the purposes of the MVP this will use the simple 2 pin
version. Control input requires 15 GPIO pins. Video output requires 7 pins. The grand total of 24 pins.

The initial choice of the RP2040 provides 30 GPIO pins - this is adequate for the simple implementation but is
inadequate for the full pin requirement of 36 pins.

### 6847 to GPIO pin mapping ###

|  6847 pin   |   RP2040 pin    |   RP Pico Pin    |
|:-----------:|:---------------:|:----------------:|
|    VSS 1    |     VSS 57      |      Gnd 38      |
|    DD6 2    |     GPIO6 8     |      GP6 9       |
|    DD0 3    |     GPIO0 2     |      GP0 1       |
|    DD1 4    |     GPIO1 3     |      GP1 2       |
|    DD2 5    |     GPIO2 4     |      GP2 4       |
|    DD3 6    |     GPIO3 5     |      GP3 5       |
|    DD4 7    |     GPIO4 6     |      GP4 6       |
|    DD5 8    |     GPIO5 7     |      GP5 7       |
|    CHB 9    |     GPIO11      |     GP11 15      |
|    B 10     | GPIO18-19 29-30 |  GP18-19 24-25   |
|    A 11     | GPIO20-21 31-32 |  GP20-21 26-27   |
|   ~MS 12    |    GPIO13 16    |     GP13 17      |
|   DA5 13    |        -        |        -         |
|   DA6 14    |        -        |        -         |
|   DA7 15    |        -        |        -         |
|   DA8 16    |        -        |        -         |
|   VCC 17    |        ?        |     Vbus 40      |
|   DA9 18    |        -        |        -         |
|   DA10 19   |        -        |        -         |
|   DA11 20   |        -        |        -         |
|   DA12 21   |        -        |        -         |
|   DA0 22    |    GPIO17 28    |     GP17 22      |
|   DA1 23    |        -        |        -         |
|   DA2 24    |        -        |        -         |
|   DA3 25    |        -        |        -         |
|   DA4 26    |        -        |        -         |
|   GM2 27    |    GPIO10 13    |     GP10 14      |
|    Y 28     | GPIO26-28 38-40 | GP26-28 31,32,34 | 
|   GM1 29    |    GPIO9 12     |      GP9 12      |
|   GM0 30    |    GPIO8 11     |      GP8 11      |
| ~INT/EXT 31 |    GPIO8 11     |      GP8 11      |
|   INV 32    |     GPIO6 8     |      GP6 9       |
|   CLK 33    |    GPIO15 18    |     GP15 20      |
|   ~A/S 34   |     GPIO7 9     |      GP7 10      |
|   ~A/G 35   |    GPIO14 17    |     GP14 19      |
|   ~RP 36    |        -        |        -         |
|   ~FS 37    |    GPIO13 16    |     GP13 17      |
|   ~HS 38    |    GPIO16 27    |     GP16 21      |
|   CSS 39    |    GPIO17 28    |     GP17 22      |
|   DD7 40    |     GPIO7 9     |      GP7 10      |

Unused GPIO pins: 12 22 (23 24)

## 6847 video output levels ##

| Color     | Y	     | phi A | phi B |
|:----------|:------:|:-----:|:-----:|
|Green	    | 0.54   | 1.0   | 1.0   |
|Yellow	    | 0.42   | 1.0   | 1.5   |
|Blue	    | 0.72   | 2.0   | 1.5   |
|Red	    | 0.72   | 1.5   | 2.0   |
|Buff	    | 0.42   | 1.5   | 1.5   |
|Cyan	    | 0.54   | 1.5   | 1.5   |
|Magenta	| 0.54   | 2.0   | 2.0   |
|Orange	    | 0.54   | 1.0   | 2.0   |
|Black	    | 0.72   | 1.5   | 1.5   |
|Dark Green	| 0.72   | 1.0   | 1.0   |
|Dark Orange| 0.72   | 1.0   | 2.0   |

Values for Y, phi A and phi B are in volts

### Y DAC ###
The 3 GPIO outputs can be converted to an analogue output using a resistor ladder array

GPIO 26 - 2kohm

GPIO 27 - 1kohm

GPIO 28 - 499ohm

Result is 8 levels from 0 to 7 - only the values 0 to 5 are needed. The output at level 
5 needs to be peak output, to achieve this some level mapping is needed.

| level | pin 2 | pin 1 | pin 0 | target | output |
|:-----:|:-----:|:-----:|:-----:|:------:|:------:|
| 0     | 0     | 0     | 0     | 0%     | 0%     |
| 1     | 0     | 0     | 1     | 20%    | 14.2%  |
| 2     | 0     | 1     | 1     | 40%    | 42.8%  |
| 3     | 1     | 0     | 0     | 60%    | 57.1%  |
| 4     | 1     | 1     | 0     | 80%    | 85.7%  |
| 5     | 1     | 1     | 1     | 100%   | 100%   |

### A and B DAC ###
The outputs both use the same arrangement with just 2 pins each

GPIO 18, 20 - 1kohm

GPIO 19, 21 - 499ohm 

Result on each pair is 4 levels from 0 to 3

The output for B can be direct but A only needs 3 levels which needs
mapping again.

| level | pin 21 | pin 20 | target | output |
|:-----:|:------:|:------:|:------:|:------:|
| 0     | 0      | 0      | 0%     | 0%     |
| 1     | 1      | 0      | 50%    | 66.6%  |
| 2     | 1      | 1      | 100%   | 100%   |

## Phase 2 ##
The next objective of development is to replace the LM889 composite modulator with the more modern AD724JR, 
the important detail here is that the AD724JR requires an RGB input instead of the YAB signals normally
produced by the original 6847P. This in turn means we have a proper RGB signal to consume externally, not 
just by the AD724JR which gives us two bonuses for the price of one.

RGB output from the RP2040 is an established capability so in many ways this is actually simplifying the
necessary work.

The complicating point here is that an 8 colour output is nice and simple and needs just 2 bits per 
component channel but the Dragon needs 9 colours (actually it is more than this as we have three shades of
black generated in normal output and two different shades of orange - even if the normal green and bright
orange are not generated on the same screen mode). Nonetheless, if the original output is to be retained
there must be scope in the output signal to do this.

Approximation of output levels per channel for each colour:

| Colour        | Red  | Green | Blue  |
|:-------------:|:----:|:-----:|:-----:|
| Black         | 0%   | 0%    | 0%    |
| Red           | 75%  | 0%    | 12%   |
| Green         | 0%   | 100%  | 0%    |
| Blue          | 12%  | 0%    | 75%   |
| Yellow        | 100% | 100%  | 25%   |
| Buff          | 100% | 100%  | 100%  |
| Orange        | 100% | 25%   | 0%    |
| Cyan          | 0%   | 87%   | 50%   |
| Magenta       | 100% | 0%    | 100%  |
| Green/Black   | 0%   | 25%   | 0%    |
| Orange/Black  | 37%  | 0%    | 0%    |
| Bright Orange | 100% | 75%   | 25%   |

The tricky part comes from the orange and off-black colours. All the others can be treated as simple, single bit output.
All three channels require 3 bits for output, two more than the YAB output in phase 1. Fortunately we have two unused
GPIO pins, and better still the desired output levels match up with the bit step values (close enough to look genuine).

Again the signal pins are building a signal through a resistor ladder:

Pin 0 -> 2kohm
Pin 1 -> 1kohm
Pin 2 -> 499ohm

The outputs are just combined in the same way as before to create a linear DAC.

The input to the AD724JR need to be grounded via a 75ohm pull-down resistor on each signal line, and through a 0.1uF
capacitor. The concern is that the raw RGB signal is then attenuated for a parallel VGA output and may require amplifying
(to be tested). Without the AD724JR the output from the ladder DAC is sufficient to drive a VGA port directly.

The VGA output still needs a hsync and vsync signal as well, these too can be driven directly (with a 47ohm resistor each),
the input to the AD724JR shouldn't need the resistors in line.

## AD724JR Input ##

Beyond the RGB, Hsync and Vsync, the AD724JR video encoder doesn't need much extra. Two 5V inputs, two grounds, a timing
signal (4.43362MHz for PAL) and some fixed values for "Encode" (high for run, low for standby), "select" (low), and 
"standard" (low for PAL).

## AD724JR Output ##

The encoder provides three signals for two independent outputs. The only one of these needed is "COMP" - a combined
composite output. The other two are isolated chrominance and luminance outputs used in s-video.

Using the composite output on the AD724JR means the carefully controlled, phase matched Y/A/B output can be removed from 
the Pico's responsibility. The composite output can be fed directly to the monitor port instead, although this then would
require removing or disabling the onboard signal encoder (LM1889). The new output line can be supplied directly to the
video half of the power board (pin 6). If the modulator is to be retained it would need to be fed to the base pin of TR4,
this then allows the AD724JR to drive composite and rf output. The simplest point to inject the signal would be
pin 10 (todo: verify pin 10 or 11) of the vacated LM1889 position but this then feeds through a maze of crude amplifying
transistors (TR2 and TR3) likely to destroy the quality of the signal given this task has already been performed by the
AD724JR. Given the likelihood of needing a modulated RF output it is pretty much a moot point unless a need for nostalgic
completeness is present.

Note: the output levels are double the normal strength to accomodate 75ohm remote termination
