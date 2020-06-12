
import threading, time, os, socket, logging

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

#Reboot device 
def reboot():
    try:
        print (ping())
        if ping() == False:
            os.system('reboot')
    except :
        logging.exception("Exception occurred reboot")

def main():
    while True:
        try:
            ticker = threading.Event()
            while not ticker.wait(1800):#1800 Seconds atau 30 Menit
                reboot()
        except Exception:
            logging.exception("Exception occurred main") 

if __name__ == "__main__":
    main()