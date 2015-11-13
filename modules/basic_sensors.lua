
local sensors = {}
local i2c_address = 0x03
local i2c_speed = 100   -- 100 kbps

local id_table = {'acc_x'; 'acc_y', 'acc_z', 'light', 'temperature'}

function sensors.update()
    i2c.setup(i2c_address, i2c_speed)
    info = i2c.txrx(0, 4)
    f, ff, fff, n = string.byte(info, 1, 4)
    for i = 0, n, 1 do
        if f % 2 then
            f = f / 2
            sensor_data = i2c.txrx(i * 8 + 4, 8)
            if not sensor_data then
                return
            end
            t, r, id1, id2 = string.byte(sensor_data, 1, 4)
            v1, v2, v3, v4 = string.byte(sensor_data, 5, 8)
            v = v1 + v2 * 0x100 + v3 * 0x10000 + v4 * 0x1000000
            sensors[id_table[id1 + 1]] = v
        end
        
    end
end

function sensors.get_temperature()
    sensors.update()
    
    return sensors.temperature
end

function sensors.get_light()
    sensors.update()
    
    return sensors.light
end

function sensors.get_acc()
    sensors.update()
    
    return sensors.acc_x, sensors.acc_y, sensors.acc_z
end

return sensors

