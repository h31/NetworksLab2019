import argparse
from tftp_client import Client


def get_req_from_cmd():
    while True:
        command = input("\nINPUT FORMAT 'GET/PUT FILE_NAME' or 'EXIT'\n")
        command_list = command.split(" ")
        if command_list[0] == "EXIT" or command_list[0] == "exit":
            exit(0)
        elif len(command_list) < 2 or len(command_list) > 3:
            print("Unexpected arguments number")
            continue
        elif command_list[0] == "GET" or command_list[0] == "get":
            if len(command_list) == 2:
                return tftpClient.get, command_list[1], None
            else:
                return tftpClient.get, command_list[1], command_list[2]

        elif command_list[0] == "PUT" or command_list[0] == "put":
            if len(command_list) == 2:
                return tftpClient.put, command_list[1], None
            else:
                return tftpClient.put, command_list[1], command_list[2]

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
        request, fileName, targetFileName = get_req_from_cmd()

        request(fileName, targetFileName)
