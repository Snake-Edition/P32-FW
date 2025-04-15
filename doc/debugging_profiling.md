# Debugging & Profiling

How to debug or profile the printer (real hardware), step by step.

*Note: It is what worked for me. Systems are different and some modifications
might be needed.*

## Make OpenOCD work

*Note: Other people claim this works out of the box with VsCode.*

*Todo: There are some configs in utils/debug. They might be valid, I haven't
tried them.*

I'm using this config. I'm not the author and I don't know what every option
does, if it does anything any more, but it seems to work for me:

Note: The swo.out part no longer seems to work, the file happens to be the empty
all the time.


```
source [find interface/stlink.cfg]

set WORKAREASIZE 0x8000

#transport select "hla_swd"

set CHIPNAME STM32F407VGTx
set BOARDNAME genericBoard

# Enable debug when in low power modes
set ENABLE_LOW_POWER 1

# Stop Watchdog counters when halt
set STOP_WATCHDOG 1

# STlink Debug clock frequency
set CLOCK_FREQ 8000

# Reset configuration
# use hardware reset, connect under reset
# connect_assert_srst needed if low power mode application running (WFI...)
#reset_config srst_only srst_nogate connect_assert_srst
reset_config none
# reset_config srst_only srst_nogate connect_assert_srst
#set CONNECT_UNDER_RESET 1
#set CORE_RESET 0

source [find target/stm32f4x.cfg]
$_TARGETNAME configure -rtos FreeRTOS
tpiu config internal swo.out uart off 168000000 2000000
```

```
openocd -f config.cfg
```

This starts it and it keeps running (or not ‒ make sure the printer is
connected, etc...). Sometimes, this gets confused and needs to be restarted,
etc.

## Check that gdb works

This is just a test (for profiling) that OpenOCD works correctly.

Use whatever matches the firmware in the printer.

Some systems have something like `gdb-multiarch` or `gdb-armv7`, some (like
gentoo) have gdb that knows all the platforms.

```
gdb build/products/mk4_release_boot -ex 'target extended-remote localhost:3333'
```

Inside it, you can try getting backtrace and continue running, eg:

```
bt
c
```

*Warning: Stopping the printer in the middle of something can have undesired
consequences ‒ motors stopping abruptly and skipping steps, the beeper staying
in the On state and beeping continuously, similar thing happening to the nozzle
heater and melting the nozzle, etc.*

## Connect by "telnet" to it

(My OpenOCD listens for telnet connections on 4444, it writes this port on its
output, maybe it would be different somewhere else).

I'm using `socat`, but anything telnet-ish (`telnet`, `nc`) should work.

```
socat READLINE TCP-CONNECT:localhost:4444
```

## Ask it to collect some samples

Unfortunately, it collects only very few samples, so you get only few seconds.
So fire this at the right moment. The `2` is number of seconds to collect, but
making it longer won't collect more samples anyway :-(.

```
profile 2 output.gmon
```

### Read the output

```
gprof build/products/mk4_release_boot output.gmon
```
