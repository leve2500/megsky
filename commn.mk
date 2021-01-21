TMP_SRC_DIR := $(src_path)/commn
TMP_SOURCES   :=  json_frame.c \
                  param_cfg.c \
                  queue_model.c \
                  tzc_mutex.c

COMMN_OBJS := $(patsubst %.c,$(obj_path)/%.o,$(notdir $(TMP_SOURCES)))
OBJS += $(COMMN_OBJS)
include $(mkfile_path)/rule.make
