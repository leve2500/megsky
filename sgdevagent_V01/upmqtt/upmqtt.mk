TMP_SRC_DIR := $(src_path)/upmqtt
TMP_SOURCES   :=  mqtt_pub.c \
				  mqtt_app.c \
				  mqtt_container.c \
				  mqtt_dev.c \
				  mqtt_json.c 

UPMQTT_OBJS := $(patsubst %.c,$(obj_path)/%.o,$(notdir $(TMP_SOURCES)))
OBJS += $(UPMQTT_OBJS)
include $(mkfile_path)/rule.make
