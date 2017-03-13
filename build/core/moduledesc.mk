subdir_moduledesc := \
	$(shell $(BUILD_SYSTEM)/tools/findleaves.py --prune=$(OUT_DIR) --prune=.repo --prune=.git $(subdirs) ModuleDescription)

.PHONY: moduledesc
moduledesc:
	$(shell echo $(subdir_moduledesc) >> .bbbb.tmp)
	@echo "*********************************************************"
	@echo "******Please wait while the document is created..********"
	@doxygen -s -g
	@python $(BUILD_SYSTEM)/tools/creat_doc_list.py
	@python $(BUILD_SYSTEM)/tools/modify_moduledesc_doxyfile.py
	@doxygen
	@ln -fs $(TOP_DIR)/docs/ModuleDescription/html/index.html $(TOP_DIR)/ModuleDescription.html
	@echo "*********Successfull !***************"
	@rm .bbbb.tmp .a.tmp .b.tmp .c.tmp Doxyfile
	@echo "********************************************"
