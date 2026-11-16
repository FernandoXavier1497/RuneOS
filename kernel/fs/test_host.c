#include <stdio.h>
#include <string.h>
#include "fm.h"
#include "ramfs.h"

static int script[512], sn, sp;
static void h_putc(char c) { putchar(c); }
static void h_clear(void) { puts("\n[CLEAR]"); }
static void h_color(uint8_t f, uint8_t b) { (void)f; (void)b; }
static int h_key(void) { return sp < sn ? script[sp++] : FM_KEY_ESC; }
static void keys(const char *s) { for (; *s; s++) script[sn++] = (unsigned char)*s; }

static int fails;
#define CHECK(c) do { if (!(c)) { printf("FALHOU: %s (linha %d)\n", #c, __LINE__); fails++; } } while (0)

int main(void) {
    fm_io_t io = { h_putc, h_key, h_clear, h_color };
    fm_init(&io);
    const char *cmds1[] = {
        "ls -l", "mkdir a b a/c", "cd a", "pwd", "touch x.txt", "write x.txt ola mundo", "append x.txt linha 2",
        "cat x.txt", "cp x.txt c/copia.txt", "mv x.txt y.txt", "ls", "cd ..", "tree /", "mv a b", "ls /b/a", 0
    };
    for (int i = 0; cmds1[i]; i++) { printf("\n$ %s\n", cmds1[i]); if (fm_exec(cmds1[i])) puts("(desconhecido)"); }

    rfs_info_t e;
    CHECK(rfs_stat("/b/a/y.txt", &e) == RFS_OK && e.size == 18);
    CHECK(rfs_stat("/b/a/c/copia.txt", &e) == RFS_OK && e.size == 18);
    CHECK(rfs_remove("/", 1) == RFS_ERR_INVAL);
    CHECK(rfs_mkdir("/b/a") == RFS_ERR_EXIST);
    CHECK(rfs_chdir("/welcome.txt") == RFS_ERR_NOTDIR);
    CHECK(rfs_rename("/b", "/b/a/c") == RFS_ERR_INVAL);

    const char *cmds2[] = {
        "rm a", "rm b", "rm -r b/a", "ls /b", "mkdir /b/a/z", "cd /b", "rm /b", "cd /", "rm /b", "cat /nada", "df", "foo", 0
    };
    for (int i = 0; cmds2[i]; i++) { printf("\n$ %s\n", cmds2[i]); if (fm_exec(cmds2[i])) puts("(desconhecido)"); }
    CHECK(rfs_stat("/b", &e) == RFS_ERR_NOENT);

  
    static char big[RFS_MAX_FILE + 1];
    memset(big, 'x', sizeof big);
    rfs_chdir("/");
    CHECK(rfs_write("/grande", big, RFS_MAX_FILE, 0) == RFS_OK);
    CHECK(rfs_write("/grande", big, 1, 1) == RFS_ERR_NOSPACE);
    CHECK(rfs_write("/grande2", big, RFS_MAX_FILE + 1, 0) == RFS_ERR_NOSPACE);
    CHECK(rfs_stat("/grande2", &e) == RFS_ERR_NOENT);
    /* encher o disco e liberar */
    uint32_t before = rfs_free_blocks();
    char nm[16]; int made = 0;
    for (int i = 0; i < 40; i++) { sprintf(nm, "/f%d", i); if (rfs_write(nm, big, RFS_MAX_FILE, 0) == RFS_OK) made++; }
    printf("\narquivos de 4KB criados: %d (livres antes %u)\n", made, before);
    for (int i = 0; i < 40; i++) { sprintf(nm, "/f%d", i); rfs_remove(nm, 0); }
    CHECK(rfs_free_blocks() == before);

   
    sn = sp = 0;
    keys("\n"); keys(" ");                    
    script[sn++] = FM_KEY_BACKSPACE;          
    keys("nnovo.txt\n"); keys("m"); keys("pasta2\n");
    script[sn++] = FM_KEY_DOWN;
    keys("sw");                               
    keys("q");
    printf("\n=== TUI ===\n");
    fm_run_tui();
    CHECK(rfs_stat("/novo.txt", &e) == RFS_OK);
    CHECK(rfs_stat("/pasta2", &e) == RFS_OK);

    printf("\n%s\n", fails ? "COM FALHAS" : "TUDO OK");
    return fails != 0;
}
