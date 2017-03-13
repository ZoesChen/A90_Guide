###########################################################
## Standard rules for copying files that are multi_prebuilt
##
## None.
##
###########################################################

prebuilt_static_libs := $(filter %.a,$(LOCAL_PREBUILT_LIBS))
prebuilt_shared_libs := $(filter-out %.a,$(LOCAL_PREBUILT_LIBS))
prebuilt_executables := $(LOCAL_PREBUILT_EXECUTABLES)
prebuilt_module_tags := $(LOCAL_MODULE_TAGS)
prebuilt_module_dir := $(LOCAL_MODULE_DIR)
prebuilt_module_files:= $(LOCAL_COPY_FILES)
prebuilt_module_name:= $(LOCAL_MODULE)
prebuilt_out_path := $(LOCAL_MODULE_PATH)
LOCAL_MODULE_BUILD := $(LOCAL_MODULE)
CLEAN_DEP_FILES.$(LOCAL_MODULE_BUILD) :=
CLEAN_DEP_MODULES.$(LOCAL_MODULE_BUILD) :=

ALL_MODULES_CLEAN += $(LOCAL_MODULE)

ifneq ($(strip x$(LOCAL_DEPANNER_MODULES)),x)
$(warning "Prebuilt can't have depanner modules.")
endif

define auto-prebuilt-boilerplate
$(if $(filter %: :%,$(1)), \
    $(error $(LOCAL_PATH): Leading or trailing colons in "$(1)")) \
$(foreach t,$(1), \
	$(eval include $(CLEAR_VARS)) \
	$(eval LOCAL_IS_HOST_MODULE := $(2)) \
	$(eval LOCAL_MODULE_CLASS := $(3)) \
	$(eval LOCAL_MODULE_TAGS := $(4)) \
	$(eval LOCAL_MODULE_PATH := $(5)) \
	$(eval LOCAL_MODULE:= $(6)) \
	$(eval LOCAL_COPY_FILES := $(t)) \
	$(eval include $(BUILD_SYSTEM)/prebuilt_base.mk) \
)
endef


$(call auto-prebuilt-boilerplate,\
	$(prebuilt_shared_libs),\
	$(prebuilt_is_host),\
	SHARED_LIBRARIES,\
	$(prebuilt_module_tags), \
	$(prebuilt_out_path), \
	$(prebuilt_module_name))

$(call auto-prebuilt-boilerplate,\
	$(prebuilt_static_libs),\
	$(prebuilt_is_host),\
	STATIC_LIBRARIES,\
	$(prebuilt_module_tags), \
	$(prebuilt_out_path), \
	$(prebuilt_module_name))


ifeq ($(strip x$(LOCAL_MODULE_CLASS)), xDIR)
$(call auto-prebuilt-boilerplate,\
	$(prebuilt_module_dir),\
	$(prebuilt_is_device),\
	DIR,\
	$(prebuilt_module_tags), \
	$(prebuilt_out_path), \
	$(prebuilt_module_name))
else
$(call auto-prebuilt-boilerplate,\
	$(prebuilt_module_files),\
	$(prebuilt_is_host),\
	FILES,\
	$(prebuilt_module_tags), \
	$(prebuilt_out_path), \
	$(prebuilt_module_name))
endif

$(LOCAL_MODULE_BUILD)-clean:PRIVATE_MODULE_FILES_CLEAN:= $(CLEAN_DEP_FILES.$(LOCAL_MODULE_BUILD))
