define check_micro
ifeq ( x$(strip $($(1))),x)
$(info Please define $(1) in $(LOCAL_PATH)/$(LOCAL_MODULE))
endif
endef

LOCAL_MODULE_BUILD:=$(PREFIX)$(LOCAL_MODULE)
ALL_MODULES_CLEAN += $(PREFIX)$(LOCAL_MODULE)

THIRDPART_DEPANNER_MODULES:=$(if $(LOCAL_DEPANNER_MODULES),$(addprefix $(PREFIX),$(LOCAL_DEPANNER_MODULES)),)
PREFIX_LOCAL_DEPANNER_MODULES :=$(if $(LOCAL_DEPANNER_MODULES),$(addprefix $(PREFIX),$(LOCAL_DEPANNER_MODULES)),)
$(call check_micro LOCAL_MODULE_CONFIG_FILES)

__local_sub_modules_build:=$(foreach t,$(ALL_GEN_FILE_TYPE),\
				$(eval __local_module_path:= $(if $(LOCAL_MODULE_PATH),\
												  $(LOCAL_MODULE_PATH),\
											      $(OUT_$(BUILD_MODE)_$(t)_DIR))) \
				$(addprefix $(__local_module_path)/,$(notdir $(LOCAL_MODULE_GEN_$(t)_FILES))))
THIRDPART_MODULE_CONFIG_FILE:=$(LOCAL_PATH)/$(LOCAL_MODULE_CONFIG_FILES)

THIRDPART_LOCAL_STATIC_LIBRARIES := $(if $(LOCAL_STATIC_LIBRARIES),$(addprefix $(OUT_$(BUILD_MODE)_STATIC_DIR)/,$(LOCAL_STATIC_LIBRARIES)),)
THIRDPART_LOCAL_SHARED_LIBRARIES := $(if $(LOCAL_SHARED_LIBRARIES),$(addprefix $(OUT_$(BUILD_MODE)_SHARED_DIR)/,$(LOCAL_SHARED_LIBRARIES)),)

#__local_export_compiler_path := $(if $(EXPORT_COMPILER_PATH),$(EXPORT_COMPILER_PATH);,)
#$(LOCAL_MODULE_BUILD):PRIVATE_CONFIG_COMMAND:=$(__local_export_compiler_path)cd $(LOCAL_PATH);$(LOCAL_MODULE_CONFIG)
#$(LOCAL_MODULE_BUILD):PRIVATE_COMPILER_COMMAND:=$(__local_export_compiler_path)cd $(LOCAL_PATH);$(LOCAL_MODULE_COMPILE)

include $(BUILD_SYSTEM)/module_install.mk
LOCAL_MODULE_COMPILE_CLEAN:=$(if $(LOCAL_FILTER_MODULE),$(LOCAL_MODULE_COMPILE_CLEAN),)
THIRDPART_DEPANNER_MODULES:=$(if $(LOCAL_FILTER_MODULE),$(THIRDPART_DEPANNER_MODULES),)
THIRDPART_LOCAL_SHARED_LIBRARIES:=$(if $(LOCAL_FILTER_MODULE),$(THIRDPART_LOCAL_SHARED_LIBRARIES),)
THIRDPART_LOCAL_STATIC_LIBRARIES:=$(if $(LOCAL_FILTER_MODULE),$(THIRDPART_LOCAL_STATIC_LIBRARIES),)
ALL_MODULES += $(LOCAL_FILTER_MODULE)

include $(BUILD_SYSTEM)/mma_build.mk

__local_module_clean:=

ifneq (x$(strip $(LOCAL_MODULE_COMPILE_CLEAN)),x)
$(LOCAL_MODULE_BUILD)-clean:PRIVATE_MODULE_FILES_CLEAN:=$(__local_sub_modules_build)
CLEAN_DEP_FILES.$(LOCAL_MODULE_BUILD):=$(LOCAL_PATH)/$(LOCAL_MODULE_BUILD) $(__local_static_libraries) $(__local_static_libraries)

__local_module_clean:=$(if $(LOCAL_FILTER_MODULE),$(LOCAL_PATH)/$(LOCAL_MODULE_BUILD)-clean)

$(__local_module_clean):PRIVATE_CLEAN_COMMAND:=$(__local_export_compiler_path)cd $(LOCAL_PATH);$(LOCAL_MODULE_COMPILE_CLEAN)
$(__local_module_clean):PRIVATE_CONFIG_COMMAND:=$(__local_export_compiler_path)cd $(LOCAL_PATH);$(LOCAL_MODULE_CONFIG)
THIRDPART_CLEAN_MODULES += $(__local_module_clean)
endif

ifeq (x$(filter-out $(__local_module_config_file),$(ALL_THIRDPART_CONFIG_FILES)),x)
ALL_THIRDPART_CONFIG_FILES += $(__local_module_config_file)
else
__local_module_config_file:=
endif


include $(BUILD_SYSTEM)/install_inc.mk

define sub_modules_build
$(foreach sub,$(LOCAL_MODULE_GEN_$(1)_FILES), \
			$(eval THIRDPART_MODULE=$(sub)) \
			$(eval include $(BUILD_SYSTEM)/thirdpart_module_build.mk) \
)
endef
$(foreach t,$(ALL_GEN_FILE_TYPE),$(eval $(call sub_modules_build,$(t))))
ALL_INSTALL_MODULES += $(__local_sub_modules_build)
$(LOCAL_MODULE_BUILD):$(__local_sub_modules_build)
	$(transform_install_includes)
ifneq (x$(strip $(THIRDPART_MODULE_CONFIG_FILE)),x)
$(THIRDPART_MODULE_CONFIG_FILE):$(PREFIX_LOCAL_DEPANNER_MODULES)
	$(PRIVATE_CONFIG_COMMAND)
endif

ifneq (x$(strip $(__local_module_clean)),x)
$(__local_module_clean):$(__local_module_config_file)
	$(PRIVATE_CLEAN_COMMAND)
endif
