# HW-S-Environment Monitoring System

This repository contains firmware development in Environment Monitoring System. This project is collecting data from XY-MD02 via modbus and then send data to the backend via AMQP.

## Dependencies

```
Linux-based system with docker and docker-compose
```

## Usage

To get started clone this repo or download zip and extract on your local machine. To clone using git:

```
$ git clone https://gitlab.com/environment-monitoring-system/hw-s-environment-monitoring-system.git
```
### Setup Configuration
Open config.py file, change configuration same as your AMQP connection. 

```
USERNAME = 'guest'
PASSWORD = 'guest'
ID='testing' 
SERVER='172.16.5.145' 
PORT='5672'
QUEUE='environment_sensor'
ROUTING_KEY='environment_sensor'
```
### Running file

Run docker-compose  
```
$ docker-compose up --build -d
```
Done !!

## Authors

**Yudha Bhakti** - *Initial work* 



