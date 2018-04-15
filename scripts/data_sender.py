#!/usr/bin/env python

import subprocess as sbp
import json
import requests as req
from time import sleep

requests = {}

listener_url = "http://192.168.0.207/serv/listener.php"

def json2request(json_):
  json_ = json_.decode("UTF-8").replace('":', '=').replace(',"', '&').replace('"', '').replace('{', '')
  json_ = json_.replace('}', '') 
  return json_

def execute(command): 
  result = ""
  p = sbp.Popen(command, shell=True, stdout=sbp.PIPE)
  lines = p.stdout.readlines()
  if lines.__len__() > 0:
    result = json2request(lines[0])
  else:
    result = "null"
  return result


def prepare_requests():
  #corridor
  dht = execute("./get_dht_json 192.168.0.102")
  relay = f"sensor=relay&{execute('./get_relay_json 192.168.0.102 0')}"
  requests["corridor"] = f"{listener_url}?room=corridor&{dht}&{relay}"
  print(requests["corridor"])

  #room
  dht = execute("./get_dht_json 192.168.0.100")
  relay = f"sensor=relay&{execute('./get_relay_json 192.168.0.100 0')}"
  relay1 =  f"sensor=relay&{execute('./get_relay_json 192.168.0.100 1')}"
  gas = execute("./get_gas_sensor_json 192.168.0.100")
  requests["room"] = f"{listener_url}?room=room&{dht}&{relay}&{relay1}&{gas}"
  print(requests["room"])

  #outside
  dht = execute("./get_dht_json 192.168.0.101")
  requests["outside"] = f"{listener_url}?room=outside&{dht}"
  print(requests["outside"])

if __name__ == "__main__":
  prepare_requests()
  while True:
    print(req.post(requests["corridor"]))
    print(req.post(requests["room"]))
    print(req.post(requests["outside"]))
    sleep(30)
