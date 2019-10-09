#!/bin/bash

konsole -e "./server" &
sleep 2s
konsole -e "./client 127.0.0.1 5001 igorlo" &
konsole -e "./client 127.0.0.1 5001 alex" &
konsole -e "./client 127.0.0.1 5001 vladik" &
