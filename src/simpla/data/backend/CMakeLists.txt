#FIND_PACKAGE(VTK)
#MESSAGE("-- VTK version: ${VTK_VERSION}")
#SET(XML2_DIR ${PROJECT_SOURCE_DIR}/external_project/xdmf)
#SET(XDMF_DIR ${PROJECT_SOURCE_DIR}/external_project/xdmf)
#FIND_PACKAGE(XDMF)

#FILE(GLOB database_SRC  *.cpp *.h )

LIST(APPEND data_backend_SRC
        DataEntryMemory.h
        DataEntryMemory.cpp
        DataEntryLua.cpp
        DataEntryHDF5.cpp
        DataEntryXDMF.cpp
        DataEntryIMAS.cpp
        HDF5Common.cpp
        HDF5Common.h
        LuaObject.cpp
        LuaObject.h
        LuaObjectExt.h
        )
#IF (XDMF_FOUND)
#    LIST(APPEND data_backend_SRC DataEntryXDMF3.cpp)
#ENDIF (XDMF_FOUND)
ADD_LIBRARY(data_backend ${data_backend_SRC})
target_include_directories(data_backend BEFORE PRIVATE
        ${HDF5_INCLUDE_DIRS}
        ${LUA_INCLUDE_DIR}
        ${MPI_C_INCLUDE_PATH}
        ${XDMF_INCLUDE_DIRS}
        #${VTK_INCLUDE_DIR}
        )
TARGET_LINK_LIBRARIES(data_backend
        ${LUA_LIBRARIES}
        ${HDF5_LIBRARIES}
        ${MPI_LIBRARIES}
        ${XDMF_LIBRARIES}
        #${VTK_LIBRARIES}
        )