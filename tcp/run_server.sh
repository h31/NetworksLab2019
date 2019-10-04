# /bin/bash
die(){
	printf "ERROR: $*\n"

	for server_process in $(ps | grep server_linux | awk '{print $1}');do
		printf "kill $server_process\n"
		kill -9 $server_process
	done

	exit 1
}

# ---------------------- run server ---------------------
PORT=""
while getopts "p:" opt; do
    case "$opt" in
    p)  PORT=$OPTARG
        ;;
    \?) die "illegal option: $OPTARG" >&2
       ;;
    esac
done

if [[ $PORT == "" ]];then die "PORT isn't passed";fi


# nohup server_linux/server_linux 1>/dev/null 2>/dev/null &
server/server_linux -p $PORT
serv_status=$?
serv_pid=$!

if [[ $serv_status == '0' ]];then
	printf "correct working: serv_pid=$serv_pid\n"
else
	die "server failed with $serv_status"
fi
