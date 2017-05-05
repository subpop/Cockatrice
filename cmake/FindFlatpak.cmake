# FindFlatpak
# -----------
#
# Defines the following variables:
#
# ``FLATPAK_EXECUTABLE``
#   Path to the 'flatpak' program
# ``FLATPAK_BUILDER``
#   Path to the 'flatpak-builder' program

if(CMAKE_HOST_UNIX)
  find_program(FLATPAK_EXECUTABLE flatpak)
  find_program(FLATPAK_BUILDER flatpak-builder)
endif()
