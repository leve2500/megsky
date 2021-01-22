TMP_SRC_DIR := $(src_path)/uprmtman
TMP_SOURCES   :=  rmt_frame.c \
                  rmt_socket.c

UPRMTMAN_OBJS := $(patsubst %.c,$(obj_path)/%.o,$(notdir $(TMP_SOURCES)))
OBJS += $(UPRMTMAN_OBJS)
include $(mkfile_path)/rule.make
