# Lua_for_RePhone

## Requirements
+ [gcc-arm-embedded](https://launchpad.net/gcc-arm-embedded)
+ make

Add gcc-arm-embedded path to PATH variable.

# Compile & Download
Run `make` to get lua.vxp file, follow [the guide](http://www.seeedstudio.com/wiki/Lua_for_RePhone#Download_Lua_for_RePhone_Application) to download it to the RePhone. 

1. [Upgrade firmware](http://www.seeedstudio.com/wiki/Arduino_IDE_for_RePhone_Kit#Update.2FFlash_the_Firmware) to get serial console support
2. Hold the RePhone's button, and turn it into storage mode in which we'll be able to see a USB disk on the computer.
3. Copy the vxp file to the disk
4. Change the path of `autostart.txt`, which is in the root directory of the disk, with the new vxp file path. 

## Get started with Lua

>Note:
>To use the serial console, [a firmware upgrade](http://www.seeedstudio.com/wiki/Arduino_IDE_for_RePhone_Kit#Update.2FFlash_the_Firmware) is a must. 


Use serial port tool like miniterm.py to open RePhone USB Modem Port and enter:
```
print('hello, lua')
```

## API
+ audio
    - audio.play(music.mp3)
    - audio.pause()
    - audio.resume()
    - audio.stop()
    - audio.set_volome(n), n from 1 to 6
    - audio.get_volome()
    
+ gsm
    - gsm.call(phone_number)
    - gsm.hang()
    - gsm.accept()
    - gsm.on_incoming_call(function (phone_number) print('incoming call') end)
    - gsm.text(phone_number, message)
    - gsm.on_new_message(function (phone_number, message) print('got a message') end)
    
+ timer
    - id = timer.create(interval, repeat_function)
    - timer.delete(id)
    
+ gpio
    - gpio.mode(pin, mode) - mode: gpio.INPUT, gpio.OUTPUT, gpio.INPUT_PULLUP
    - gpio.read(pin)
    - gpio.write(pin, value)
    
+ i2c
    - i2c.setup(address, speed)
    - i2c.send(number [, string, table, ...])
    - i2c.recv(size) - return string
    - i2c.txrx(number [, string, table, ...], size)
    
+ screen
    - screen.init()
    - screen.set_color(rgb)
    - screen.get_color()
    - screen.point(x, y)
    - screen.line(x1, y1, x2, y2 [, rgb])
    - screen.rectangle(x, y, width, height [, rgb])
    - screen.fill(x, y, width, height [, rgb])
    - screen.update()
    - screen.set_brightness()
    - screen.touch()

+ tcp
    - s = tcp.connect(ip, port)
    - tcp.write(s, str)
    - tcp.read(s, size)
    - tcp.close(s)
    
+ https
    - https.get(url, callback)
    
