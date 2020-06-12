import pytest
import collect

@pytest.fixture
def serialclass():
    serial_test = collect.SerialClass()
    return serial_test

@pytest.fixture
def parseclass():
    parse_test = collect.ParseClass()
    return parse_test

@pytest.fixture
def kirimclass():
    data = '{"temperatur": "27", "kelembapan": "36", "kecepatan_angin": "0", "tekanan": "1012", "kadar_amonia": "0", "intensitas_cahaya": "23", "id_perangkat": "sector 1", "waktu": "2019-08-19 11:25:39.739876 +0700"}'
    id_perangkat = 'sector_1'
    kirim_test = collect.KirimClass(data,id_perangkat)
    return kirim_test

@pytest.fixture
def datalogclass():
    datalog_test = collect.DataLogClass()
    return datalog_test

def test_serial_ports(serialclass):
    assert 'USB' in str(serialclass.serial_ports())

def test_serial_connection(serialclass):
    assert serialclass.serial_connection() != 0

def test_serial_read(serialclass):
    assert serialclass.serial_read() != ''
    
def test_parse_json(parseclass):
    data = str('{"id_perangkat":"sector 1","kadar_amonia":0,"kelembapan":36,"intensitas_cahaya":23,"kecepatan_angin":0,"temperatur":27,"tekanan":1012}')
    assert "id_perangkat" in parseclass.parse_json(data)
    assert 'temperatur' in parseclass.parse_json(data)
    assert 'kelembapan' in parseclass.parse_json(data)
    assert 'kecepatan_angin' in parseclass.parse_json(data)
    assert 'tekanan' in parseclass.parse_json(data)
    assert 'kadar_amonia' in parseclass.parse_json(data)
    assert 'intensitas_cahaya' in parseclass.parse_json(data)

def test_parse_data(parseclass):
    assert 'waktu' in parseclass.parse_data("sector 1",27,36,0,1012,0,23)

def test_parse_datalog(parseclass):
    data = '{"temperatur": "26", "kelembapan": "45", "kecepatan_angin": "0", "tekanan": "1009", "kadar_amonia": "0", "intensitas_cahaya": "39", "id_perangkat": "sector 2", "waktu": "2019-08-19 12:40:40.997703 +0700"}'
    file = '/mnt/d/WORK/dokumentasi hardware jeager/EMS/hw-s-environment-monitoring-system-development/collect/sector 2 2019-08-19.txt'
    assert parseclass.parse_datalog(data,"sector 2") == file

def test_kirim_intervalkirim(kirimclass):
    assert kirimclass.intervalkirim(30,10) > 0 and kirimclass.intervalkirim(30,10) < 12

def test_kirim_threadkirim(kirimclass):
    assert kirimclass.threadkirim() == True

def test_kirim_datakirim(kirimclass):
    data = '{"temperatur": "27", "kelembapan": "36", "kecepatan_angin": "0", "tekanan": "1012", "kadar_amonia": "0", "intensitas_cahaya": "23", "id_perangkat": "sector 1", "waktu": "2019-08-19 11:25:39.739876 +0700"}'
    file = '/mnt/d/WORK/dokumentasi hardware jeager/EMS/hw-s-environment-monitoring-system-development/connect/Data 2019-08-19.txt'
    assert kirimclass.datakirim(data) == file

def test_datalog_deletelog(datalogclass):
    data = ['sector 2 2019-08-19.txt', 'sector 1 2019-08-19.txt']
    assert datalogclass.deletefilelog(3) == data