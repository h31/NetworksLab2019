from errPackages import *

import os
import socket
import struct
import threading
import time


class Server:
    """
    Class Main that receive packets and pass they to the PacketProcess
    """

    def __init__(self, dPath, portno, verbose):
        self.serverDir = dPath
        self.serverLocalSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.portno = portno
        self.serverLocalSocket.bind(('', self.portno))
        self.remoteDict = {}
        self.verbose = verbose

    def verbose_print(self, text):
        if self.verbose:
            print("DEBUG:%s" % text)

    def run(self):
        print("server running on port = %s" % self.portno)
        self.verbose_print("TURNED ON")
        while True:

            data, remoteSocket = self.serverLocalSocket.recvfrom(4096)

            if remoteSocket in self.remoteDict:
                self.remoteDict[remoteSocket].runProc(data)
            else:
                self.remoteDict[remoteSocket] = PacketProcess(remoteSocket, self)
                self.remoteDict[remoteSocket].runProc(data)


class Watchdog(threading.Thread):
    """
    Class for time control
    """

    def __init__(self, owner, server):
        threading.Thread.__init__(self)
        self.setDaemon(True)
        self.resetEvent = threading.Event()
        self.stopEvent = threading.Event()
        self.owner = owner
        self.server = server

    def run(self):
        timeCount = 0

        while True:

            if self.stopEvent.isSet():
                break

            if timeCount % 5 == 0 and 0 < timeCount < 25:
                self.server.remoteDict[self.owner].reSend()
                print('Resend data.(%s:%s)' % (self.owner[0], self.owner[1]))

            elif timeCount >= 25:
                self.server.remoteDict[self.owner].clear('Session timeout. (%s:%s)' \
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


class PacketProcess:
    """
    Class for processing packets
    """

    def __init__(self, remoteSocket, server):
        self.remoteSocket = remoteSocket
        self.endFrag = False
        self.server = server
        self.watchdog = Watchdog(self.remoteSocket, self.server)
        self._chunkSize = 512

    def runProc(self, data):
        self.watchdog.countReset()
        Opcode = struct.unpack('!H', data[0:2])[0]
        # --------------------- New read request -------------------------
        ##Opcode 1 [ Read request ]
        ##
        ##          2 bytes    string   1 byte     string   1 byte
        ##          -----------------------------------------------
        ##   RRQ   |  01   |  Filename  |   0  |    Mode    |   0  |
        ##          -----------------------------------------------

        if Opcode == 1:

            filename = bytes.decode(data[2:].split(b'\x00')[0])
            filePath = os.path.join(self.server.serverDir, filename)
            print('Read request from:%s:%s, filename:%s' \
                  % (self.remoteSocket[0], self.remoteSocket[1], filename))

            if os.path.isfile(filePath):
                try:
                    self.sendFile = open(filePath, 'rb')
                except:
                    self.server.serverLocalSocket.sendto(errFileOpen, self.remoteSocket)
                    self.clear('Can not read file. Session closed. (%s:%s)' \
                               % (self.remoteSocket[0], self.remoteSocket[1]))
                    return None

                dataChunk = self.sendFile.read(self._chunkSize)
                self.totalDatalen = len(dataChunk)
                self.countBlock = 1

                self.sendPacket = struct.pack(b'!2H', 3, self.countBlock) \
                                  + dataChunk
                self.server.serverLocalSocket.sendto(self.sendPacket, self.remoteSocket)

                if len(dataChunk) < self._chunkSize:
                    self.endFrag = True

                self.watchdog.start()

            else:
                self.server.serverLocalSocket.sendto(errNoFile, self.remoteSocket)
                self.clear('Requested file not found. Session closed. (%s:%s)' \
                           % (self.remoteSocket[0], self.remoteSocket[1]))

        # --------------------- New write request -------------------------
        ##Opcode 2 [ Write request ]
        ##
        ##          2 bytes    string   1 byte     string   1 byte
        ##          -----------------------------------------------
        ##   WRQ   |  02   |  Filename  |   0  |    Mode    |   0  |
        ##          -----------------------------------------------

        elif Opcode == 2:

            filename = bytes.decode(data[2:].split(b'\x00')[0])
            filePath = os.path.join(self.server.serverDir, filename)
            print('Write request from:%s:%s, filename:%s' \
                  % (self.remoteSocket[0], self.remoteSocket[1], filename))

            if os.path.isfile(filePath):
                self.server.serverLocalSocket.sendto(errFileExists, self.remoteSocket)
                self.clear('File already exist. Session closed. (%s:%s)' \
                           % (self.remoteSocket[0], self.remoteSocket[1]))

            else:
                try:
                    self.rcvFile = open(filePath, 'wb')
                except:
                    self.server.serverLocalSocket.sendto(errFileOpen, self.remoteSocket)
                    self.clear('Can not open file. Session closed. (%s:%s)' \
                               % (self.remoteSocket[0], self.remoteSocket[1]))
                    return None

                self.totalDatalen = 0
                self.countBlock = 1

                self.sendPacket = struct.pack(b'!2H', 4, 0)
                self.server.serverLocalSocket.sendto(self.sendPacket, self.remoteSocket)

                self.watchdog.start()

        # --------------------- Receive new block from client -------------------------
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
                    self.server.serverLocalSocket.sendto(errFileWrite, self.remoteSocket)
                    self.clear('Can not write data. Session closed. (%s:%s)' \
                               % (self.remoteSocket[0], self.remoteSocket[1]))
                    return None

                self.countBlock += 1
                if self.countBlock == 65536:
                    self.countBlock = 0

                self.sendPacket = struct.pack(b'!2H', 4, blockNo)
                self.server.serverLocalSocket.sendto(self.sendPacket, self.remoteSocket)

                self.watchdog.countReset()

                if len(dataPayload) < self._chunkSize:
                    self.clear('Data receive finish. %s bytes (%s:%s)' \
                               % (self.totalDatalen, self.remoteSocket[0],
                                  self.remoteSocket[1]))

            else:
                print('Receive wrong block. Resend data. (%s:%s)'
                      % (self.remoteSocket[0], self.remoteSocket[1]))

        # --------------------- Send new block to client -------------------------
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
                        dataChunk = self.sendFile.read(self._chunkSize)
                    except:
                        dataChunk = ''

                    dataLen = len(dataChunk)
                    self.totalDatalen += dataLen
                    self.countBlock += 1
                    if self.countBlock == 65536:
                        self.countBlock = 0

                    self.sendPacket = struct.pack(b'!2H', 3, self.countBlock) \
                                      + dataChunk
                    self.server.serverLocalSocket.sendto(self.sendPacket, self.remoteSocket)

                    self.watchdog.countReset()

                    if dataLen < self._chunkSize:
                        self.endFrag = True

                else:
                    print('Receive wrong block. Resend data. (%s:%s)'
                          % (self.remoteSocket[0], self.remoteSocket[1]))

        # --------------------- Error processing -------------------------
        ##Opcode 5 [ error ]
        ##
        ##          2 bytes  2 bytes        string    1 byte
        ##          ----------------------------------------
        ##   ERROR | 05    |  ErrorCode |   ErrMsg   |   0  |
        ##          ----------------------------------------

        elif Opcode == 5:
            self.server.verbose_print("ERROR OPCODE")
            errCode = struct.unpack('!H', data[2:4])[0]
            errString = data[4:-1]
            self.clear('Received error code %s:%s Session closed.(%s:%s)' \
                       % (str(errCode), errString, self.remoteSocket[0],
                          self.remoteSocket[1]))


        ##
        ##Unknown Opcode
        ##

        else:
            self.server.serverLocalSocket.sendto(errUnknown, self.remoteSocket)
            self.clear('Unknown error. Session closed.(%s:%s)' \
                       % (self.remoteSocket[0], self.remoteSocket[1]))

    def reSend(self):
        self.server.serverLocalSocket.sendto(self.sendPacket, self.remoteSocket)

    def clear(self, message):
        try:
            self.sendFile.close()
        except:
            pass
        try:
            self.rcvFile.close()
        except:
            pass

        del self.server.remoteDict[self.remoteSocket]
        self.watchdog.stop()
        print(message.strip())
