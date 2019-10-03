# /bin/bash
die(){
	printf "$*\n"
	exit 1
}

# ---------------------- run client ---------------------
client/client_linux \
					-p 5001 \
					-s $(ifconfig | sed -En 's/127.0.0.1//;s/.*inet (addr:)?(([0-9]*\.){3}[0-9]*).*/\2/p' | grep 192)
client_status=$?
client_pid=$!

if [[ $client_status == '0' ]];then
	printf "correct working: client_pid=$client_pid\n"
else
	die "client failed with $client_status"
fi
