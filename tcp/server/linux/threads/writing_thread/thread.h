
#ifndef SERVER_LINUX_WTHREAD_H
#define SERVER_LINUX_WTHREAD_H

#include "../../user_info_list/user_info_list.h"

void *writing_thread(void *arg);

void create_writing_thread(void);

#endif //SERVER_LINUX_THREAD_H
