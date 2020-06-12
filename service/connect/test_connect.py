import pika
import pika.exceptions as exceptions
import pytest
import connect
import config


@pytest.fixture
def connectclass():
    connect_test = connect.ConnectClass()
    return connect_test

def test_connect_server(connectclass):
    data = str('{"id_perangkat":"sector 1","kadar_amonia":0,"kelembapan":36,"intensitas_cahaya":23,"kecepatan_angin":0,"temperatur":27,"tekanan":1012}')
    assert connectclass.server(data) == True

def test_connect_publish(connectclass):
    credentials = pika.PlainCredentials(config.USERNAME,config.PASSWORD)
    connection = pika.BlockingConnection(pika.ConnectionParameters(config.SERVER,config.PORT,'/',credentials))
    channel = connection.channel()
    data = str('{"id_perangkat":"sector 1","kadar_amonia":0,"kelembapan":36,"intensitas_cahaya":23,"kecepatan_angin":0,"temperatur":27,"tekanan":1012}')
    assert connectclass.publish(data,channel) == True

def test_connect_ping(connectclass):
    assert connectclass.ping() == True

def test_connect_ceklog(connectclass):
    assert connectclass.ceklog() == True

def test_connect_sendfromlog(connectclass):
    assert "id_perangkat" in connectclass.sendfromlog()

def test_datalog_deletelog(connectclass):
    data = ['Data 2020-04-23.txt']
    assert connectclass.deletefilelog() == data

