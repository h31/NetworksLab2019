from errPackages import *

import os
import socket
import struct
import threading
import time


class Server:
    def __init__(self, dPath, portno):
        global serverDir, serverLocalSocket, remoteDict
        serverDir = dPath
        serverLocalSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        serverLocalSocket.bind(('', portno))
        remoteDict = {}

    def run(self):
        while True:

            try:
                data, remoteSocket = serverLocalSocket.recvfrom(4096)

                if remoteSocket in remoteDict:
                    remoteDict[remoteSocket].runProc(data)
                else:
                    remoteDict[remoteSocket] = packetProcess(remoteSocket)
                    remoteDict[remoteSocket].runProc(data)

            except:
                pass


class watchdog(threading.Thread):
    def __init__(self, owner):
        threading.Thread.__init__(self)
        self.setDaemon(True)
        self.resetEvent = threading.Event()
        self.stopEvent = threading.Event()
        self.owner = owner

    def run(self):
        timeCount = 0

        while True:

            if self.stopEvent.isSet():
                break

            if timeCount % 5 == 0 and 0 < timeCount < 25:
                remoteDict[self.owner].reSend()
                print('Resend data.(%s:%s)' % (self.owner[0], self.owner[1]))

            elif timeCount >= 25:
                remoteDict[self.owner].clear('Session timeout. (%s:%s)' \
                                             % (self.owner[0], self.owner[1]))
                break

            if self.resetEvent.isSet():
                timeCount = 0
                self.resetEvent.clear()

            time.sleep(1)
            timeCount += 1

    def countReset(self):
        self.resetEvent.set()

    def stop(self):
        self.stopEvent.set()


class packetProcess:
    def __init__(self, remoteSocket):
        self.remoteSocket = remoteSocket
        self.endFrag = False
        self.watchdog = watchdog(self.remoteSocket)

    def runProc(self, data):
        self.watchdog.countReset()
        Opcode = struct.unpack('!H', data[0:2])[0]

        ##Opcode 1 [ Read request ]
        ##
        ##          2 bytes    string   1 byte     string   1 byte
        ##          -----------------------------------------------
        ##   RRQ   |  01   |  Filename  |   0  |    Mode    |   0  |
        ##          -----------------------------------------------

        if Opcode == 1:

            filename = bytes.decode(data[2:].split(b'\x00')[0])
            filePath = os.path.join(serverDir, filename)
            print('Read request from:%s:%s, filename:%s' \
                  % (self.remoteSocket[0], self.remoteSocket[1], filename))

            if os.path.isfile(filePath):
                try:
                    self.sendFile = open(filePath, 'rb')
                except:
                    serverLocalSocket.sendto(errFileopen, self.remoteSocket)
                    self.clear('Can not read file. Session closed. (%s:%s)' \
                               % (self.remoteSocket[0], self.remoteSocket[1]))
                    return None

                dataChunk = self.sendFile.read(512)
                self.totalDatalen = len(dataChunk)
                self.countBlock = 1

                self.sendPacket = struct.pack(b'!2H', 3, self.countBlock) \
                                  + dataChunk
                serverLocalSocket.sendto(self.sendPacket, self.remoteSocket)

                if len(dataChunk) < 512:
                    self.endFrag = True

                self.watchdog.start()

            else:
                serverLocalSocket.sendto(errNofile, self.remoteSocket)
                self.clear('Requested file not found. Session closed. (%s:%s)' \
                           % (self.remoteSocket[0], self.remoteSocket[1]))


        ##Opcode 2 [ Write request ]
        ##
        ##          2 bytes    string   1 byte     string   1 byte
        ##          -----------------------------------------------
        ##   WRQ   |  02   |  Filename  |   0  |    Mode    |   0  |
        ##          -----------------------------------------------

        elif Opcode == 2:

            filename = bytes.decode(data[2:].split(b'\x00')[0])
            filePath = os.path.join(serverDir, filename)
            print('Write request from:%s:%s, filename:%s' \
                  % (self.remoteSocket[0], self.remoteSocket[1], filename))

            if os.path.isfile(filePath):
                serverLocalSocket.sendto(errFileExists, self.remoteSocket)
                self.clear('File already exist. Session closed. (%s:%s)' \
                           % (self.remoteSocket[0], self.remoteSocket[1]))

            else:
                try:
                    self.rcvFile = open(filePath, 'wb')
                except:
                    serverLocalSocket.sendto(errFileopen, self.remoteSocket)
                    self.clear('Can not open file. Session closed. (%s:%s)' \
                               % (self.remoteSocket[0], self.remoteSocket[1]))
                    return None

                self.totalDatalen = 0
                self.countBlock = 1

                self.sendPacket = struct.pack(b'!2H', 4, 0)
                serverLocalSocket.sendto(self.sendPacket, self.remoteSocket)

                self.watchdog.start()


        ##Opcode 3 [ Data ]
        ##
        ##          2 bytes    2 bytes       n bytes
        ##          ---------------------------------
        ##   DATA  | 03    |   Block #  |    Data    |
        ##          ---------------------------------

        elif Opcode == 3:

            blockNo = struct.unpack('!H', data[2:4])[0]
            dataPayload = data[4:]
            self.totalDatalen += len(dataPayload)

            if blockNo == self.countBlock:
                try:
                    self.rcvFile.write(dataPayload)
                except:
                    serverLocalSocket.sendto(errFilewrite, self.remoteSocket)
                    self.clear('Can not write data. Session closed. (%s:%s)' \
                               % (self.remoteSocket[0], self.remoteSocket[1]))
                    return None

                self.countBlock += 1
                if self.countBlock == 65536:
                    self.countBlock = 0

                self.sendPacket = struct.pack(b'!2H', 4, blockNo)
                serverLocalSocket.sendto(self.sendPacket, self.remoteSocket)

                self.watchdog.countReset()

                if len(dataPayload) < 512:
                    self.clear('Data receive finish. %s bytes (%s:%s)' \
                               % (self.totalDatalen, self.remoteSocket[0],
                                  self.remoteSocket[1]))

            else:
                print('Receive wrong block. Resend data. (%s:%s)'
                      % (self.remoteSocket[0], self.remoteSocket[1]))


        ##Opcode 4 [ ack ]
        ##
        ##          2 bytes    2 bytes
        ##          -------------------
        ##   ACK   | 04    |   Block #  |
        ##          --------------------

        elif Opcode == 4:

            if self.endFrag:
                self.clear('Data send finish. %s bytes (%s:%s)' \
                           % (self.totalDatalen, self.remoteSocket[0],
                              self.remoteSocket[1]))

            else:
                blockNo = struct.unpack('!H', data[2:4])[0]

                if blockNo == self.countBlock:
                    try:
                        dataChunk = self.sendFile.read(512)
                    except:
                        dataChunk = ''

                    dataLen = len(dataChunk)
                    self.totalDatalen += dataLen
                    self.countBlock += 1
                    if self.countBlock == 65536:
                        self.countBlock = 0

                    self.sendPacket = struct.pack(b'!2H', 3, self.countBlock) \
                                      + dataChunk
                    serverLocalSocket.sendto(self.sendPacket, self.remoteSocket)

                    self.watchdog.countReset()

                    if dataLen < 512:
                        self.endFrag = True

                else:
                    print('Receive wrong block. Resend data. (%s:%s)'
                          % (self.remoteSocket[0], self.remoteSocket[1]))


        ##Opcode 5 [ error ]
        ##
        ##          2 bytes  2 bytes        string    1 byte
        ##          ----------------------------------------
        ##   ERROR | 05    |  ErrorCode |   ErrMsg   |   0  |
        ##          ----------------------------------------

        elif Opcode == 5:

            errCode = struct.unpack('!H', data[2:4])[0]
            errString = data[4:-1]
            self.clear('Received error code %s:%s Session closed.(%s:%s)' \
                       % (str(errCode), errString, self.remoteSocket[0],
                          self.remoteSocket[1]))


        ##
        ##Unknown Opcode
        ##

        else:
            serverLocalSocket.sendto(errUnknown, self.remoteSocket)
            self.clear('Unknown error. Session closed.(%s:%s)' \
                       % (self.remoteSocket[0], self.remoteSocket[1]))

    def reSend(self):
        serverLocalSocket.sendto(self.sendPacket, self.remoteSocket)

    def clear(self, message):
        try:
            self.sendFile.close()
        except:
            pass
        try:
            self.rcvFile.close()
        except:
            pass

        del remoteDict[self.remoteSocket]
        self.watchdog.stop()
        print(message.strip())
