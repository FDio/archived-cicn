define generate_file
	./init.sh ${ABI} ${NDK}
	
endef
all:
	$(call generate_file)
