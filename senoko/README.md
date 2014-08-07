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

* System-on-fire - Shut down charger, power
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
<<<<<<< HEAD

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
    | 0x03 | Uptime (byte 1)   | Byte 1 of the Senoko uptime (seconds)       |
    +------+-------------------+---------------------------------------------+
    | 0x04 | Uptime (byte 2)   | Byte 2 of the Senoko uptime (seconds)       |
    +------+-------------------+---------------------------------------------+
    | 0x05 | Uptime (byte 3)   | Byte 3 of the Senoko uptime (seconds)       |
    +------+-------------------+---------------------------------------------+
    | 0x06 | Uptime (byte 4)   | Byte 4 of the Senoko uptime (seconds)       |
    +------+-------------------+---------------------------------------------+
    | 0x07 | Power Control     | Reflects Senoko's current power status.     |
    |      |                   |  Bits: xxxx xwpp                            |
    |      |                   |    p Power state.  Values:                  |
    |      |                   |      0 - System is powered up               |
    |      |                   |      1 - System is powered down             |
    |      |                   |      2 - System will power down for 250ms   |
    |      |                   |          and then power up again (reboot)   |
    |      |                   |      3 - Reserved                           |
    |      |                   |    w Watchdog enabled.  Values:             |
    |      |                   |      0 - Watchdog is disabled               |
    |      |                   |      1 - Watchdog is enabled                |
    +------+-------------------+---------------------------------------------+
    | 0x08 | Watchdog Seconds  | Number of seconds until watchdog resets     |
    |      |                   | the mainboard.  Write a new value to update |
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

=======
>>>>>>> 4177a65a07b748bb28ca7f5533e1ca3dadba5e2c
