BUILDBR_DIR := ../buildroot

DIR := $(abspath ./)
OUTPUT_DIR := $(abspath ./output)

all:
	@$(MAKE) -C $(BUILDBR_DIR) O=$(OUTPUT_DIR)

initConfig:
	@$(MAKE) -C $(BUILDBR_DIR) O=$(OUTPUT_DIR) defconfig BR2_DEFCONFIG=$(DIR)/defconfig.conf

save:
	@$(MAKE) -C $(BUILDBR_DIR) O=$(OUTPUT_DIR) savedefconfig

%:
	@$(MAKE) -C $(BUILDBR_DIR) O=$(OUTPUT_DIR) $(MAKECMDGOALS)

.PHONY: all
