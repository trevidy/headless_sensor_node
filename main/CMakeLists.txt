idf_component_register(
    SRCS "headless_node.cpp"
         "../core/boot.cpp"
         "../core/runtime.cpp"
    INCLUDE_DIRS "."
                 "../core" #.. is parent directory
)