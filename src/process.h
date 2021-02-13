#ifndef PUSB_PROCESS_H_
# define PUSB_PROCESS_H_

#include <unistd.h>

void get_process_name(const pid_t pid, char * name);

void get_process_parent_id(const pid_t pid, pid_t * ppid);

#endif /* !PUSB_PROCESS_H_ */