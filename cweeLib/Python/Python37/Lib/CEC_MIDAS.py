import requests
import json
import base64

def GetToken(username, password):
    credentials = username + ":" + password
    credentials_encodedBytes = base64.b64encode(credentials.encode("utf-8"))   
    headers = {b'Authorization': b'BASIC ' + credentials_encodedBytes}
    url = 'https://midasapi.energy.ca.gov/api/token'
    response = requests.get(url,headers=headers)
    return (response.headers['Token'])

def GetRINList(token, signaltype):
    headers = {'accept': 'application/json', 'Authorization': "Bearer " + token}
    url = 'https://midasapi.energy.ca.gov/api/valuedata?signaltype=' + signaltype
    list_response = requests.get(url, headers=headers)
    return (json.loads(list_response.text))

def GetValue(token, rateID, queryType):
    headers = {'accept': 'application/json', 'Authorization': "Bearer " + token}
    url = 'https://midasapi.energy.ca.gov/api/valuedata?id=' + rateID + '&querytype=' + queryType
    pricing_response = requests.get(url, headers=headers)
    return (json.loads(pricing_response.text))

def LookupTable(token, LookupTable):
    headers = {'accept': 'application/json', 'Authorization': "Bearer " + token}
    url = 'https://midasapi.energy.ca.gov/api/valuedata?' + 'LookupTable=' + LookupTable
    pricing_response = requests.get(url, headers=headers)
    response = requests.get(url,headers=headers) 
    return (json.loads(pricing_response.text))