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
incorporate custom modes of operation provided the same addressing and timing is retained

## Objectives ##
1. MVP is to reproduce the existing 6847 as per the original Motorola datasheet using the RP2040 microcontroller
2. Add custom font definition
3. Modify palette definitions per mode
4. Create custom video modes following original timing
5. Create custom video modes with novel screen construction and timing

## Video Output ##
The 6847 has three output lines - Luminance (Y), ∅ A and ∅ B
The three signals combine to provide 9 colours - black, green, yellow, blue, red, buff (white), cyan, magenta
and orange

Y is a 6 level analogue signal, A is a 3 level analogue signal, B is a 4 level analogue signal. Not all signal
combinations are valid for the primary display area

| Colour  | Y | A | B |
|:--------|:-:|:-:|:-:|
| Black   | 5 |0/1| 2 |
| Green   | 1 | 0 | 0 |
| Yellow  | 0 | 1 | 0 |
| Blue    | 2 | 1 | 3 |
| Red     | 2 | 2 | 2 |
| Buff    | 0 | 1 | 2 |
| Cyan    | 1 | 0 | 2 |
| Magenta | 1 | 2 | 3 |
| Orange  | 1 | 2 | 0 |

Using the original datasheet for reference the default output is for NTSC and requires a timing phase difference
between the Y, A and B signals.
Y leads the trio, B follows, then A. The Y-B delay is half the rise/fall time of Y, while the B-A delay is fully
the sum of Y-B and the rise/fall time of B.
