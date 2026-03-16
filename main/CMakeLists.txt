idf_component_register(
    SRCS "headless_node.cpp"
         "../core/boot.cpp"
         "../core/runtime.cpp"
         "../services/event_queue.cpp"
         "../services/logger.cpp"
         "../app/state_machine.cpp"
    INCLUDE_DIRS "."
                 "../core" 
                 "../services"
                 "../app"
)