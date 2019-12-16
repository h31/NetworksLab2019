import socket
import struct
from random import randint
from uuid import getnode as get_mac
from time import sleep

UDP_PORT_OUT = 67
UDP_PORT_IN = 68
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
sock.bind(('', UDP_PORT_IN))


def get_mac_address():
    mac = ':'.join(("%012X" % get_mac())[i:i + 2] for i in range(0, 12, 2))  # create string with : separated mac values
    byte_mac = b''
    for element in mac.split(":"):
        byte_mac += struct.pack('!B', int(element,
                                         16))  # create byte package with mac address  bytes.fromhex(hex(get_mac())[2:])
    print(byte_mac)
    return byte_mac


def generate_transaction_id():
    id = b''
    for i in range(4):
        id += struct.pack('!B', randint(0, 255))
    return id


class Client:
    def __init__(self):
        self.operation = "Unknown"
        self.requests = {'SubnetMask': False, 'DomainName': False, 'Router': False, 'DNS': False, 'StaticRoute': False}
        self.options = {}
        self.data = 0
        self.end_byte = b'\xff'
        self.transaction_id = generate_transaction_id()
        self.magic_cookie = b'\x63\x82\x53\x63'
        self.subnetMask = b''
        self.offerIP = 0
        self.nextServerIP = 0
        self.router = 0
        self.DNS = 0
        self.leaseTime = 0
        self.DHCPServerIdentifier = 0

    def set_data(self, data):

        self.data = data
        self.options_analyzer()
        self.options_handler()
        if self.operation == "DHCPOFFER":
            self.unpack()
            pack = self.build_request_pack()
            print(f"!!!!!!!!! Отправляем REQUEST в овет на на {self.operation} в бродкаст !!!!!!!!!")
            sock.sendto(pack, ("255.255.255.255", UDP_PORT_OUT))
        elif self.operation == "DHCPACK":
            print(f"!!!!!!!!! Получил DHCPACK, теперь {self.offerIP} мой на {self.leaseTime} секунд?! !!!!!!!!!")

    def build_discover_pack(self):
        mac_address = get_mac_address()
        package = b''
        package += b'\x01'  # Message type: Boot Request (1)
        package += b'\x01'  # Hardware type: Ethernet
        package += b'\x06'  # Hardware address length: 6
        package += b'\x00'  # Hops: 0
        package += self.transaction_id  # Transaction ID
        package += b'\x00\x00'  # Seconds elapsed: 0
        package += b'\x80\x00'  # Bootp flags: 0x8000 (Broadcast) + reserved flags
        package += b'\x00\x00\x00\x00'  # Client IP address: 0.0.0.0
        package += b'\x00\x00\x00\x00'  # Your (client) IP address: 0.0.0.0
        package += b'\x00\x00\x00\x00'  # Next server IP address: 0.0.0.0
        package += b'\x00\x00\x00\x00'  # Relay agent IP address: 0.0.0.0
        package += mac_address  # may be 0 here
        package += b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'  # Client hardware address padding: 00000000000000000000
        package += b'\x00' * 64  # Server host name not given
        package += b'\x00' * 128  # Boot file name not given

        package += b'\x63\x82\x53\x63'  # Magic cookie: DHCP
        package += b'\x35\x01\x01'  # Option: (t=53,l=1) DHCP Message Type = DHCP Discover
        package += b'\x3d\x06' + mac_address  # Option (t=61 , l=6) Client identifier
        package += b'\x37\x03\x03\x01\x06'  # Option: (t=55,l=3) Parameter Request List
        package += b'\xff'  # End Option
        return package

    def build_request_pack(self):
        mac_address = get_mac_address()
        package = b''
        package += b'\x01'  # Message type: Boot Request (1)
        package += b'\x01'  # Hardware type: Ethernet
        package += b'\x06'  # Hardware address length: 6
        package += b'\x00'  # Hops: 0
        package += self.transaction_id  # Transaction ID
        package += b'\x00\x00'  # Seconds elapsed: 0
        package += b'\x80\x00'  # Bootp flags: 0x8000 (Broadcast) + reserved flags
        package += b'\x00\x00\x00\x00'  # Client IP address: 0.0.0.0
        package += socket.inet_pton(socket.AF_INET, self.offerIP)  # Your (client) IP address: 0.0.0.0
        # package += b'\x00\x00\x00\x00'  # Your (client) IP address: 0.0.0.0
        package += b'\x00\x00\x00\x00'  # Next server IP address: 0.0.0.0
        package += b'\x00\x00\x00\x00'  # Relay agent IP address: 0.0.0.0
        package += mac_address  # may be 0 here
        package += b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'  # Client hardware address padding: 00000000000000000000
        package += b'\x00' * 64  # Server host name not given
        package += b'\x00' * 128  # Boot file name not given

        package += b'\x63\x82\x53\x63'  # Magic cookie: DHCP
        package += b'\x35\x01\x03'  # Option: (t=53,l=1) DHCP Message Type = DHCP Request
        package += b'\x3d\x06' + mac_address  # Option (t=61 , l=6) Client identifier
        package += b'\x32\x04' + socket.inet_pton(socket.AF_INET,
                                                  self.offerIP)  # Option: (t=50,l=1) Requested IP Address
        package += b'\x36\x04' + socket.inet_pton(socket.AF_INET,
                                                  self.DHCPServerIdentifier)  # Option: (t=54,l=1) DHCP server identifer
        package += b'\x0c\x0f' + bytes(socket.gethostname(), 'utf-8')  # Option: (t=12,l=15) Host Name
        package += b'\x37\x03\x03\x01\x06'  # Option: (t=55,l=3) Parameter Request List
        package += b'\xff'  # End Option
        return package

    def options_analyzer(self):
        pointer = 240
        if self.magic_cookie == b'\x63\x82\x53\x63':
            while self.data[pointer] != struct.unpack('!B', self.end_byte)[0]:  # returns tuple
                self.options[f"{self.data[pointer]}"] = list(
                    self.data[i] for i in range(pointer + 2, pointer + 2 + self.data[pointer + 1]))
                pointer = pointer + 1 + self.data[pointer + 1] + 1
        print(f"WE GET SUCH OPTIONS {self.options}")

    def unpack(self):
        if self.data[4:8] == self.transaction_id:
            self.offerIP = '.'.join(map(lambda x: str(x), self.data[16:20]))
            self.nextServerIP = '.'.join(map(lambda x: str(x), self.data[20:24]))

    def options_handler(self):
        for key, value in self.options.items():
            # print(f" Key is {key} VALUES is {value}")
            if int(key) == 1:
                self.subnetMask = '.'.join(map(lambda x: str(x), value))
            if int(key) == 3:
                self.router = '.'.join(map(lambda x: str(x), value))
            if int(key) == 6:
                self.DNS = '.'.join(map(lambda x: str(x), value))
            if int(key) == 51:
                self.leaseTime = 0
                for i, elem in enumerate(value):
                    self.leaseTime += 255 ** (3 - i) * elem
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
            if int(key) == 54:
                self.DHCPServerIdentifier = '.'.join(
                    map(lambda x: str(x), value))  # need to take it from options


if __name__ == '__main__':
    first_client = Client()
    sock.sendto(first_client.build_discover_pack(), ('255.255.255.255', UDP_PORT_OUT))
    print(f"!!!!!!!!! Посылаю DHCPDISCOVER в бродакст !!!!!!!!!")
    while True:
        recv_data, addr = sock.recvfrom(1024)
        print(f"!!!!!!!!! Получили пакет !!!!!!!!!")
        print(recv_data[236:240])
        first_client.set_data(recv_data)
