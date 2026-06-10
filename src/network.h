#ifndef NETWORK_H
#define NETWORK_H

#include "common.h"

/* ── Virtual Internet ── */
void cmd_browser(int argc, char **argv);

/* ── Email ── */
void cmd_mail(int argc, char **argv);
void cmd_send_mail(int argc, char **argv);
void cmd_inbox(int argc, char **argv);

/* ── LAN chat ── */
void cmd_connect(int argc, char **argv);
void cmd_chat(int argc, char **argv);
void cmd_disconnect(int argc, char **argv);

#endif
