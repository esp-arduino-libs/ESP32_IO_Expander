idf_component_register(SRCS "test_esp_io_expander.cpp")

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-missing-field-initializers)
