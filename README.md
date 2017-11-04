What is dbgeth?
===============
It is planned as a kind of Ethernet protocol for my Raspberry PI 3.
When I first bought the RPi, I did not have a keyboard left and needed a special way to set it up.
I did not even have a monitor other except my tux(linux running on my laptop).
so, every initial progress was done by CLI and hand-by-hand...;

I installed raspbian stretch lite and modified /etc/rc.local file to open ssh access, 
but there was no way to watch its IP, so I needed a more obvious way. 
so, I forcibly assigned an IP and assigned the same band IP to my ethernet card in the same way.
However, there is a disadvantage in that DHCP is disabled if i keep this way to enjoy my RPi.

So, I've designed "dbgeth" and its protocol.

![Screenshot of dbgeth](http://cfile5.uf.tistory.com/image/99C5243359FD781836B9F6)


----------


How to use?
-------------
Basically, dbgeth supports two methods. One is listener mode, other is signaller mode.
> **Usage:**
> 
> - 1. Listener Mode: dbgeth NIC -l SCRIPT_FILE_1
> - 2. Signaller Mode: dbgeth NIC -s SCRIPT_FILE_2

**Descriptions:**
1) NIC (Network Interface Card): Network interface for receiving/transmitting dbgeth packets.
2) SCRIPT_FILE: Script file to be executed(actually, command to be executed) when they attached through dbgeth protocol.

**Message Flow:**
```sequence
User->Signaller: .
Signaller->Listener: Broadcasts E_DBGQRY_DISCOVERY packet
Note right of Signaller: Broadcasts every 1 seconds
Listener->Signaller: Respond E_DBGQRY_DISCOVERY packet.
Listener->RPi: Executes specified SCRIPT_FILE_1.
Signaller->User: Executes specified SCRIPT_FILE_2.
```


----------


Maybe...
-------------
I will improve this program to be more good!
