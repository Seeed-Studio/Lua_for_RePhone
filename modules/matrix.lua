
local matrix = {}
local i2c_address = 33
local i2c_speed = 100   -- 100 kbps

-- display a character for some time (millisecond)
function matrix.char(c, duration)
    duration = duration or 1000
    command = string.char(0x80, string.byte(c, 1), (duration / 256) % 256, duration % 256)
    i2c.setup(i2c_address, i2c_speed)
    i2c.send(command)
end

function matrix.emoji(e, duration)
    duration = duration or 1000
    command = string.char(0x84, e, (duration / 256) % 256, duration % 256)
    i2c.setup(i2c_address, i2c_speed)
    i2c.send(command)
end

function matrix.string(s, speed)
    speed = speed or 100
    if string.len(s) > 4 then
        s = string.sub(s, 1, 4)
    end
    command = string.char(0x81, string.len(s)) .. s .. string.char((speed / 256) % 256, speed % 256)
    i2c.setup(i2c_address, i2c_speed)
    i2c.send(command)
end

return matrix

