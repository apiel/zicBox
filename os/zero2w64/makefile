BUILDBR_DIR := ../buildroot

DIR := $(abspath ./)
OUTPUT_DIR := $(abspath ./output)

all:
	@$(MAKE) -C $(BUILDBR_DIR) O=$(OUTPUT_DIR)

initConfig:
	@$(MAKE) -C $(BUILDBR_DIR) O=$(OUTPUT_DIR) defconfig BR2_DEFCONFIG=$(DIR)/defconfig.conf

save:
	@$(MAKE) -C $(BUILDBR_DIR) O=$(OUTPUT_DIR) savedefconfig
#	@$(MAKE) -C $(BUILDBR_DIR) O=$(OUTPUT_DIR) linux-savedefconfig
#	cp $(OUTPUT_DIR)/build/linux-custom/defconfig $(DIR)/linux_defconfig

# This is a generic target that forwards any command to Buildroot
# Example: make menuconfig, make linux-menuconfig, make clean
%:
	@$(MAKE) -C $(BUILDBR_DIR) O=$(OUTPUT_DIR) $(MAKECMDGOALS)

.PHONY: all