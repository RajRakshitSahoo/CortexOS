#ifndef APPS_H
#define APPS_H

#include "common.h"

/* ── Package manager ── */
void cmd_install(int argc, char **argv);
void cmd_remove(int argc, char **argv);
void cmd_update(int argc, char **argv);
void cmd_packages(int argc, char **argv);
void cmd_store(int argc, char **argv);

/* ── Built-in apps ── */
void app_calculator(void);
void app_calendar(void);
void app_notes(void);
void app_snake(void);
void app_todo(void);
void app_chess(void);
void app_sudoku(void);
void app_minesweeper(void);
void app_text_editor(const char *filename);

void cmd_edit(int argc, char **argv);
void cmd_calc(int argc, char **argv);
void cmd_calendar_cmd(int argc, char **argv);
void cmd_notes(int argc, char **argv);
void cmd_snake(int argc, char **argv);
void cmd_todo(int argc, char **argv);
void cmd_chess(int argc, char **argv);
void cmd_sudoku(int argc, char **argv);
void cmd_minesweeper(int argc, char **argv);

/* ── VM simulator ── */
void cmd_vm(int argc, char **argv);

/* ── History ── */
void history_push(const char *cmd);
void cmd_history(int argc, char **argv);

#endif
