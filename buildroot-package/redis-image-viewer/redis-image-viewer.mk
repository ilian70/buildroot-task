###############################################################################
# redis-image-viewer Buildroot package
###############################################################################

REDIS_IMAGE_VIEWER_VERSION = 1.0
REDIS_IMAGE_VIEWER_SITE = $(TOPDIR)/../buildroot-task
REDIS_IMAGE_VIEWER_SITE_METHOD = local
REDIS_IMAGE_VIEWER_LICENSE = unknown
REDIS_IMAGE_VIEWER_DEPENDENCIES = sdl2 sdl2_image hiredis
REDIS_IMAGE_VIEWER_SUPPORTS_IN_SOURCE_BUILD = NO

# Keep the default CMAKE_INSTALL_PREFIX (/usr) set by Buildroot.
REDIS_IMAGE_VIEWER_CONF_OPTS += -DCMAKE_BUILD_TYPE=Release

define REDIS_IMAGE_VIEWER_INSTALL_TARGET_CMDS
	$(MAKE) -C $(@D)/buildroot-build DESTDIR=$(TARGET_DIR) install
	$(INSTALL) -D -m 0644 $(@D)/app.cfg.json $(TARGET_DIR)/etc/redis-image-viewer/config.json
	$(INSTALL) -D -m 0644 $(REDIS_IMAGE_VIEWER_PKGDIR)/redis-image-viewer.service \
		$(TARGET_DIR)/usr/lib/systemd/system/redis-image-viewer.service
	$(INSTALL) -d $(TARGET_DIR)/etc/systemd/system/multi-user.target.wants
	ln -sf ../../usr/lib/systemd/system/redis-image-viewer.service \
		$(TARGET_DIR)/etc/systemd/system/multi-user.target.wants/redis-image-viewer.service
endef

$(eval $(cmake-package))
