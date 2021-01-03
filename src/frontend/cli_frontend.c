#include <stdio.h>
#include <unistd.h>
#include <cflags.h>
#include <log.h>
#include <system/n64system.h>
#include <mem/pif.h>
#include <rdp/rdp.h>
#include <rdp/parallel_rdp_wrapper.h>
#include <frontend/tas_movie.h>
#include <signal.h>

void usage(cflags_t* flags) {
    cflags_print_usage(flags,
                       "[OPTION]... FILE",
                       "n64, a dgb n64 emulator",
                       "https://github.com/Dillonb/n64");
}

void sig_handler(int signum) {
    if (signum == SIGUSR1) {
        delayed_log_set_verbosity(LOG_VERBOSITY_DEBUG);
    } else if (signum == SIGUSR2) {
        delayed_log_set_verbosity(LOG_VERBOSITY_WARN);
    }
}

int main(int argc, char** argv) {

    signal(SIGUSR1, sig_handler);
    signal(SIGUSR2, sig_handler);

    cflags_t* flags = cflags_init();
    cflags_flag_t * verbose = cflags_add_bool(flags, 'v', "verbose", NULL, "enables verbose output, repeat up to 4 times for more verbosity");

    bool interpreter = false;
    cflags_add_bool(flags, 'i', "interpreter", &interpreter, "Force the use of the interpreter");

    bool debug = false;
#ifdef N64_DEBUG_MODE
    char description[110];
    snprintf(description, sizeof(description), "Enable debug mode. Starts halted and listens on port %d for gdb. NOTE: implies -i!", GDB_CPU_PORT);
    cflags_add_bool(flags, 'd', "debug", &debug, description);
#endif

#ifndef N64_WIN
    const char* rdp_plugin_path = NULL;
    cflags_add_string(flags, 'r', "rdp", &rdp_plugin_path, "Load RDP plugin (Mupen64Plus compatible)");
#endif

    const char* tas_movie_path = NULL;
    cflags_add_string(flags, 'm', "movie", &tas_movie_path, "Load movie (Mupen64Plus .m64 format)");

    const char* pif_rom_path = NULL;
    cflags_add_string(flags, 'p', "pif", &pif_rom_path, "Load PIF ROM");

    cflags_parse(flags, argc, argv);
    if (flags->argc != 1) {
        usage(flags);
        return 1;
    }
    log_set_verbosity(verbose->count);
#ifdef N64_DEBUG_MODE
    // In debug builds, always log at least warnings.
    if (log_get_verbosity() < LOG_VERBOSITY_WARN) {
        log_set_verbosity(LOG_VERBOSITY_WARN);
    }
    // if debug mode is enabled, force the interpreter
    if (debug && !interpreter) {
        logwarn("Debug mode enabled, forcing the use of the interpreter!");
        interpreter = true;
    }
#endif
    n64_system_t* system;
#ifndef N64_WIN // Don't allow loading an RDP plugin on Windows
    if (rdp_plugin_path != NULL) {
        system = init_n64system(flags->argv[0], true, debug, OPENGL, interpreter);
        load_rdp_plugin(system, rdp_plugin_path);
    } else {
#endif
        system = init_n64system(flags->argv[0], true, debug, VULKAN, interpreter);
        load_parallel_rdp(system);
#ifndef N64_WIN
    }
#endif
    if (tas_movie_path != NULL) {
        load_tas_movie(tas_movie_path);
    }
    if (pif_rom_path) {
        load_pif_rom(system, pif_rom_path);
    } else if (access("pif.rom", F_OK) == 0) {
        logalways("Found PIF ROM at pif.rom, loading");
        load_pif_rom(system, "pif.rom");
    }
    pif_rom_execute(system);
#ifdef N64_DEBUG_MODE
#ifndef N64_WIN
    if (debug) {
        printf("Listening on 0.0.0.0:%d - Waiting for GDB to connect...\n", GDB_CPU_PORT);
        system->debugger_state.broken = true;
    }
#endif
#endif
    cflags_free(flags);
    n64_system_loop(system);
    n64_system_cleanup(system);
    return 0;
}