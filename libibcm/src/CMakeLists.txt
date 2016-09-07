publish_headers(infiniband
  ../include/infiniband/cm.h
  )

rdma_library(ibcm libibcm.map 1 1.0.0
  cm.c
  )
target_link_libraries(ibcm LINK_PUBLIC ibverbs)
