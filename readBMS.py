from pymodbus.client.sync import ModbusSerialClient as ModbusClient

client = ModbusClient(method='rtu', port='COM4',stopbits=1, bytesize=8, parity='N',baudrate=19200, timeout=8)
client.connect()

sensor1 = client.read_input_registers(4,1,unit=1)  
datasensor1 = sensor1.registers[0] * 0.01
print (sensor1)
# print ("Sensor1", datasensor1)

# sensor2 = client.read_input_registers(1,1,unit=1)  
# datasensor2 = sensor2.registers[0] * 0.01
# print ("Sensor2 ", datasensor2)

client.close()
