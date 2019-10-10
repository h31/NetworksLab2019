# /bin/bash
die(){
	printf "ERROR: $*\n"

	for server_process in $(ps | grep server_linux | awk '{print $1}');do
		printf "kill $server_process\n"
		kill -9 $server_process
	done

	exit 1
}

set -x
# ---------------------- run server ---------------------
PORT=""
DOCKER_IMAGE=""
CONT_NAME="server_container"
while getopts "p:i:" opt; do
    case "$opt" in
    p)  PORT=$OPTARG
        ;;
    i)  DOCKER_IMAGE=$OPTARG
        ;;
    \?) die "illegal option: $OPTARG" >&2
       ;;
    esac
done

if [[ $PORT == "" ]];then die "PORT isn't passed";fi
if [[ $DOCKER_IMAGE == "" ]];then die "DOCKER_IMAGE isn't passed";fi

# build project
make -j5 1>make.stdout 2>make.stderr


# run server in docker container
docker rm --force $CONT_NAME
docker run --name $CONT_NAME -it -d -p $PORT:$PORT $DOCKER_IMAGE /bin/bash
docker cp server/server_linux $CONT_NAME:/tmp
docker exec -it $CONT_NAME /tmp/server_linux -p $PORT
