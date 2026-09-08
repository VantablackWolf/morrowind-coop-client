add_subdirectory(shaders)
add_subdirectory(data)
add_subdirectory(data-mw)
add_subdirectory(lua_api)

if (NOT DEFINED OPENMW_RESOURCES_ROOT)
    return()
endif()

copy_resource_file("launcher/images/openmw.png" "${OPENMW_RESOURCES_ROOT}" "resources/openmw.png")

# Start of tes3mp addition
#
# The window icon. 0.47 installed mygui/ directly under resources/, so 0.8.1
# put the logo there; 0.51 moved mygui/ into resources/vfs/ and reads its own
# icon flat out of resources/. Follow upstream's placement rather than
# recreating a resources/mygui/ directory that nothing else uses.
copy_resource_file("tes3mp/tes3mp_logo.png" "${OPENMW_RESOURCES_ROOT}" "resources/tes3mp_logo.png")
# End of tes3mp addition
