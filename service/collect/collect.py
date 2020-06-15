'''
#########################################################
# File name: Collect.py                                 #
# Author: Yudha Bhakti                                  #
# Date created: 20/06/2019                              #
# Date last modified: 30/10/2019                        #
# Python Version: 3.6                                   #
# Purpose: Collect data from sensor                     #
#########################################################
'''
import time
import serial
import serial.tools.list_ports
import logging
import json
import threading
import math
import datetime
import os

#Modbus
from pymodbus.client.sync import ModbusSerialClient as ModbusClient

#Error Log File untuk file collect.py
logging.basicConfig(filename='collect.log', filemode='w', format='%(name)s - %(levelname)s - %(message)s')
logging.basicConfig(format='%(asctime)s - %(message)s', datefmt='%d-%b-%y %H:%M:%S')

class DataLogClass(object):
    #Menghapus file log berdasarkan tanggal pembuatannya
    def deletefilelog(self,deletelog):
        try:
            foldersekarang = os.path.dirname(os.path.realpath(__file__))
            foldersensor = os.path.abspath(os.path.join(foldersekarang, "..",".."))+'/data/collect/'  
            for file in os.listdir(foldersensor):
                file = os.path.join(foldersensor, file)
                if os.stat(file).st_mtime < time.time() - deletelog * 86400 and os.path.isfile(file):
                    os.remove(os.path.join(foldersensor, file))
            return os.listdir(foldersensor)
        except Exception:
            logging.exception("Exception occurred create deletefilelog")

class ParseClass(object):
    def __init__(self):
        self.modbus = SensorModbus() # Memanggil class modbus
        self.log = DataLogClass() # Memanggil class data log
        self.datajson = ''
        self.flagkirim = True

    #Menyimpan data sensor kedalam data log dalam format text
    def parse_datalog(self,data,id_perangkat):
        try:
            tanggalsekarang = datetime.datetime.now().strftime("%Y-%m-%d") # Mendapatkan parameter tanggal sekarang
            foldersekarang = os.path.dirname(os.path.realpath(__file__)) # Membuka direksi folder tempat file
            foldersensor = os.path.abspath(os.path.join(foldersekarang, "..",".."))+'/data/collect/' # Membuka direksi folder collect
            filesensor = os.path.join(foldersensor,id_perangkat+' '+tanggalsekarang+'.txt') # Membuat file log
            datasensor = open(filesensor,'a+') # Membuka file log
            datasensor.write(data + '\n') # Menulis data ke file log
            datasensor.close() # Menutup file
            self.log.deletefilelog(7) # Menghapus file 7 hari sebelum
            return filesensor
        except Exception:
            logging.exception("Exception occurred create datalog")

    #Fungsi untuk menyimpan data sensor dalam format data yang ditentukan dalam bentuk JSON
    def parse_data(self,id_perangkat,temperatur,baterai): 
        try:
            payload = {}
            payload_temperatur = {}
            payload_baterai = {}
            payload_temperatur['sensor_name'] = 'temperatur'
            payload_temperatur['value'] = temperatur
            payload_baterai['sensor_name'] = 'baterai'
            payload_baterai['value'] = baterai
            payload['device_id'] = str(id_perangkat)
            payload['sensors'] = payload_temperatur,payload_baterai
            payload['time']= str(datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')) + " +0700" 
            return (json.dumps(payload))
        except Exception:
            logging.exception("Exception occurred create json") 

    #Parsing data JSON dari serial
    def parse_json(self):
        try:
            for unit in range(1,4):
                if unit == 1:
                    self.id_perangkat = "Ruang Instrument"
                if unit == 2:
                    self.id_perangkat = "Ruang Panel Listrik 1"
                else:
                    self.id_perangkat = "Ruang Panel Listrik 2"
                temperatur,kelembapan = self.modbus.temperaturehumidity(unit)
                self.datajson = self.parse_data(self.id_perangkat,temperatur,baterai) # Parsing data kedalam bentuk JSON untuk dikirim ke backend
                self.parse_datalog(self.datajson,self.id_perangkat) # Parsing Data untuk disimpan di log file
                self.intervalkirim(datetime.datetime.now().second,self.datajson)
                time.sleep(0.5)
                print (self.datajson)
        except Exception:
            logging.exception("Exception occurred parse serial")

    #Fungsi untuk interval kirim ke Backend
    def intervalkirim(self,waktukirim,data):
        try:
            kirim = waktukirim % 30
            if kirim >= 0 and kirim <= 2 and self.flagkirim == True:
                self.datakirim(data)
                self.flagkirim = False
                #print (self.flagkirim)
            if kirim != 0:
                self.flagkirim = True
        except Exception:
            logging.exception("Exception occurred interval kirim") 

    #Menyimpan data sensor dalam data log pengiriman data dalam format text
    def datakirim(self,data):
        try:
            tanggalsekarang = datetime.datetime.now().strftime("%Y-%m-%d") #Mendapatkan parameter tanggal sekarang
            foldersekarang = os.path.dirname(os.path.realpath(__file__)) # Membuka direksi folder tempat file
            folderkirimsensor = os.path.abspath(os.path.join(foldersekarang, "..",".."))+'/data/connect/' # Membuka direksi folder connect
            filekirimsensor = os.path.join(folderkirimsensor,'Data '+tanggalsekarang+'.txt') # Membuat file kirim
            datasensor = open(filekirimsensor,'a+') # Membuka file kirim
            datasensor.write(data + '\n') # Menyimpan file kirim
            datasensor.close() # Menutup file kirim
            return filekirimsensor
        except Exception:
            logging.exception("Exception occurred create datakirim")

class SensorModbus(object):
    def __init__(self):
        self.serial_connection()
    
    #Fungsi Deteksi Serial Ports
    def serial_ports(self):
        import sys
        import glob
        try:
            if sys.platform.startswith('win'): # deteksi pada platform windows
                ports = ['COM%s' % (i + 1) for i in range(256)]
            elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'): # deteksi pada platfoem linux
                ports = glob.glob('/dev/tty[A-Za-z]*')
            elif sys.platform.startswith('darwin'): # deteksi pada platform darwin
                ports = glob.glob('/dev/tty.*')
            else:
                raise EnvironmentError('Unsupported platform')
            result = []
            for port in ports: # Membaca setiap port yang terdeteksi dan menyimpannya ke list
                try:
                    s = serial.Serial(port)
                    s.close()
                    result.append(port)
                except (OSError, serial.SerialException):
                    pass
            return result
        except Exception:
            logging.exception("Exception occurred serial_ports")
    
    #Fungsi Menghubungkan ke Koneksi Serial
    def serial_connection(self):
        try:
            for port in self.serial_ports():
                if 'ttyUSB' in port: # Mendeteksi ttyUSB terdeteksi
                    self.client = ModbusClient(method='rtu', port=str(port),stopbits=1, bytesize=8, parity='N',baudrate=19200, timeout=5)
                    self.client.connect()

        except Exception:
            logging.exception("Exception occurred serial_connect")

    def temperaturehumidity(self,unit):
        #Autonics
        # rawtemperature = self.client.read_input_registers(0,1,unit=1) #Register no 1 in datasheet size 1 words Device ID 1
        # temperature = round(rawtemperature.registers[0] * 0.01,2)
        # rawhumidity = self.client.read_input_registers(1,1,unit=1) #Register no 2 in datasheet size 1 words Device ID 1
        # humidity = round(rawhumidity.registers[0] * 0.01,2)
        #SHT20
        rawtemperature = self.client.read_input_registers(3,1,unit=1) #Register no 1 in datasheet size 2 words Decice ID 3
        temperature = round(rawtemperature.registers[0] *0.001,2)
        rawbattery = self.client.read_input_registers(4,1,unit=1) #Register no 3 in datasheet size 2 words Device ID 3
        battery = round(rawbattery.registers[0]*0.001,2)
        return (temperature,battery)

def main():
    datasensor = SensorModbus()
    parse = ParseClass()
    datasensor.serial_ports()
    while True:
        try:
            parse.parse_json()
            time.sleep(0.1) #Mencegah overload processing
        except Exception:
            logging.exception("Exception occurred create main")
            
if __name__ == "__main__":
    main()
