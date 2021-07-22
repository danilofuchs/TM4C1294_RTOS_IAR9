# Lab 05

## Goal

Write a concurrent program in which the data producer is an ISR and the
consumer entity is a thread.

On SW1 pressed, cause an ISR. On each interruption, increase a global counter
and put it in the communications buffer.

The task shall remove the value from the buffer and write its binary value
on the 4 builtin LEDs (4 least significant bits).

[Instructions (Portuguese)](./lab5.pdf)
