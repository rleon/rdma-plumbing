rdma_executable(iwpmd
  iwarp_pm_common.c
  iwarp_pm_helper.c
  iwarp_pm_server.c
  )
target_link_libraries(iwpmd
  ${NL_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
  )

install(FILES "../iwpmd.service" DESTINATION "${CMAKE_INSTALL_SYSTEMD_SERVICEDIR}")
install(FILES "../iwpmd_init" DESTINATION "${CMAKE_INSTALL_INITDDIR}" RENAME "iwpmd")
install(FILES "../iwpmd.conf" DESTINATION "${CMAKE_INSTALL_SYSCONFDIR}")
