TMP_SRC_DIR := $(src_path)/taskexe
TMP_SOURCES   :=  task_link.c \
                  task_deal.c \
				  task_container.c \
				  task_dev.c \
				  task_app.c
TASKEXE_OBJS := $(patsubst %.c,$(obj_path)/%.o,$(notdir $(TMP_SOURCES)))
OBJS += $(TASKEXE_OBJS)
include $(mkfile_path)/rule.make
