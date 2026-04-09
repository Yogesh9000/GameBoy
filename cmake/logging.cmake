function(apply_logging_settings target)
    # Map human-friendly names to spdlog numeric levels
    set(LOG_LEVEL_MAP_TRACE 0)
    set(LOG_LEVEL_MAP_DEBUG 1)
    set(LOG_LEVEL_MAP_INFO  2)
    set(LOG_LEVEL_MAP_WARN  3)
    set(LOG_LEVEL_MAP_ERROR 4)
    set(LOG_LEVEL_MAP_OFF   6)

    set(level_num ${LOG_LEVEL_MAP_${LOG_LEVEL}})
    if(NOT DEFINED level_num)
        message(FATAL_ERROR "Unknown LOG_LEVEL: ${LOG_LEVEL}. "
                            "Choose: TRACE DEBUG INFO WARN ERROR OFF")
    endif()

    target_compile_definitions(${target} PRIVATE
        # Compile-time gate: strips log calls below this level entirely
        SPDLOG_ACTIVE_LEVEL=${level_num}
        # Pass the string level so logging.cpp can set the runtime level too
        GB_LOG_LEVEL="${LOG_LEVEL}"
    )
endfunction()
