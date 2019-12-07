
#ifndef ELECTRONIC_STORE_CL_THREAD_H
#define ELECTRONIC_STORE_CL_THREAD_H

#define MAX_USER_COMMAND_SIZE 256

pthread_t create_user_listening_thread(int* initial_socket);

#endif //ELECTRONIC_STORE_THREAD_H
