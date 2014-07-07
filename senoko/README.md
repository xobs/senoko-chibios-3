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
