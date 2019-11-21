import argparse
from tftp_test import Client

if __name__ == '__main__':

    # ---------------------------- parsing arguments ----------------------------
    parser = argparse.ArgumentParser(description="Some description")

    parser.add_argument("-p", "--port", type=int, action='store',
                        help="port")

    parser.add_argument("-i", "--ip", type=str, action='store',
                        help="ip")

    args = parser.parse_args()

    serverPort = args.port

    serverIp = args.ip

    if serverPort is None:
        raise Exception("-p Port was't passed")
    if serverIp is None:
        raise Exception("-i IP was't passed")

    # ---------------------------- running client ----------------------------
    tftpClient = Client(serverIp, serverPort, "client_dir", "Mikrokontrollery.pdf")
    ## get
    tftpClient.get()
    ## put
    # tftpClient.put()
