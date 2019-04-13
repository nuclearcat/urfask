# urfask
Miniature implementation of TX-only RadioHead ASK encoder for ASK RF modules.
I tested it with STM32 HAL w FreeRTOS, probably will work elsewhere.
Used with DWT_Delay package, as we need microsecond precision delays.
We are calculating data on fly (and such way saving on RAM, as we dont need to make large 100 byte buffer), 
and on STM32F103 CPU frequency is 72Mhz, which is more than enough to not introduce significant delays.
