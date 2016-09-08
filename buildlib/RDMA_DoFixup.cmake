# COPYRIGHT (c) 2016 Obsidian Research Corporation. See COPYING file

# Execute a header fixup based on NOT_NEEDED for HEADER

# The buildlib includes alternate header file shims for several scenarios, if
# the build system detects a feature is present then it should call OrcDoFixup
# with the test as true. If false then the shim header will be installed.

# Typically the shim header will replace a missing header with stubs, or it
# will augment an existing header with include_next.
function(RDMA_DoFixup not_needed header)
  set(DEST "${BUILD_INCLUDE}/${header}")
  if (NOT "${not_needed}")
    get_filename_component(DIR ${DEST} DIRECTORY)
    file(MAKE_DIRECTORY "${DIR}")
    string(REPLACE / - header-bl ${header})
    execute_process(COMMAND "ln" "-Tsf" "${BUILDLIB}/fixup-include/${header-bl}" "${DEST}")
  else()
    file(REMOVE ${DEST})
  endif()
endfunction()
