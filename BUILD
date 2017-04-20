cc_library(
    name = "verilated",
    srcs = [
         "include/verilated.cpp",
         "include/verilated_imp.h",
         "include/verilated_syms.h",
         "include/verilated_config.h",
         "include/verilatedos.h"
    ],
    hdrs = [
         "include/verilated.h",
         "include/verilated_heavy.h",
    ],
    includes = ["include/"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "verilated_vcd_c",
    srcs = [
         "include/verilated_vcd_c.cpp",
         "include/verilated_imp.h",
         "include/verilated_syms.h",
         "include/verilated_config.h",
         "include/verilatedos.h",
         "include/verilated.h",
         "include/verilated_heavy.h",
    ],
    hdrs = [
         "include/verilated_vcd_c.h",
    ],
    includes = ["include/"],
    visibility = ["//visibility:public"],
)
