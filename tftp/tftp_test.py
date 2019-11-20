from tftp_client import Client
from tftp_server import Server
import sys


def test():
    '''
    server runnning
        Usage: python -m minimumTFTP -s [directory]
    client get
        Usage: python -m minimumTFTP -g [serverIP] [directory] [filename]
    client put
        Usage: python -m minimumTFTP -p [serverIP] [directory] [filename]
    '''

    if '-s' in sys.argv:
        try:
            Server(sys.argv[2]).run()
        except:
            print(sys.exc_info()[0])
            raise

    elif '-g' in sys.argv:
        try:
            Client(sys.argv[2], sys.argv[3], sys.argv[4]).get()
        except:
            print(sys.exc_info()[0])
            raise

    elif '-p' in sys.argv:
        try:
            Client(sys.argv[2], sys.argv[3], sys.argv[4]).put()
        except:
            print(sys.exc_info()[0])
            raise

    elif 'help' in sys.argv:
        print(test.__doc__)
        sys.exit(0)

    else:
        print(test.__doc__)
        sys.exit(0)


if __name__ == '__main__':
    test()
