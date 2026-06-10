# ============================================================
#  CortexOS Makefile  –  MinGW / GCC  (Windows)
# ============================================================

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99 -O2 -Isrc
LDFLAGS = -lws2_32
TARGET  = CortexOS.exe

SRCS = src/main.c     \
       src/shell.c    \
       src/users.c    \
       src/security.c \
       src/filesystem.c \
       src/process.c  \
       src/scheduler.c \
       src/memory.c   \
       src/network.c  \
       src/database.c \
       src/blockchain.c \
       src/backup.c   \
       src/apps.c     \
       src/logs.c

OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)
	@echo ====================================
	@echo  CortexOS built successfully!
	@echo  Run: $(TARGET)
	@echo ====================================

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	del /Q src\*.o $(TARGET) 2>nul || true

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
