#include "fm.h"
#include "ramfs.h"

#define VIEW_ROWS 18
#define ROW_W     60

static const fm_io_t *io;
static rfs_info_t entries[RFS_MAX_NODES];
static char linebuf[FM_LINE_MAX];
static char *args[FM_MAX_ARGS];
static char textbuf[RFS_MAX_FILE + 2];
static char vbuf[RFS_MAX_FILE + 1];
static char tpath[RFS_PATH_MAX];
static char pbuf[RFS_PATH_MAX];

static void put_c(char c) { io->putc(c); }
static void put_s(const char *s) { while (*s) io->putc(*s++); }
static void color(uint8_t fg, uint8_t bg) { if (io->set_color) io->set_color(fg, bg); }
static int f_strlen(const char *s) { int n = 0; while (s[n]) n++; return n; }
static int f_streq(const char *a, const char *b) { while (*a && *a == *b) { a++; b++; } return *a == *b; }
static char printable(char c) { return (c >= 32 && c < 127) ? c : (c == '\t' ? ' ' : '.'); }

static int u2s(uint32_t v, char *t) {
    char r[10]; int n = 0, i;
    if (v == 0) { t[0] = '0'; return 1; }
    while (v) { r[n++] = (char)('0' + (v % 10)); v /= 10; }
    for (i = 0; i < n; i++) t[i] = r[n - 1 - i];
    return n;
}
static void put_u(uint32_t v) { char t[11]; int n = u2s(v, t); for (int i = 0; i < n; i++) put_c(t[i]); }
static void report(const char *cmd, int rc) {
    put_s(cmd); put_s(": "); put_s(rfs_strerror(rc)); put_c('\n');
}

static void print_long(const rfs_info_t *e) {
    char t[11]; int n = 0;
    if (e->is_dir) { put_s("  <DIR>  "); }
    else {
        n = u2s(e->size, t);
        put_s("  ");
        for (int i = n; i < 5; i++) put_c(' ');
        for (int i = 0; i < n; i++) put_c(t[i]);
        put_s("  ");
    }
    put_s(e->name);
    if (e->is_dir) put_c('/');
    put_c('\n');
}

static void cmd_ls(int argc, char **a) {
    int longf = 0, col = 0;
    const char *path = ".";
    for (int i = 1; i < argc; i++) { if (f_streq(a[i], "-l")) longf = 1; else path = a[i]; }
    int n = rfs_list(path, entries, RFS_MAX_NODES);
    if (n == RFS_ERR_NOTDIR) {
        rfs_info_t e;
        if (rfs_stat(path, &e) == RFS_OK) print_long(&e);
        return;
    }
    if (n < 0) { report("ls", n); return; }
    for (int i = 0; i < n; i++) {
        if (longf) { print_long(&entries[i]); continue; }
        int len = f_strlen(entries[i].name) + (entries[i].is_dir ? 1 : 0);
        if (col + len + 2 > 78) { put_c('\n'); col = 0; }
        put_s(entries[i].name);
        if (entries[i].is_dir) put_c('/');
        put_s("  ");
        col += len + 2;
    }
    if (!longf && col) put_c('\n');
}

static void tree_rec(int depth) {
    rfs_info_t e;
    int len = f_strlen(tpath);
    for (int i = 0; rfs_readdir(tpath, i, &e) == RFS_OK; i++) {
        for (int k = 0; k < depth; k++) put_s("  ");
        put_s("- "); put_s(e.name);
        if (e.is_dir) put_c('/');
        put_c('\n');
        int nl = f_strlen(e.name);
        if (e.is_dir && depth < 8 && len + nl + 2 < RFS_PATH_MAX) {
            int p = len;
            if (!(len == 1 && tpath[0] == '/')) tpath[p++] = '/';
            for (int k = 0; k < nl; k++) tpath[p++] = e.name[k];
            tpath[p] = 0;
            tree_rec(depth + 1);
            tpath[len] = 0;
        }
    }
}

static void cmd_tree(int argc, char **a) {
    const char *p = argc > 1 ? a[1] : ".";
    rfs_info_t st;
    int rc = rfs_stat(p, &st);
    if (rc < 0) { report("tree", rc); return; }
    if (!st.is_dir) { report("tree", RFS_ERR_NOTDIR); return; }
    int n = f_strlen(p);
    if (n >= RFS_PATH_MAX) { report("tree", RFS_ERR_NAMETOOLONG); return; }
    for (int i = 0; i <= n; i++) tpath[i] = p[i];
    put_s(p); put_c('\n');
    tree_rec(1);
}

static void cmd_write(int argc, char **a, int append) {
    if (argc < 3) { put_s("usage: "); put_s(append ? "append" : "write"); put_s(" <file> <text...>\n"); return; }
    int n = 0;
    for (int i = 2; i < argc; i++) {
        if (i > 2 && n < RFS_MAX_FILE) textbuf[n++] = ' ';
        for (const char *s = a[i]; *s && n < RFS_MAX_FILE; s++) textbuf[n++] = *s;
    }
    textbuf[n++] = '\n';
    int rc = rfs_write(a[1], textbuf, (uint32_t)n, append);
    if (rc < 0) report(append ? "append" : "write", rc);
}

static void cmd_cat(const char *path) {
    int n = rfs_read(path, vbuf, RFS_MAX_FILE);
    if (n < 0) { report("cat", n); return; }
    for (int i = 0; i < n; i++) put_c(vbuf[i] == '\n' ? '\n' : printable(vbuf[i]));
    if (n > 0 && vbuf[n - 1] != '\n') put_c('\n');
}

static void cmd_help(void) {
    put_s("File manager commands:\n");
    put_s("  ls [-l] [dir]   list         cd [dir]       change directory\n");
    put_s("  pwd             current dir  tree [dir]     directory tree\n");
    put_s("  mkdir <dir>     new folder   touch <file>   new empty file\n");
    put_s("  cat <file>      show file    write <file> <text...>   replace\n");
    put_s("  append <file> <text...>      append\n");
    put_s("  cp <a> <b>      copy         mv <a> <b>     move/rename\n");
    put_s("  rm [-r] <path>  delete       df             free space\n");
    put_s("  fm              full-screen mode (arrows, Enter, n m e r c d q)\n");
}

int fm_dispatch(int argc, char **a) {
    const char *c = a[0];
    int rc;
    if (f_streq(c, "help") || f_streq(c, "ajuda")) cmd_help();
    else if (f_streq(c, "pwd")) { rfs_getcwd(pbuf, RFS_PATH_MAX); put_s(pbuf); put_c('\n'); }
    else if (f_streq(c, "cd")) { rc = rfs_chdir(argc > 1 ? a[1] : "/"); if (rc < 0) report("cd", rc); }
    else if (f_streq(c, "ls") || f_streq(c, "dir")) cmd_ls(argc, a);
    else if (f_streq(c, "tree")) cmd_tree(argc, a);
    else if (f_streq(c, "mkdir")) {
        if (argc < 2) put_s("usage: mkdir <dir>...\n");
        for (int i = 1; i < argc; i++) { rc = rfs_mkdir(a[i]); if (rc < 0) report("mkdir", rc); }
    }
    else if (f_streq(c, "touch")) {
        if (argc < 2) put_s("usage: touch <file>...\n");
        for (int i = 1; i < argc; i++) { rc = rfs_touch(a[i]); if (rc < 0) report("touch", rc); }
    }
    else if (f_streq(c, "cat")) { if (argc < 2) put_s("usage: cat <file>\n"); else cmd_cat(a[1]); }
    else if (f_streq(c, "write")) cmd_write(argc, a, 0);
    else if (f_streq(c, "append")) cmd_write(argc, a, 1);
    else if (f_streq(c, "cp")) {
        if (argc < 3) put_s("usage: cp <src> <dst>\n");
        else { rc = rfs_copy(a[1], a[2]); if (rc < 0) report("cp", rc); }
    }
    else if (f_streq(c, "mv")) {
        if (argc < 3) put_s("usage: mv <src> <dst>\n");
        else { rc = rfs_rename(a[1], a[2]); if (rc < 0) report("mv", rc); }
    }
    else if (f_streq(c, "rm")) {
        int rec = 0, any = 0;
        for (int i = 1; i < argc; i++) {
            if (f_streq(a[i], "-r")) { rec = 1; continue; }
            any = 1;
            rc = rfs_remove(a[i], rec);
            if (rc < 0) report("rm", rc);
        }
        if (!any) put_s("usage: rm [-r] <path>...\n");
    }
    else if (f_streq(c, "df")) {
        put_s("Free blocks: "); put_u(rfs_free_blocks()); put_c('/'); put_u(RFS_NUM_BLOCKS);
        put_s(" ("); put_u(RFS_BLOCK_SIZE); put_s(" bytes each)\n");
        put_s("Entries used: "); put_u(rfs_used_nodes()); put_c('/'); put_u(RFS_MAX_NODES); put_c('\n');
    }
    else if (f_streq(c, "clear") || f_streq(c, "cls")) io->clear();
    else if (f_streq(c, "fm") || f_streq(c, "files")) fm_run_tui();
    else return 1;
    return 0;
}

int fm_exec(const char *line) {
    int len = f_strlen(line);
    if (len >= FM_LINE_MAX) len = FM_LINE_MAX - 1;
    for (int i = 0; i < len; i++) linebuf[i] = line[i];
    linebuf[len] = 0;

    
    int argc = 0;
    char *s = linebuf;
    while (*s) {
        while (*s == ' ' || *s == '\t') s++;
        if (!*s) break;
        if (argc >= FM_MAX_ARGS) break;
        if (*s == '"') { s++; args[argc++] = s; while (*s && *s != '"') s++; if (*s) *s++ = 0; }
        else { args[argc++] = s; while (*s && *s != ' ' && *s != '\t') s++; if (*s) *s++ = 0; }
    }
    if (argc == 0) return 0;

    return fm_dispatch(argc, args);
}


static void line_pad(const char *txt, int width) {
    int n = 0;
    while (*txt && n < width) { put_c(*txt++); n++; }
    while (n < width) { put_c(' '); n++; }
}

static void fmt_row(char *o, const rfs_info_t *e, int sel) {
    int i, c = 2;
    for (i = 0; i < ROW_W; i++) o[i] = ' ';
    o[ROW_W] = 0;
    o[0] = sel ? '>' : ' ';
    for (const char *p = e->name; *p; p++) o[c++] = *p;
    if (e->is_dir) o[c++] = '/';
    char t[12]; int n;
    if (e->is_dir) { t[0]='<'; t[1]='D'; t[2]='I'; t[3]='R'; t[4]='>'; n = 5; }
    else n = u2s(e->size, t);
    int start = 58 - n;
    for (i = 0; i < n; i++) o[start + i] = t[i];
}

static void set_msg(char *dst, const char *s) {
    int i = 0;
    while (s[i] && i < 60) { dst[i] = s[i]; i++; }
    dst[i] = 0;
}

static int prompt(const char *label, char *buf, int max) {
    int n = 0;
    put_s(label);
    for (;;) {
        int k = io->getkey();
        if (k == '\n' || k == '\r') { buf[n] = 0; put_c('\n'); return n; }
        if (k == FM_KEY_ESC) return -1;
        if (k == FM_KEY_BACKSPACE || k == 127) {
            if (n > 0) { n--; put_c('\b'); put_c(' '); put_c('\b'); }
            continue;
        }
        if (k >= 32 && k < 127 && n < max - 1) { buf[n++] = (char)k; put_c((char)k); }
    }
}

static void view_file(const char *name) {
    int n = rfs_read(name, vbuf, RFS_MAX_FILE);
    io->clear();
    if (n < 0) { put_s("Error: "); put_s(rfs_strerror(n)); put_c('\n'); }
    else {
        int line = 0;
        for (int i = 0; i < n; i++) {
            if (vbuf[i] == '\n') {
                put_c('\n');
                if (++line >= 22) {
                    put_s("-- more (q quits) --");
                    int k = io->getkey();
                    put_c('\n');
                    if (k == 'q') break;
                    line = 0;
                }
            } else put_c(printable(vbuf[i]));
        }
    }
    put_s("\n[press any key]");
    io->getkey();
}

static void draw(int count, int sel, int top, const char *msg) {
    char row[ROW_W + 1];
    io->clear();
    color(0, 3);
    rfs_getcwd(pbuf, RFS_PATH_MAX);
    put_s(" File Manager  ");
    line_pad(pbuf, 53);
    color(7, 0);
    put_c('\n');
    for (int r = 0; r < VIEW_ROWS; r++) {
        int i = top + r;
        if (i < count) {
            fmt_row(row, &entries[i], i == sel);
            if (i == sel) color(0, 7);
            put_s(row);
            color(7, 0);
        }
        put_c('\n');
    }
    color(8, 0);
    put_s(" Enter open | Backspace up | n new file | m new folder | e edit | q quit\n");
    put_s(" r rename | c copy | d delete | w/s or arrows to move\n");
    color(14, 0);
    put_s(msg);
    color(7, 0);
    put_c('\n');
}

void fm_run_tui(void) {
    int sel = 0, top = 0, count;
    char msg[64] = "";
    char in[RFS_PATH_MAX];

    for (;;) {
        count = rfs_list(".", entries, RFS_MAX_NODES);
        if (count < 0) count = 0;
        if (sel >= count) sel = count ? count - 1 : 0;
        if (sel < top) top = sel;
        if (sel >= top + VIEW_ROWS) top = sel - VIEW_ROWS + 1;
        draw(count, sel, top, msg);
        msg[0] = 0;

        int k = io->getkey();
        const rfs_info_t *cur = count ? &entries[sel] : 0;
        int rc;

        if (k == 'q' || k == FM_KEY_ESC) break;
        else if (k == FM_KEY_UP || k == 'w' || k == 'k') { if (sel > 0) sel--; }
        else if (k == FM_KEY_DOWN || k == 's' || k == 'j') { if (sel < count - 1) sel++; }
        else if (k == FM_KEY_ENTER || k == '\r' || k == FM_KEY_RIGHT) {
            if (!cur) continue;
            if (cur->is_dir) { rfs_chdir(cur->name); sel = 0; top = 0; }
            else view_file(cur->name);
        }
        else if (k == FM_KEY_BACKSPACE || k == 127 || k == FM_KEY_LEFT) { rfs_chdir(".."); sel = 0; top = 0; }
        else if (k == 'n') {
            if (prompt("File name: ", in, 29) > 0) {
                rc = rfs_touch(in);
                set_msg(msg, rc < 0 ? rfs_strerror(rc) : "File created");
            }
        }
        else if (k == 'm') {
            if (prompt("Folder name: ", in, 29) > 0) {
                rc = rfs_mkdir(in);
                set_msg(msg, rc < 0 ? rfs_strerror(rc) : "Folder created");
            }
        }
        else if (k == 'e') {
            if (!cur) continue;
            if (cur->is_dir) { set_msg(msg, "That is a folder"); continue; }
            int n = prompt("Text (replaces content): ", textbuf, 120);
            if (n >= 0) {
                textbuf[n] = '\n';
                rc = rfs_write(cur->name, textbuf, (uint32_t)n + 1, 0);
                set_msg(msg, rc < 0 ? rfs_strerror(rc) : "File saved");
            }
        }
        else if (k == 'r') {
            if (!cur) continue;
            if (prompt("New name: ", in, 29) > 0) {
                rc = rfs_rename(cur->name, in);
                set_msg(msg, rc < 0 ? rfs_strerror(rc) : "Renamed");
            }
        }
        else if (k == 'c') {
            if (!cur) continue;
            if (cur->is_dir) { set_msg(msg, "Copying folders is not supported"); continue; }
            if (prompt("Copy to: ", in, RFS_PATH_MAX) > 0) {
                rc = rfs_copy(cur->name, in);
                set_msg(msg, rc < 0 ? rfs_strerror(rc) : "Copied");
            }
        }
        else if (k == 'd') {
            if (!cur) continue;
            put_s("Delete '"); put_s(cur->name); put_s("'? (y/n) ");
            int a = io->getkey();
            if (a == 's' || a == 'S' || a == 'y' || a == 'Y') {
                rc = rfs_remove(cur->name, 1);
                set_msg(msg, rc < 0 ? rfs_strerror(rc) : "Deleted");
            }
        }
    }
    io->clear();
}

void fm_init(const fm_io_t *i) {
    io = i;
    rfs_init();
    {
        static const char hello[] = "Welcome to the file manager!\nType 'fm' for full-screen mode.\n";
        rfs_write("/welcome.txt", hello, (uint32_t)(sizeof hello - 1), 0);
    }
}
