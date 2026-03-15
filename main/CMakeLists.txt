idf_component_register(
    SRCS "headless_node.cpp"
         "../core/boot.cpp"
         "../core/runtime.cpp"
         "../services/event_queue.cpp"
    INCLUDE_DIRS "."
                 "../core" 
                 "../services"
)