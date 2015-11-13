
local blingbling = {}
local i2c_address = 35
local i2c_speed = 100   -- 100 kbps

function blingbling.on()
    i2c.setup(i2c_address, i2c_speed)
    i2c.send(0x81, 1)
end

function blingbling.off()
    i2c.setup(i2c_address, i2c_speed)
    i2c.send(0x81, 0)
end

-- set the color of a pixel
function blingbling.pixel(n, color)
    i2c.setup(i2c_address, i2c_speed)
    i2c.send(0x80, n, color.r, color.g, color.b)
end

function blingbling.monochrome(n, color, duration)
    duration = duration or 1000
    i2c.setup(i2c_address, i2c_speed)
    command = string.char(0x81, 2, n, color.r, color.g, color.b, duration / 256, duration % 256)
    i2c.send(command)
end

function blingbling.marquee(n, color, duration)
    duration = duration or 1000
    i2c.setup(i2c_address, i2c_speed)
    command = string.char(0x81, 3, n, color.r, color.g, color.b, duration / 256, duration % 256)
    i2c.send(command)
end

function blingbling.rainbow(n, duration)
    duration = duration or 1000
    i2c.setup(i2c_address, i2c_speed)
    i2c.send(0x81, 4, n, duration / 256, duration % 256)
end

function blingbling.rgb(red, green, blue)
    return {r=red; g=green; b=blue} 
end

return blingbling

