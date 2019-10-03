# /bin/bash
die(){
	printf "$*\n"

	for server_process in $(ps | grep server_linux | awk '{print $1}');do
		printf "kill $server_process\n"
		kill -9 $server_process
	done

	exit 1
}


# ---------------------- run server ---------------------
# nohup server_linux/server_linux 1>/dev/null 2>/dev/null &
server/server_linux -p 5001
serv_status=$?
serv_pid=$!

if [[ $serv_status == '0' ]];then
	printf "correct working: serv_pid=$serv_pid\n"
else
	die "server failed with $serv_status"
fi
