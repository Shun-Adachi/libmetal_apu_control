TARGET = libmetal_apu_control
SRC = libmetal_apu_control.c \
      system/linux/xlnx/zynqmp_amp_demo/sys_init.c
OBJ = $(SRC:.c=.o)

# WSL環境（libmetalはインストール済み）
INC = -I. -Isystem/linux/xlnx/zynqmp_amp_demo -I../libmetal/install/include
LIBS = ../libmetal/build/lib/libmetal.a

# APU側（インストール済libmetal使用 or 動的リンクも可）
# APU上では以下に差し替えも可能：
# INC = -I/usr/include
# LIBS = -lmetal

CFLAGS = -Wall -O2
LDFLAGS = -lpthread -lrt -lsysfs

CC = gcc

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(LIBS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

clean:
	rm -f $(TARGET) *.o