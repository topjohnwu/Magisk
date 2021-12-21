.NOTPARALLEL:
default:
	python $(LIT) -sv --param android_mode=$(LIT_MODE) $(LIT_ARGS) \
        $(ANDROID_BUILD_TOP)/external/libcxx/test
