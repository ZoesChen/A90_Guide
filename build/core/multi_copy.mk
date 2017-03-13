###########################################################
## Standard rules for copying files that are multi_copy
##
## None.
##
###########################################################

copy_module_tags := $(LOCAL_MODULE_TAGS)
copy_module_dir := $(LOCAL_MODULE_DIR)
copy_module_files:= $(LOCAL_COPY_FILES)
copy_module_name:= $(LOCAL_MODULE)
copy_depanner_module:=$(LOCAL_DEPANNER_MODULES)
copy_out_path := $(LOCAL_MODULE_PATH)
COPY_PREFIX_LOCAL_DEPANNER_MODULES :=$(if $(LOCAL_DEPANNER_MODULES),$(addprefix $(PREFIX),$(LOCAL_DEPANNER_MODULES)),)
LOCAL_MODULE_BUILD := $(LOCAL_MODULE)
CLEAN_DEP_FILES.$(LOCAL_MODULE_BUILD) :=
CLEAN_DEP_MODULES.$(LOCAL_MODULE_BUILD) :=
CLEAN_DEP_MODULES.$(LOCAL_MODULE_BUILD) := $(COPY_PREFIX_LOCAL_DEPANNER_MODULES)

ALL_MODULES_CLEAN += $(LOCAL_MODULE)

define auto-copy-boilerplate
$(if $(filter %: :%,$(1)), \
    $(error $(LOCAL_PATH): Leading or trailing colons in "$(1)")) \
$(foreach t,$(1), \
	$(eval include $(CLEAR_VARS)) \
	$(eval LOCAL_IS_HOST_MODULE := $(2)) \
	$(eval LOCAL_MODULE_CLASS := $(3)) \
	$(eval LOCAL_MODULE_TAGS := $(4)) \
	$(eval LOCAL_MODULE_PATH := $(5)) \
	$(eval LOCAL_MODULE:= $(6)) \
	$(eval LOCAL_DEPANNER_MODULES:= $(7)) \
	$(eval LOCAL_COPY_FILES := $(t)) \
	$(eval include $(BUILD_SYSTEM)/copy_base.mk) \
)
endef

ifeq ($(strip x$(LOCAL_MODULE_CLASS)), xDIR)
$(call auto-copy-boilerplate,\
	$(copy_module_dir),\
	$(copy_is_device),\
	DIR,\
	$(copy_module_tags), \
	$(copy_out_path), \
	$(copy_module_name), \
	$(copy_depanner_module))
else
$(call auto-copy-boilerplate,\
	$(copy_module_files),\
	$(copy_is_host),\
	FILES,\
	$(copy_module_tags), \
	$(copy_out_path), \
	$(copy_module_name) , \
	$(copy_depanner_module))
endif

$(LOCAL_MODULE_BUILD)-clean:PRIVATE_MODULE_FILES_CLEAN:= $(CLEAN_DEP_FILES.$(LOCAL_MODULE_BUILD))
