TMP_SRC_DIR := $(src_path)/commn
TMP_SOURCES   :=  sgdev_list.c \
                  sgdev_mutex.c \
                  sgdev_param.c \
                  sgdev_queue.c \
                  sgdev_curl.c \
				  sgdev_debug.c \
				  sgdev_common.c		  
COMMN_OBJS := $(patsubst %.c,$(obj_path)/%.o,$(notdir $(TMP_SOURCES)))
OBJS += $(COMMN_OBJS)
include $(mkfile_path)/rule.make
