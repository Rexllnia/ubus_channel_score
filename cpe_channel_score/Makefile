include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk
 
PKG_NAME:=cpe_channel_score
PKG_RELEASE:=1.0
 
PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)
PKG_CONFIG_DEPENDS :=
 
include $(INCLUDE_DIR)/package.mk
 
define Package/$(PKG_NAME)
	SECTION:=utils
	CATEGORY:=Milesight
	TITLE:=cpe_channel_score utility
	DEPENDS:=+libuci +libubus +libubox +libpthread +libjson-c
endef
 
define Package/$(PKG_NAME)/description
	This is Ubus Test OpenWrt.
endef
 
define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef
 
 
define Build/Configure
endef
 
define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS)" \
		CPPFLAGS="$(TARGET_CPPFLAGS)"\ 
		LDFLAGS="$(TARGET_LDFLAGS)"
endef
 
define Package/$(PKG_NAME)/install	
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/cpe_channel_score.init $(1)/etc/init.d/cpe_channel_score
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/cpe_channel_score $(1)/etc/cpe_channel_score
endef
 


$(eval $(call BuildPackage,$(PKG_NAME)))
# feeds/wireless/ruijie/wifi_services
