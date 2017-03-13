HOST:=
LOCAL_EXT_NAME:=.a
LOCAL_OUT_INSTALL_DIR:=$(OUT_DEVICE_STATIC_DIR)

include $(BUILD_SYSTEM)/base_ruler.mk

$(LOCAL_INSTALL_MODULE):$(LOCAL_OUT_MODULE_BUILD)
	$(call host-mkdir,$(dir $@))
	$(hide) $(call host-cp,$^,$@)

$(LOCAL_OUT_MODULE_BUILD):$(LOCAL_MODULE_OBJS)
	$(hide) $(TARGET_AR) -rsv $@ $^
