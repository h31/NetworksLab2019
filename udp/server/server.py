import copy
import socket
import struct
from random import randint
from time import sleep

ip_range = ["255.255.125.13", "255.255.255.255"]
used_ip = []
UDP_PORT = 67
UDP_PORT_CLIENT = 68
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
sock.bind(('', UDP_PORT))

dict1 = {'magic_cookie': 'None',
         'transaction_id': 'None',
         'requests': {'SubnetMask': False, 'DomainName': False, 'Router': False, 'DNS': False,
                      'StaticRoute': False},
         'boot_req': 'None',
         'hw_type': 'None',
         'hw_addr_len': 'None',
         'hops': 'None',
         'sec_elapsed': 'None',
         'boot_flags': 'None',
         'client_ip': 'None',
         'offered_ip': 'None',
         'next_server_ip': 'None',
         'relay_agent_ip': 'None',
         'padding': 'None',
         'server_host_name': 'None',
         'boot_file_name': 'None',
         }


def range_generation():
    ans = ""
    first = ip_range[0].split(".")
    second = ip_range[1].split(".")
    for i in range(len(first)):
        ans += str(randint(int(first[i]), int(second[i])))
        if i != len(first) - 1:
            ans += "."
    if ans in used_ip:
        range_generation()
    else:
        return ans


class Server:
    def __init__(self):
        self.all_clients = {}  # {'client_mac': [options]}
        self.options = {}
        self.operation = "Unknown"
        self.requests = {'SubnetMask': False, 'DomainName': False, 'Router': False, 'DNS': False,
                         'StaticRoute': False}
        self.end_byte = b'\xff'
        self.mac_address = b'\xd8\xcb\x8a\xee~\xa1' # may be change on random mac
        self.magic_cookie = b''

    def set_data(self, data):
        self.magic_cookie = data[236:240]
        print(f"MAGIC COOKIE IS {self.magic_cookie}")
        self.options_analyzer(data)
        self.options_handler()
        pack = self.data_setter(data)
        print(f"!!!!!!!!! Отправляем ответ на {self.operation} в бродкаст !!!!!!!!!")
        sock.sendto(pack, ("255.255.255.255", UDP_PORT_CLIENT))

    def options_analyzer(self, data):
        pointer = 240
        if data[236:240] == b'\x63\x82\x53\x63':
            while data[pointer] != struct.unpack('!B', self.end_byte)[0]:  # returns tuple
                self.options[f"{data[pointer]}"] = list(
                    data[i] for i in range(pointer + 2, pointer + 2 + data[pointer + 1]))
                pointer = pointer + 1 + data[pointer + 1] + 1
        print(f"WE GET SUCH OPTIONS {self.options}")

    def options_handler(self):
        for key, value in self.options.items():
            # print(f" Key is {key} VALUES is {value}")
            if int(key) == 53:
                if value[0] == 1:
                    self.operation = "DHCPDISCOVER"
                if value[0] == 2:
                    self.operation = "DHCPOFFER"
                if value[0] == 3:
                    self.operation = "DHCPREQUEST"
                if value[0] == 4:
                    self.operation = "DHCPDECLINE"
                if value[0] == 5:
                    self.operation = "DHCPACK"

            if int(key) == 55:
                for elem in value:
                    if elem == 1:
                        self.requests['SubnetMask'] = True
                    if elem == 15:
                        self.requests['DomainName'] = True
                    if elem == 3:
                        self.requests['Router'] = True
                    if elem == 6:
                        self.requests['DNS'] = True
                    if elem == 33:
                        self.requests['StaticRoute'] = True
            if int(key) == 61:
                self.mac_address = b''
                for elem in value:
                    self.mac_address += struct.pack('!B', elem)
                    #   self.mac_address = '.'.join(map(lambda x: str(x), value))  # set usual mac_address

    def data_setter(self, data):
        if self.mac_address not in self.all_clients:
            self.all_clients[self.mac_address] = copy.deepcopy(dict1)
        if self.operation == "DHCPDISCOVER" or self.operation == "DHCPREQUEST":
            self.all_clients[self.mac_address]['magic_cookie'] = data[236:240]
            self.all_clients[self.mac_address]['transaction_id'] = data[4:8]
            self.all_clients[self.mac_address]['requests'] = self.requests
            self.all_clients[self.mac_address]['boot_req'] = data[0]
            self.all_clients[self.mac_address]['hw_type'] = data[1]
            self.all_clients[self.mac_address]['hw_addr_len'] = data[2]
            self.all_clients[self.mac_address]['hops'] = data[3]
            self.all_clients[self.mac_address]['sec_elapsed'] = data[8:10]
            self.all_clients[self.mac_address]['boot_flags'] = data[10:12]
            self.all_clients[self.mac_address]['client_ip'] = data[12:16]
            self.all_clients[self.mac_address]['offered_ip'] = data[16:20]
            self.all_clients[self.mac_address]['next_server_ip'] = data[20:24]
            self.all_clients[self.mac_address]['relay_agent_ip'] = data[24:28]
            self.all_clients[self.mac_address]['padding'] = data[34:44]  # Client hardware address padding
            self.all_clients[self.mac_address]['server_host_name'] = data[44:108]
            self.all_clients[self.mac_address]['boot_file_name'] = data[108:236]
            if self.operation == "DHCPDISCOVER":
                return self.create_dhcp_offer()
            if self.operation == "DHCPREQUEST":
                return self.create_dhcp_pack()

    def create_dhcp_offer(self):
        package = b''
        package += b'\x02'  # Message type: OFFER
        package += b'\x01'  # Hardware type: Ethernet
        package += b'\x06'  # Hardware address length: 6
        package += b'\x00'  # Hops: 0
        package += self.all_clients[self.mac_address]['transaction_id']  # Transaction ID
        print(len(package)) #8
        package += b'\x00\x00'  # Seconds elapsed: 0
        package += b'\x80\x00'  # Bootp flags: 0x8000 (Broadcast) + reserved flags
        package += b'\x00\x00\x00\x00'  # Client IP address: 0.0.0.0 (sending to)
        print(len(package)) #16
        package += socket.inet_pton(socket.AF_INET,
                                    range_generation())  # Your (client) IP address: 0.0.0.0 (offered IP)
        package += socket.inet_pton(socket.AF_INET, "192.168.0.71")  # Next server IP address: 0.0.0.0 (our IP)
        package += socket.inet_pton(socket.AF_INET, "0.0.0.0")  # Relay agent IP address: 0.0.0.0
        print(len(package)) #28
        package += self.mac_address
        print(len(package)) #34
        package += b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'  # Client hardware address padding: 00000000000000000000
        print(len(package)) # 46
        package += b'\x00' * 64  # Server host name not given
        package += b'\x00' * 128  # Boot file name not given
        package += b'c\x82Sc'  # Magic cookie: DHCP
        print(len(package))
        package += b'\x35\x01\x02'  # Option: (t=53,l=1) DHCP Message Type = DHCP Offer
        package += b'\x36\x04' + socket.inet_pton(socket.AF_INET,
                                                  "192.168.0.71")  # Option (t=54 , l=6) Client identifier
        package += b'\x33\x04\x00\x00\x01\xff'  # # Option: (t=51,l=4) LeaseTime    The time is in units of seconds
        package += b'\x01\x04' + socket.inet_pton(socket.AF_INET, "255.255.255.0")  # Option: (t=1,l=4) SubNetMask
        package += b'\x03\x04' + socket.inet_pton(socket.AF_INET, "192.168.0.1")  # Option: (t=3,l=4) Router
        package += b'\x06\x04' + socket.inet_pton(socket.AF_INET, "8.8.8.8")  # Option: (t=6,l=4) DNS
        package += b'\xff'  # End Option
        return package

    def create_dhcp_pack(self):
        # answer on dhcp_request
        package = b''
        package += b'\x02'  # Message type: OFFER
        package += b'\x01'  # Hardware type: Ethernet
        package += b'\x06'  # Hardware address length: 6
        package += b'\x00'  # Hops: 0
        package += self.all_clients[self.mac_address]['transaction_id']  # Transaction ID
        package += b'\x00\x00'  # Seconds elapsed: 0
        package += b'\x80\x00'  # Bootp flags: 0x8000 (Broadcast) + reserved flags
        package += b'\x00\x00\x00\x00'  # Client IP address: 0.0.0.0 (sending to)

        string_ip = '.'.join(map(lambda x: str(x), self.all_clients[self.mac_address]['offered_ip']))
        print(f"!!!!!!!!! Отдаем клиенту {string_ip} !!!!!!!!!")
        package += socket.inet_pton(socket.AF_INET, string_ip)  # Your (client) IP address: 0.0.0.0 (offered IP)
        used_ip.append(string_ip)
        package += socket.inet_pton(socket.AF_INET, "192.168.0.71")  # Next server IP address: 0.0.0.0 (our IP)
        package += socket.inet_pton(socket.AF_INET, "0.0.0.0")  # Relay agent IP address: 0.0.0.0
        package += self.mac_address
        package += b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'  # Client hardware address padding: 00000000000000000000
        package += b'\x00' * 64  # Server host name not given
        package += b'\x00' * 128  # Boot file name not given

        package += b'c\x82Sc'  # Magic cookie: DHCP
        print(len(package))
        package += b'\x35\x01\x05'  # Option: (t=53,l=1) DHCP Message Type = DHCP Pack
        package += b'\x36\x04' + socket.inet_pton(socket.AF_INET,
                                                  "192.168.0.71")  # Option (t=54 , l=6) Client identifier
        package += b'\x33\x04\x00\x00\x01\xff'  # # Option: (t=51,l=4) LeaseTime    The time is in units of seconds
        package += b'\x01\x04' + socket.inet_pton(socket.AF_INET, "255.255.255.0")  # Option: (t=1,l=4) SubNetMask
        package += b'\x03\x04' + socket.inet_pton(socket.AF_INET, "192.168.0.1")  # Option: (t=3,l=4) Router
        package += b'\x06\x04' + socket.inet_pton(socket.AF_INET, "8.8.8.8")  # Option: (t=6,l=4) DNS
        package += b'\xff'  # End Option

        return package


server_one = Server()

while True:
    recv_data, addr = sock.recvfrom(1024)
    print(f"!!!!!!!!! Получил новый UDP пакет !!!!!!!!!")
    # print(recv_data)
    server_one.set_data(recv_data)  #
