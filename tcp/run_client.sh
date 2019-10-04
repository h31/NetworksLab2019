#! /bin/bash
die(){
	printf "ERROR: $*\n"
	exit 1
}

# ---------------------- run client ---------------------
USER_NAME=""
IP_SERVER=""
PORT=""

while getopts "n:i:p:" opt; do
    case "$opt" in
    i)  IP_SERVER=$OPTARG
        ;;
    n)  USER_NAME=$OPTARG
        ;;
    p)  PORT=$OPTARG
        ;;
    \?) die "illegal option: $OPTARG" >&2
       ;;
    esac
done

if [[ $USER_NAME == "" ]];then die "USER_NAME isn't passed";fi
if [[ $IP_SERVER == "" ]];then die "IP_SERVER isn't passed";fi
if [[ $PORT == "" ]];then die "PORT isn't passed";fi


client/client_linux \
					-p $PORT \
					-s $IP_SERVER\
					-n $USER_NAME
client_status=$?
client_pid=$!

if [[ $client_status == '0' ]];then
	printf "correct working: client_pid=$client_pid\n"
else
	die "client failed with $client_status"
fi
