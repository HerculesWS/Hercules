// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#ifndef CHAR_MAPIF_H
#define CHAR_MAPIF_H

#include "char.h"

struct s_elemental;

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
    void (*auction_message) (int char_id, unsigned char result);
    void (*auction_sendlist) (int fd, int char_id, short count, short pages, unsigned char *buf);
    void (*parse_auction_requestlist) (int fd);
    void (*auction_register) (int fd, struct auction_data *auction);
    void (*parse_auction_register) (int fd);
    void (*auction_cancel) (int fd, int char_id, unsigned char result);
    void (*parse_auction_cancel) (int fd);
    void (*auction_close) (int fd, int char_id, unsigned char result);
    void (*parse_auction_close) (int fd);
    void (*auction_bid) (int fd, int char_id, int bid, unsigned char result);
    void (*parse_auction_bid) (int fd);
    bool (*elemental_save) (struct s_elemental* ele);
    bool (*elemental_load) (int ele_id, int char_id, struct s_elemental *ele);
    bool (*elemental_delete) (int ele_id);
    void (*elemental_send) (int fd, struct s_elemental *ele, unsigned char flag);
    void (*parse_elemental_create) (int fd, struct s_elemental* ele);
    void (*parse_elemental_load) (int fd, int ele_id, int char_id);
    void (*elemental_deleted) (int fd, unsigned char flag);
    void (*parse_elemental_delete) (int fd, int ele_id);
    void (*elemental_saved) (int fd, unsigned char flag);
    void (*parse_elemental_save) (int fd, struct s_elemental* ele);
} mapif_s;

struct mapif_interface *mapif;

void mapif_defaults(void);

#endif /* CHAR_MAPIF_H */
