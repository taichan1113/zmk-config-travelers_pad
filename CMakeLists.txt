zephyr_library()
# Driver/functions are west projects; this module owns keyboard platform code only.
# driver・機能群はwest project。ここではキーボード固有platformコードだけを扱う。
zephyr_library_sources_ifdef(CONFIG_TRAVELERS_PAD_XIAO_BOOT_PROBE src/platform/xiao/xiao_boot_probe.c)
