#! /bin/bash
die(){
	printf "$*\n"
	exit 1
}

# ---------------------- run client ---------------------
if [[ $# < 2 ]];then
  die "user_name isn't passed, exiting..."
fi

USER_NAME=$1
IP_SERVER=$2

client/client_linux \
					-p 5001 \
					-s $IP_SERVER\
					-n $USER_NAME
client_status=$?
client_pid=$!

if [[ $client_status == '0' ]];then
	printf "correct working: client_pid=$client_pid\n"
else
	die "client failed with $client_status"
fi
