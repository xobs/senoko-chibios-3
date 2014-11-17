SenokoOS
========

The operating system for the Senoko power supply board.


IRQ Handlers
------------

* Serial Console - Handle the debug shell
* I2C Slave - Listen for I2C requests from host machine
* GPIO Buttons - Listen for reset / power / AC Present / EMERGENCY line
* Alarm Timer - Wake up main system if this timer is hit


Threads (handled by timers)
---------------------------

* Charger WDT - Continuously write charger charge voltage
* Battery level monitor - shut down the system if battery is too low, temp
	is too high.
* Power Button Timer - Count down five seconds if system is on,
	for hard shutdown in case of lockup.
* Shell (provides debug interaction)


GPIO Handlers (invoked by PLA IRQ)
----------------------------------

* AC unplug - Generate I2C interrupt
* Power button - On press, run power button event engine.


Debug Shell
-----------

There is a debug shell that allows for interaction with the Senoko OS.


I2C slave
---------

Senoko acts as an I2C slave device, to allow for things such as setting
time and date, and powering off the board.  You can also do initial battery
configuration over I2C.

Since Senoko is a multifunction device, functional areas are broken up into
banks of I2C registers.


    +------+-----------------------------------------------------------------+
    | Bank | Description                                                     |
    +------+-----------------------------------------------------------------+
    | 0x00 | General registers (ID, IRQ, etc.)                               |
    +------+-----------------------------------------------------------------+
    | 0x0f | Power management (state of AC, power, power button, etc.)       |
    +------+-----------------------------------------------------------------+
    | 0x10 | GPIO expansion                                                  |
    +------+-----------------------------------------------------------------+
    | 0x20 | Realtime clock RTC alarm, and watchdog                          |
    +------+-----------------------------------------------------------------+

The following registers are defined

    +------+-------------------+---------------------------------------------+
    | Reg  | Name              | Description                                 |
    +------+-------------------+---------------------------------------------+
    | 0x00 | Signature         | Always returns 'S' (0x53)                   |
    +------+-------------------+---------------------------------------------+
    | 0x01 | Version (Major)   | Returns Senoko major version                |
    +------+-------------------+---------------------------------------------+
    | 0x02 | Version (Minor)   | Returns Senoko minor version                |
    +------+-------------------+---------------------------------------------+
    | 0x03 | Board type        | ID code of this board:                      |
    |      |                   |   0 - Abbreviated Senoko (i.e. desktop)     |
    |      |                   |   1 - Full Senoko (i.e. laptop with battery)|
    +------+-------------------+---------------------------------------------+
    | 0x04 | Uptime (byte 1)   | Byte 1 of the Senoko uptime (seconds)       |
    +------+-------------------+---------------------------------------------+
    | 0x05 | Uptime (byte 2)   | Byte 2 of the Senoko uptime (seconds)       |
    +------+-------------------+---------------------------------------------+
    | 0x06 | Uptime (byte 3)   | Byte 3 of the Senoko uptime (seconds)       |
    +------+-------------------+---------------------------------------------+
    | 0x07 | Uptime (byte 4)   | Byte 4 of the Senoko uptime (seconds)       |
    +------+-------------------+---------------------------------------------+
    | 0x08 | IRQ enable        | Which IRQs to enable.                       |
    |      |                   |  Bits: xxxx arkg                            |
    |      |                   |    a - Alarm (for wake-alarm)               |
    |      |                   |    r - Regulator (i.e. AC plug status)      |
    |      |                   |    k - Keypad IRQ (e.g. power button)       |
    |      |                   |    g - GPIO event                           |
    +------+-------------------+---------------------------------------------+
    | 0x09 | IRQ status        | Which IRQs are currently firing.            |
    |      |                   |  Bits: xxxx arkg                            |
    |      |                   |    a - Alarm (for wake-alarm)               |
    |      |                   |    r - Regulator (i.e. AC plug status)      |
    |      |                   |    k - Keypad IRQ (e.g. power button)       |
    |      |                   |    g - GPIO event                           |
    |      |                   | Write 0 to clear a IRQs.                    |
    +------+-------------------+---------------------------------------------+
    | 0x0f | Power Control     | Reflects Senoko's current power status.     |
    |      |                   |  Bits: kxxb awpp                            |
    |      |                   |    p Power state.  Values:                  |
    |      |                   |      0 - System is powered up               |
    |      |                   |      1 - System is powered down             |
    |      |                   |      2 - System will power down for 250ms   |
    |      |                   |          and then power up again (reboot)   |
    |      |                   |      3 - Reserved                           |
    |      |                   |    w Watchdog enabled.  Values:             |
    |      |                   |      0 - Watchdog is disabled               |
    |      |                   |      1 - Watchdog is enabled                |
    |      |                   |    a AC plug status.  Values:               |
    |      |                   |      0 - AC is unplugged                    |
    |      |                   |      1 - AC is connected                    |
    |      |                   |    b Power button status.  Values:          |
    |      |                   |      0 - Power button released              |
    |      |                   |      1 - Power button pressed               |
    |      |                   |    k Key.  Must be set to 1 to change power |
    |      |                   |            state from 'powered up'.         |
    +------+-------------------+---------------------------------------------+
    | ...................................................................... |
    +------+-------------------+---------------------------------------------+
    | 0x10 | GPIO Direction    | Sets GPIO direction for each GPIO           |
    |      |                   |  Bits: 7654 3210                            |
    |      |                   |    0 - GPIO is an input                     |
    |      |                   |    1 - GPIO is an output                    |
    +------+-------------------+---------------------------------------------+
    | 0x11 | Reserved          |                                             |
    +------+-------------------+---------------------------------------------+
    | 0x12 | GPIO Value        | For inputs, the current value. For outputs, |
    |      |                   | write a value to this register to output.   |
    |      |                   |  Bits: 7654 3210                            |
    +------+-------------------+---------------------------------------------+
    | 0x13 | Reserved          |                                             |
    +------+-------------------+---------------------------------------------+
    | 0x14 | GPIO IRQ Rising   | Indicates whether an IRQ is generated       |
    |      |                   | on GPIO rising event                        |
    |      |                   |  Bits: 7654 3210                            |
    |      |                   |    0 - No interrupt is generated            |
    |      |                   |    1 - Generate an interrupt on rise        |
    +------+-------------------+---------------------------------------------+
    | 0x15 | Reserved          |                                             |
    +------+-------------------+---------------------------------------------+
    | 0x16 | GPIO IRQ Falling  | Indicates whether an IRQ is generated       |
    |      |                   | on GPIO falling event                       |
    |      |                   |  Bits: 7654 3210                            |
    |      |                   |    0 - No interrupt is generated            |
    |      |                   |    1 - Generate an interrupt on fall        |
    +------+-------------------+---------------------------------------------+
    | 0x17 | Reserved          |                                             |
    +------+-------------------+---------------------------------------------+
    | 0x18 | GPIO IRQ Status   | A given bit will be 1 if an event occurred. |
    |      |                   | Clear the bit by writing "0" to re-arm.     |
    +------+-------------------+---------------------------------------------+
    | 0x19 | Reserved          |                                             |
    +------+-------------------+---------------------------------------------+
    | 0x1a | GPIO Pull enable  | If a bit is set to 1, a pull up/down is     |
    |      |                   | enabled on the pin.                         |
    +------+-------------------+---------------------------------------------+
    | 0x1b | Reserved          |                                             |
    +------+-------------------+---------------------------------------------+
    | 0x1c | GPIO Pull dir     | If a bit is set to 1, a pullup will be set. |
    |      |                   | Otherwise, a pulldown will be set.          |
    +------+-------------------+---------------------------------------------+
    | 0x1d | Reserved          |                                             |
    +------+-------------------+---------------------------------------------+
    | ...................................................................... |
    +------+-------------------+---------------------------------------------+
    | 0x20 | RTC Seconds       | Number of seconds since 1 January 2014      |
    |      |                   | Bits 0-7                                    |
    +------+-------------------+---------------------------------------------+
    | 0x21 | RTC Seconds       | Bits 8-15                                   |
    +------+-------------------+---------------------------------------------+
    | 0x22 | RTC Seconds       | Bits 16-23                                  |
    +------+-------------------+---------------------------------------------+
    | 0x23 | RTC Seconds       | Bits 24-31                                  |
    +------+-------------------+---------------------------------------------+
    | 0x24 | Wake alarm        | Send an IRQ this many seconds from now      |
    |      |                   | Bits 0-7                                    |
    +------+-------------------+---------------------------------------------+
    | 0x25 | Wake alarm        | Bits 8-15                                   |
    +------+-------------------+---------------------------------------------+
    | 0x26 | Wake alarm        | Bits 16-23                                  |
    +------+-------------------+---------------------------------------------+
    | 0x27 | Wake alarm        | Bits 24-31                                  |
    +------+-------------------+---------------------------------------------+
    | 0x28 | Watchdog Seconds  | Number of seconds until watchdog resets     |
    |      |                   | the mainboard.  Write a new value to kick   |
    |      |                   | the watchdog.  Write 0 to disable.          |
    +------+-------------------+---------------------------------------------+
