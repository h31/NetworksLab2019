import argparse
from tftp_test import Client


def get_req_from_cmd():
    while True:
        command = input("\nINPUT FORMAT 'GET/PUT FILE_NAME' or 'EXIT'\n")
        command_list = command.split(" ")
        if command_list[0] == "EXIT" or command_list[0] == "exit":
            exit(0)
        elif command_list.__len__() != 2:
            print("Unexpected arguments number")
            continue
        elif command_list[0] == "GET" or command_list[0] == "get":
            return tftpClient.get, command_list[1]
        elif command_list[0] == "PUT" or command_list[0] == "put":
            return tftpClient.put, command_list[1]
        else:
            print("Unexpected command '%s'" % command_list[0])


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
    tftpClient = Client(serverIp, serverPort, "client_dir")
    while True:
        request, fileName = get_req_from_cmd()

        request(fileName)
