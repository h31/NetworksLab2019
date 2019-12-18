import argparse
import os
from tftp_server import Server


def main():
    # ---------------------------- parsing arguments ----------------------------
    parser = argparse.ArgumentParser(description="Some description")

    parser.add_argument("-p", "--port", type=int, action='store',
                        help="port")

    parser.add_argument("-d", "--dir", type=str, action='store',
                        help="direcotry with data")

    parser.add_argument("-v", "--verbose", action='store_true',
                        help='verbosity')

    args = parser.parse_args()

    localDir = args.dir

    localPort = args.port

    verbose = args.verbose

    if localDir is None:
        raise Exception("-d Dir was't passed")
    if localPort is None:
        raise Exception("-p Port was't passed")

    # ---------------------------- running server ----------------------------
    tftpServer = Server(os.path.abspath(localDir), localPort, verbose)
    tftpServer.run()


if __name__ == '__main__':
    main()
