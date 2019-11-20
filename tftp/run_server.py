import os
from tftp_test import Server

## server running
tftpServer = Server(os.path.abspath('data_dir'), 5000)
tftpServer.run()
