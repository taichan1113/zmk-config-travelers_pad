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

zephyr_library_sources_ifdef(CONFIG_IQS9150_EV02_DRIVER src/driver/iqs9150/iqs9150_ev02_driver.c)

# Reusable IQS9150 function modules. Keep their paths grouped by feature so
# they can later be extracted together without changing their public DTS APIs.
# 再利用可能なIQS9150機能モジュール。公開DTS APIを変えずに将来切り出せるよう、
# 機能ごとにパスをまとめる。
zephyr_library_sources_ifdef(CONFIG_TRAVELERS_PAD_IQS9150_POINTER src/function/iqs9150/core/iqs9150_pointer.c)
zephyr_library_sources_ifdef(CONFIG_TRAVELERS_PAD_IQS9150_MULTIFINGER_ROUTER src/function/iqs9150/core/iqs9150_multifinger_router.c)
zephyr_library_sources_ifdef(CONFIG_TRAVELERS_PAD_IQS9150_TAP_DRAG src/function/iqs9150/tap_drag/iqs9150_tap_drag.c)
zephyr_library_sources_ifdef(CONFIG_TRAVELERS_PAD_IQS9150_GRID_KSCAN src/function/iqs9150/grid_adapter/iqs9150_grid_kscan.c)
zephyr_library_sources_ifdef(CONFIG_TRAVELERS_PAD_IQS9150_MULTIFINGER_GESTURE_KSCAN src/function/iqs9150/multifinger_gesture/iqs9150_multifinger_gesture_kscan.c)
zephyr_library_sources_ifdef(CONFIG_TRAVELERS_PAD_IQS9150_MATRIX_INPUT src/function/iqs9150/grid_adapter/iqs9150_matrix_input.c)
zephyr_library_sources_ifdef(CONFIG_TRAVELERS_PAD_IQS9150_EDGE_DIAL_KSCAN src/function/iqs9150/edge_dial/iqs9150_edge_dial_kscan.c)
zephyr_library_sources_ifdef(CONFIG_TRAVELERS_PAD_IQS9150_EDGE_HOLD_CONTINUE src/function/iqs9150/edge_hold_continue/iqs9150_edge_hold_continue.c)
zephyr_library_sources_ifdef(CONFIG_TRAVELERS_PAD_IQS9150_SCROLL_INERTIA src/function/iqs9150/scroll_inertia/iqs9150_scroll_inertia.c)
zephyr_library_sources_ifdef(CONFIG_TRAVELERS_PAD_IQS9150_CONTACT_SESSION src/function/iqs9150/session/contact_session_manager.c)
zephyr_library_sources_ifdef(CONFIG_TRAVELERS_PAD_IQS9150_CONTACT_SESSION src/function/iqs9150/session/contact_arbiter.c)
zephyr_library_sources_ifdef(CONFIG_TRAVELERS_PAD_IQS9150_CONTACT_SESSION src/function/iqs9150/session/contact_output_broker.c)
zephyr_library_sources_ifdef(CONFIG_TRAVELERS_PAD_IQS9150_TIP_TAP src/function/iqs9150/tip_tap/iqs9150_tip_tap_kscan.c)
zephyr_library_sources_ifdef(CONFIG_TRAVELERS_PAD_IQS9150_TRANSFORM_GESTURE src/function/iqs9150/transform_gesture/iqs9150_transform_gesture.c)

zephyr_library_sources_ifdef(CONFIG_TRAVELERS_PAD_XIAO_BOOT_PROBE src/platform/xiao/xiao_boot_probe.c)
