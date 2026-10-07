zephyr_library()
zephyr_include_directories(${APPLICATION_SOURCE_DIR}/include)
zephyr_library_sources(src/platform/xiao/management_gate.c)
# Driver/functions are west projects; this module owns keyboard platform code only.
# driver・機能群はwest project。ここではキーボード固有platformコードだけを扱う。
zephyr_library_sources_ifdef(CONFIG_TRAVELERS_PAD_XIAO_BOOT_PROBE src/platform/xiao/xiao_boot_probe.c)
zephyr_library_sources_ifdef(CONFIG_TRAVELERS_PAD_BLE_IDLE_HISTORY src/platform/xiao/ble_idle_history.c)
