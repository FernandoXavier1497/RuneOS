#include "command.h"
#include "init.h"
#include "kstdio.h"
#include "kstring.h"
#include "vga.h"
#include "clock.h"
#include "fs.h"
#include "fm.h"
#include "process.h"
#include "io.h"

static int cmd_help(int argc, char **argv)
{
    (void)argc; (void)argv;
    for (int i = 0; i < command_count; i++)
        kprintf("  %-8s %s\n", command_list[i].name, command_list[i].help);
    return 0;
}

static int cmd_clear(int argc, char **argv)
{
    (void)argc; (void)argv;
    vga_clear();
    return 0;
}

static int cmd_echo(int argc, char **argv)
{
    for (int i = 1; i < argc; i++) {
        if (i > 1) kputchar(' ');
        kputs(argv[i]);
    }
    kputchar('\n');
    return 0;
}

static int cmd_version(int argc, char **argv)
{
    (void)argc; (void)argv;
    kprintf("RuneOS %s (i386, protected mode)\n", RUNEOS_VERSION);
    return 0;
}

static int cmd_about(int argc, char **argv)
{
    (void)argc; (void)argv;
    print_banner();
    return 0;
}

static int cmd_uptime(int argc, char **argv)
{
    (void)argc; (void)argv;
    uint32_t s = clock_uptime_seconds();
    kprintf("up %02u:%02u:%02u (%u ticks)\n", s / 3600, (s / 60) % 60, s % 60, clock_ticks());
    return 0;
}

static int cmd_sleep(int argc, char **argv)
{
    if (argc < 2) { kputs("usage: sleep <ms>\n"); return -1; }
    clock_sleep_ms((uint32_t)katoi(argv[1]));
    return 0;
}

/* Gerenciador de arquivos (ramfs com pastas). Um unico handler atende todos os
 * comandos de arquivo: o fm_dispatch escolhe pelo argv[0]. */
static int cmd_fm(int argc, char **argv)
{
    rune_fm_init();                          /* so inicializa na primeira vez */
    return fm_dispatch(argc, argv) ? -1 : 0;
}

static int cmd_ps(int argc, char **argv)
{
    (void)argc; (void)argv;
    kputs("  PID  STATE    NAME\n");
    const process_t *p;
    for (int i = 0; (p = process_at(i)); i++)
        kprintf("  %-4u %-8s %s\n", p->pid, p->state == PROC_RUNNING ? "running" : "ready", p->name);
    return 0;
}

static int cmd_spawn(int argc, char **argv)
{
    if (argc < 2) { kputs("usage: spawn <name>\n"); return -1; }
    int pid = process_spawn(argv[1]);
    if (pid < 0) { kputs("spawn: process table full\n"); return -1; }
    kprintf("started '%s' with pid %d\n", argv[1], pid);
    return 0;
}

static int cmd_kill(int argc, char **argv)
{
    if (argc < 2) { kputs("usage: kill <pid>\n"); return -1; }
    int r = process_kill((uint32_t)katoi(argv[1]));
    if (r == -2) kputs("kill: that process is protected\n");
    if (r == -1) kputs("kill: no such process\n");
    return r;
}

static int cmd_color(int argc, char **argv)
{
    if (argc < 2) { kputs("usage: color <0-15>\n"); return -1; }
    int c = katoi(argv[1]);
    if (c < 0 || c > 15) { kputs("color: value must be 0-15\n"); return -1; }
    vga_set_color((enum vga_color)c, VGA_BLACK);
    return 0;
}

static int cmd_reboot(int argc, char **argv)
{
    (void)argc; (void)argv;
    kputs("Rebooting...\n");
    outb(0x64, 0xFE);                       /* pulse the CPU reset line */
    for (;;) __asm__ volatile ("hlt");
    return 0;
}

static int cmd_halt(int argc, char **argv)
{
    (void)argc; (void)argv;
    kputs("RuneOS halted. It is now safe to close the window.\n");
    for (;;) __asm__ volatile ("cli; hlt");
    return 0;
}

const command_t command_list[] = {
    { "help", "list commands",              cmd_help    },
    { "clear", "clear the screen",            cmd_clear   },
    { "echo", "print arguments",            cmd_echo    },
    { "version", "show the RuneOS version",       cmd_version },
    { "about", "show the RuneOS banner",      cmd_about   },
    { "uptime", "time since boot",              cmd_uptime  },
    { "sleep", "sleep <ms>",                  cmd_sleep   },
    { "fm", "file manager (w/s, Enter, q)", cmd_fm  },
    { "ls", "ls [-l] [dir]",            cmd_fm      },
    { "cd", "cd [dir]",                 cmd_fm      },
    { "pwd", "print current directory",  cmd_fm      },
    { "tree", "tree [dir]",               cmd_fm      },
    { "mkdir", "mkdir <dir>",                cmd_fm      },
    { "cat", "cat <file>",                cmd_fm      },
    { "touch", "touch <file>",                cmd_fm      },
    { "write", "write <file> <text...>",      cmd_fm      },
    { "append", "append <file> <text...>",      cmd_fm      },
    { "cp", "cp <src> <dst>",           cmd_fm      },
    { "mv", "mv <src> <dst> (rename)",  cmd_fm      },
    { "rm", "rm [-r] <path>",           cmd_fm      },
    { "df", "free space",               cmd_fm      },
    { "ps", "list processes",           cmd_ps      },
    { "spawn", "spawn <name>",                cmd_spawn   },
    { "kill", "kill <pid>",                 cmd_kill    },
    { "color", "color <0-15>",                cmd_color   },
    { "reboot", "restart the machine",          cmd_reboot  },
    { "halt", "stop the CPU",               cmd_halt    },
};

const int command_count = (int)(sizeof(command_list) / sizeof(command_list[0]));
