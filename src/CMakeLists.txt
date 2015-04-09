add_library(poco_simple_handler SimplePocoHandler.cpp SimplePocoHandler.h)
target_link_libraries(poco_simple_handler PocoNet PocoFoundation)

set(PROGS send
          receive
          new_task
          worker
          emit_log
          receive_logs
          emit_log_direct
          receive_logs_direct
          emit_log_topic
          receive_logs_topic
          rpc_client
          rpc_server
)

foreach(item ${PROGS})
    add_executable(${item} "${item}.cpp")
    target_link_libraries(${item} amqp-cpp poco_simple_handler)    
endforeach(item)