// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#ifndef CHAR_MAPIF_H
#define CHAR_MAPIF_H

#include "char.h"

/* mapif interface */
struct mapif_interface {
    void (*ban) (int id, unsigned int flag, int status);
    void (*server_init) (int id);
    void (*server_destroy) (int id);
    void (*server_reset) (int id);
    void (*on_disconnect) (int id);
    void (*on_parse_accinfo) (int account_id, int u_fd, int u_aid, int u_group, int map_fd);
    void (*char_ban) (int char_id, time_t timestamp);
    int (*sendall) (unsigned char *buf, unsigned int len);
    int (*sendallwos) (int sfd, unsigned char *buf, unsigned int len);
    int (*send) (int fd, unsigned char *buf, unsigned int len);
    void (*send_users_count) (int users);
} mapif_s;

struct mapif_interface *mapif;

void mapif_defaults(void);

#endif /* CHAR_MAPIF_H */
