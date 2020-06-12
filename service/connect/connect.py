'''
#########################################################
# File name: Connect.py                                 #
# Author: Yudha Bhakti                                  #
# Date created: 20/06/2019                              #
# Date last modified: 30/10/2019                        #
# Python Version: 3.6                                   #
# Purpose: Open data log for send data log to Backend   #
#########################################################
'''
import pika
import pika.exceptions as exceptions
import sys
import os
import socket
import datetime 
import time
import config
import logging

#Error Log File untuk file collect.py
logging.basicConfig(filename='connect.log', filemode='w', format='%(name)s - %(levelname)s - %(message)s')
logging.basicConfig(format='%(asctime)s - %(message)s', datefmt='%d-%b-%y %H:%M:%S')

class ConnectClass(object):
    def __init__(self): #server pika configuration
        self.dataflag = False
        self.initamqp()
    
    #Inisialisasi Folder
    def inisialisasifolder(self):
        try :
            self.foldersekarang = os.path.dirname(os.path.realpath(__file__)) # Membuka direksi folder tempat file
            self.folderkirimsensor = os.path.abspath(os.path.join(self.foldersekarang, "..",".."))+'/data/connect/' #Folder file log disimpan
            self.filekirimsensor = os.path.join(self.folderkirimsensor,'Data '+datetime.datetime.now().strftime("%Y-%m-%d")+'.txt') #Nama file log 
        except:
            logging.exception("Exception occurred inisialisasi folder")       

    #Membuka koneksi AMQP ke server
    def kirimdatakeserver(self,data):
        try :
            try:
                self.channel.queue_declare(queue=config.QUEUE,durable=False) # Deklarasi Queue
                try:
                    self.channel.basic_publish(exchange='',routing_key=config.ROUTING_KEY,body=data,mandatory=True) # Publish data ke server
                except exceptions.UnroutableError:
                    self.dataflag = False
                else:
                    self.dataflag = True
            except:
                self.initamqp()
                self.channel.queue_declare(queue=config.QUEUE,durable=False) # Deklarasi Queue
                self.channel.basic_publish(exchange='',routing_key=config.ROUTING_KEY,body=data,mandatory=True) # Publish data ke server
                self.dataflag = True
        except:
            logging.exception("Exception occurred server")             

    #Fungsi inisialisasi koneksi AMQP
    def initamqp(self):
        try:
            self.credentials = pika.PlainCredentials(config.USERNAME,config.PASSWORD) # Credential untuk koneksi AMQP
            self.connection = pika.BlockingConnection(pika.ConnectionParameters(config.SERVER,config.PORT,'/',self.credentials)) # Koneksi ke SERVER dan PORT
            self.channel = self.connection.channel() # Membuka channel koneksi
            self.channel.confirm_delivery()
        except:
            logging.exception("Exception occurred reset AMQP")

    #Memastikan log telah tersedia
    def ceklog(self):
        try:
            cekfile = os.path.isfile(self.filekirimsensor)
            if cekfile == True:
                with open(self.filekirimsensor) as datafile:
                    for line in datafile:
                        if '{' in line:
                            return True
                        else:
                            return False
        except Exception:
            logging.exception("Exception occurred ceklog")  

    #Membaca data dari data log, mengirimkan data dan menghapusnya setelah berhasil dikirim
    def sendfromlog(self):
        try:
            opendata = open(self.filekirimsensor,'r') #read data from data logger txt
            lines = opendata.readlines()
            writedata = open(self.filekirimsensor,'w') #write data from data logger txt
            for line in lines:           
                self.kirimdatakeserver(line)
                if self.dataflag == True:
                    print('- kirim data -')
                    print(line)
                    writedata.write('')
                    self.dataflag = False
        except Exception:
            logging.exception("Exception occurred sendfromlog") 

    #Menghapus file log berdasarkan tanggal pembuatannya
    def deletefilelog(self,days):
        try:
            for file in os.listdir(self.folderkirimsensor):
                file = os.path.join(self.folderkirimsensor, file)
                if os.stat(file).st_mtime < time.time() - days * 86400 and os.path.isfile(file):
                    os.remove(os.path.join(self.folderkirimsensor, file))
        except Exception:
            logging.exception("Exception occurred deletefilelog")

#Memastikan device terhubung dengan internet    
def ping():
    remoteserver = "www.google.com"
    try:
        host = socket.gethostbyname(remoteserver)
        connection = socket.create_connection((host,80),2)
        return True # Device connected to internet
    except :
        logging.exception("Exception occurred ping")
        return False     

def main():
    senddata = ConnectClass()
    while True:
        try:
            senddata.inisialisasifolder()
            if ping() == True and senddata.ceklog() == True:
                senddata.sendfromlog()
            while ping() == False:
                if ping() == True:
                    senddata.initamqp()
            senddata.deletefilelog(3)
            time.sleep(0.2)
        except Exception:
            logging.exception("Exception occurred main") 

if __name__ == "__main__":
    main()