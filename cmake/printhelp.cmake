macro (configure_step STEP_NAME)
	message(STATUS "Configuring ${STEP_NAME}")
endmacro ()

macro (end_configure_step STEP_NAME)
	message(STATUS "Configuring ${STEP_NAME} - Done")
endmacro ()