zephyr_library()
zephyr_include_directories(include)

# zmk-input-matrix is an optional external input processor. Its public source
# uses ZMK's input-processor driver API, whose header is owned by the ZMK app.
# Make that API visible independently of unrelated extra modules, so a clean
# Travelers Pad build has the same include contract as the normal ZMK build.
# zmk-input-matrixは任意の外部input processorであり、ZMK app所有の
# input-processor APIヘッダを使用する。無関係な追加moduleの有無に依存せず、
# clean buildでも通常のZMK buildと同じinclude契約にする。
zephyr_include_directories(${APPLICATION_SOURCE_DIR}/include)

include(cmake/driver.cmake)

# Reusable IQS9150 function modules. Keep their paths grouped by feature so
# they can later be extracted together without changing their public DTS APIs.
# 再利用可能なIQS9150機能モジュール。公開DTS APIを変えずに将来切り出せるよう、
# 機能ごとにパスをまとめる。
include(cmake/functions.cmake)

zephyr_library_sources_ifdef(CONFIG_TRAVELERS_PAD_XIAO_BOOT_PROBE src/platform/xiao/xiao_boot_probe.c)
