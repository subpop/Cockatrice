include(FindFlatpak)

if(DEFINED FLATPAK_EXECUTABLE)
    message("-- Check for Flatpak KDE runtime")
    execute_process(
        COMMAND ${FLATPAK_EXECUTABLE} --user --if-not-exists remote-add kderuntime --from https://distribute.kde.org/kderuntime.flatpakrepo
        OUTPUT_QUIET
    )

    execute_process(
        COMMAND ${FLATPAK_EXECUTABLE} --user info org.kde.Platform
        RESULT_VARIABLE MissingPlatform
        OUTPUT_QUIET
    )

    execute_process(
        COMMAND ${FLATPAK_EXECUTABLE} --user info org.kde.Sdk
        RESULT_VARIABLE MissingSdk
        OUTPUT_QUIET
    )

    if(${MissingPlatform} GREATER 0)
        execute_process(COMMAND ${FLATPAK_EXECUTABLE} --user install kderuntime org.kde.Platform)
    endif()

    if(${MissingSdk} GREATER 0)
        execute_process(COMMAND ${FLATPAK_EXECUTABLE} --user install kderuntime org.kde.Sdk)
    endif()

    unset(MissingPlatform)
    unset(MissingSdk)
endif()
