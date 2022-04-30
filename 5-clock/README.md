# The new clock firmware

New firmware for the ESP8266 based _303 WIFI LC 01 clock_.

## nCLC - a basic NTP clock

A basic NTP clock is available in source [nCLC](nCLC) or as [binary](nCLC.ino.bin).
Features:

 - Pressing button SET brings the clock in configuration mode.
   In configuration mode, it becomes an access point and webserver.
   Via the webserver one can program the SSID/PASSWD of the home network, the **timezone**,
   and how time and date is rendered.
 - During normal operation, pressing DOWN steps the display brightness.
 - During normal operation, pressing UP toggles time and date.

In configuration mode, connect to the access point created by the nCLC,
named something like `nCLC-234C66`, then browse to 10.10.10.10 and start configuring.
The `Timezone` is the hardest field, its syntax is explained
in the [GNU manuals](https://www.gnu.org/software/libc/manual/html_node/TZ-Variable.html).

![configuration](nCLC-cfg.jpg)

To upload the binary, use the tool from [step 2](../2-fwbackup)
I had some trouble ("Possible serial noise or corruption.");
it might be wise to lower to `--baud 57600` or even `--baud 9600`.

Somehow, it didn't work. I needed to erase flash.

```cmd
(env) C:\Repos\303WIFILC01\2-fwbackup>python  env\Lib\site-packages\esptool.py  --port COM9  --baud 115200  erase_flash
esptool.py v3.3
Serial port COM9
Connecting................
Detecting chip type... Unsupported detection protocol, switching and trying again...
Connecting...
Detecting chip type... ESP8266
Chip is ESP8266EX
Features: WiFi
Crystal is 26MHz
MAC: 40:f5:20:23:3c:68
Uploading stub...
Running stub...
Stub running...
Erasing flash (this may take a while)...
Chip erase completed successfully in 0.0s
Hard resetting via RTS pin...

(env) C:\Repos\303WIFILC01\2-fwbackup>python  env\Lib\site-packages\esptool.py  --port COM9  --baud 57600  write_flash 0x0000 ..\5-clock\nCLC.ino.bin
esptool.py v3.3
Serial port COM9
Connecting.............
Detecting chip type... Unsupported detection protocol, switching and trying again...
Connecting...
Detecting chip type... ESP8266
Chip is ESP8266EX
Features: WiFi
Crystal is 26MHz
MAC: 40:f5:20:23:3c:68
Uploading stub...
Running stub...
Stub running...
Configuring flash size...
Flash will be erased from 0x00000000 to 0x00052fff...
Compressed 337024 bytes to 242773...
Wrote 337024 bytes (242773 compressed) at 0x00000000 in 45.3 seconds (effective 59.5 kbit/s)...
Hash of data verified.

Leaving...
Hard resetting via RTS pin...

(env) C:\Repos\303WIFILC01\2-fwbackup>
```


(end)

