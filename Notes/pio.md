## Timing

Except for wait and push, all instructions are exactly 1 clock cycle

## Syntax

instructions are formatted:
`<instruction> <values> [<delay>]`

value syntax are specific to the instruction

## Registers

* OSR - output shift register
* ISR - input shift register
* X
* Y

## Instructions

* JMP <label>- conditional jump (on EQ)
* WAIT - wait for condition to resolve: wait <polarity> gpio <gpio_num> or wait <polarity> pin <pin_num> or wait <polarity>irq <irq_num>
* IN - shift <n> bits from source into ISR: source is PINS, X, Y, Null, ISR, OSR: <n> is 1..32
* OUT - shift <n> bits from OSR to destination: destination is PINS, X, Y, Null, PINDIRS, PC, ISR, EXEC: <n> is 1..32
* PUSH - push ISR into RX FIFO subject to condition: IfFull (1) do nothing, and Block (1) stalls while RX FIFO is full
* PULL - pull from TX FIFO into OSR subject to condition: IfEmpty (1) do nothing, and Block (1) stalls while TX FIFO is empty, or (0) copies X to OSR
* MOV - Copy data to destination from source: source is PINS, X, Y, Null, Status, ISR or OSR; Destination is PINS, X, Y, EXEC, PC, ISR, OSR; copy operation is: none, invert (! or -), bit-reverse (::)
* IRQ - set or clear IRQ flag
* SET - write immediate value to destination: destination is PINS, X, Y, PINDIRS; data is 5-bit value

## Directives

* .define <symbol> <value>
* .program <name>
* .origin <offset>
* .side-set <count> (opt) (pindirs)
* .wrap_target
* .wrap (equivalent to BRA) to .wrap_target
* .lang_opt <lang> <name> <option>
* .word <value>

## Value Formats

* Integer
* Hexadecimal (0xNN)
* Binary (0bXXXX)
* symbol
* <label>
* (<expression>) - addition, subtraction, multiplication, integer division, negation, bit-reverse or value
