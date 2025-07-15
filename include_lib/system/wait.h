#ifndef WAIT_COMPLETION_H
#define WAIT_COMPLETION_H





int wait_completion_schedule();

u16 wait_completion(int (*condition)(void), int (*callback)(void *), void *priv);

int wait_completion_del(u16 id);



#endif
