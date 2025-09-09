RTMIDI_VERSION = master
RTMIDI_SITE = https://github.com/thestk/rtmidi.git
RTMIDI_SITE_METHOD = git

RTMIDI_LICENSE = MIT
RTMIDI_LICENSE_FILES = LICENSE.txt

RTMIDI_INSTALL_STAGING = YES

define RTMIDI_BUILD_CMDS
    $(MAKE) -C $(@D)
endef

define RTMIDI_INSTALL_TARGET_CMDS
    $(INSTALL) -D -m 0755 $(@D)/librtmidi.so* $(TARGET_DIR)/usr/lib/
    $(INSTALL) -D -m 0644 $(@D)/rtmidi.h $(TARGET_DIR)/usr/include/rtmidi.h
endef

$(eval $(generic-package))
