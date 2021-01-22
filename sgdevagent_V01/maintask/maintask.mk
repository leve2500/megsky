TMP_SRC_DIR := $(src_path)/maintask
TMP_SOURCES   :=  main.c \
                  thread_interact.c \
                  thread_manage.c \
                  thread_dev_insert.c \
                  thread_task_exe.c \
                  timer_pack.c

MAINTASK_OBJS := $(patsubst %.c,$(obj_path)/%.o,$(notdir $(TMP_SOURCES)))
OBJS += $(MAINTASK_OBJS)
include $(mkfile_path)/rule.make
