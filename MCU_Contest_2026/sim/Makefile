# ============================================================================
# Makefile - Build and run the host simulation of the SN32F407 digital clock
# Da Nang MCU Contest 2026
#
# Usage (from MCU_Contest_2026/):
#   make            build clock_sim
#   make run        build and run all simulation checks
#   make clean
#
# The same build is used by the GitHub Actions CI pipeline (gcc on Linux).
# ============================================================================

CC      ?= gcc
CFLAGS  ?= -std=c99 -Wall -Wextra -DMOCK_SIMULATION -I . -I sim
TARGET   = clock_sim

SRCS = main.c clock.c keypad.c display.c buzzer.c eeprom.c \
       sim/SN32F400_mock.c sim/sim_main.c

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $(SRCS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(TARGET).exe

.PHONY: run clean
