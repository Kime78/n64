add_executable(logtester logtester.c)
target_link_libraries(logtester common core)

add_executable(testcase_gen testcase_gen.c)
target_link_libraries(testcase_gen r4300i common core)

#add_executable(rsp_fuzzer rsp_fuzzer.c)
#target_link_libraries(rsp_fuzzer ftd2xx rsp common core)