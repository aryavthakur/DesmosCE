NAME = DESMOSCE
TARGET = DESMOSCE.8xp
DESCRIPTION = "Independent native graphing calculator"
COMPRESSED = YES
ARCHIVED = YES
CFLAGS = -Wall -Wextra -Wno-misleading-indentation -Oz -fno-inline-functions -std=c11 -Iinclude
LTOFLAGS = -Oz -fno-inline-functions
LTO = YES
HAS_PRINTF = NO
PREFER_OS_CRT = YES
include $(shell cedev-config --makefile)
