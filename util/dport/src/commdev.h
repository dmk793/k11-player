#ifndef COMMDEV_H
#define COMMDEV_H

#include <types.h>
#include <termios.h>

struct commdev_t {
    int fd;
    struct termios otios;
};

void commdev_init();
void commdev_open(struct commdev_t *commdev, char *dev);
void commdev_close(struct commdev_t *commdev);
int  commdev_send(struct commdev_t *commdev, uint8 *buf, int len, int timeout);
int  commdev_recv(struct commdev_t *commdev, uint8 *buf, int len, int timeout);
void commdev_exit();

#endif

