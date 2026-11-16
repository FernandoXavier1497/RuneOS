/* ramfs.c - sistema de arquivos em RAM. Sem dependencias de libc. */
#include "ramfs.h"

#define ROOT 0

typedef struct {
    uint8_t  used;
    uint8_t  is_dir;
    int16_t  parent;
    int16_t  first_child;
    int16_t  next_sibling;
    uint32_t size;
    int16_t  blocks[RFS_DIRECT];
    char     name[RFS_NAME_MAX + 1];
} node_t;

static node_t  nodes[RFS_MAX_NODES];
static uint8_t pool[RFS_NUM_BLOCKS][RFS_BLOCK_SIZE];
static uint8_t block_used[RFS_NUM_BLOCKS];
static int     cwd_node;


static uint32_t r_strlen(const char *s) { uint32_t n = 0; while (s[n]) n++; return n; }
static int r_streq(const char *a, const char *b) { while (*a && *a == *b) { a++; b++; } return *a == *b; }
static int r_strcmp(const char *a, const char *b) {
    while (*a && *a == *b) { a++; b++; }
    return (int)(uint8_t)*a - (int)(uint8_t)*b;
}
static void r_strcpy(char *d, const char *s) { while ((*d++ = *s++)) ; }
static void r_memcpy(void *d, const void *s, uint32_t n) {
    uint8_t *dd = (uint8_t *)d; const uint8_t *ss = (const uint8_t *)s;
    while (n--) *dd++ = *ss++;
}
static void r_memset(void *d, uint8_t v, uint32_t n) {
    uint8_t *dd = (uint8_t *)d;
    while (n--) *dd++ = v;
}
static uint32_t blocks_of(uint32_t bytes) { return (bytes + RFS_BLOCK_SIZE - 1) / RFS_BLOCK_SIZE; }

uint32_t rfs_free_blocks(void) {
    uint32_t n = 0;
    for (int i = 0; i < RFS_NUM_BLOCKS; i++) if (!block_used[i]) n++;
    return n;
}
uint32_t rfs_used_nodes(void) {
    uint32_t n = 0;
    for (int i = 0; i < RFS_MAX_NODES; i++) if (nodes[i].used) n++;
    return n;
}
static int alloc_block(void) {
    for (int i = 0; i < RFS_NUM_BLOCKS; i++)
        if (!block_used[i]) { block_used[i] = 1; return i; }
    return -1;
}
static void release_blocks(int n) {
    for (int i = 0; i < RFS_DIRECT; i++) {
        if (nodes[n].blocks[i] >= 0) { block_used[nodes[n].blocks[i]] = 0; nodes[n].blocks[i] = -1; }
    }
    nodes[n].size = 0;
}


static void node_init(int n, int parent, const char *name, int is_dir) {
    node_t *x = &nodes[n];
    x->used = 1; x->is_dir = (uint8_t)is_dir; x->parent = (int16_t)parent;
    x->first_child = -1; x->next_sibling = -1; x->size = 0;
    for (int i = 0; i < RFS_DIRECT; i++) x->blocks[i] = -1;
    r_strcpy(x->name, name);
}

void rfs_init(void) {
    r_memset(nodes, 0, sizeof nodes);
    r_memset(block_used, 0, sizeof block_used);
    node_init(ROOT, ROOT, "/", 1);
    cwd_node = ROOT;
}

static int lookup_child(int dir, const char *name) {
    for (int c = nodes[dir].first_child; c >= 0; c = nodes[c].next_sibling)
        if (r_streq(nodes[c].name, name)) return c;
    return -1;
}

static void link_child(int parent, int n) {
    nodes[n].next_sibling = -1;
    nodes[n].parent = (int16_t)parent;
    if (nodes[parent].first_child < 0) { nodes[parent].first_child = (int16_t)n; return; }
    int c = nodes[parent].first_child;
    while (nodes[c].next_sibling >= 0) c = nodes[c].next_sibling;
    nodes[c].next_sibling = (int16_t)n;
}

static void unlink_node(int n) {
    int p = nodes[n].parent;
    if (nodes[p].first_child == n) { nodes[p].first_child = nodes[n].next_sibling; }
    else {
        int c = nodes[p].first_child;
        while (c >= 0 && nodes[c].next_sibling != n) c = nodes[c].next_sibling;
        if (c >= 0) nodes[c].next_sibling = nodes[n].next_sibling;
    }
    nodes[n].next_sibling = -1;
}

static void free_tree(int n) {
    int c = nodes[n].first_child;
    while (c >= 0) { int next = nodes[c].next_sibling; free_tree(c); c = next; }
    release_blocks(n);
    nodes[n].used = 0;
}

static int new_node(int parent, const char *name, int is_dir) {
    if (r_strlen(name) > RFS_NAME_MAX) return RFS_ERR_NAMETOOLONG;
    if (lookup_child(parent, name) >= 0) return RFS_ERR_EXIST;
    int n = -1;
    for (int i = 1; i < RFS_MAX_NODES; i++) if (!nodes[i].used) { n = i; break; }
    if (n < 0) return RFS_ERR_NOSPACE;
    node_init(n, parent, name, is_dir);
    link_child(parent, n);
    return n;
}

static int is_ancestor(int a, int b) { 
    int x = b;
    for (;;) {
        if (x == a) return 1;
        if (x == ROOT) return 0;
        x = nodes[x].parent;
    }
}


static int walk(int start, const char *path) {
    int cur = start;
    const char *p = path;
    if (*p == '/') cur = ROOT;
    while (*p) {
        while (*p == '/') p++;
        if (!*p) break;
        char comp[RFS_NAME_MAX + 1];
        int n = 0;
        while (*p && *p != '/') {
            if (n >= RFS_NAME_MAX) return RFS_ERR_NAMETOOLONG;
            comp[n++] = *p++;
        }
        comp[n] = 0;
        if (r_streq(comp, ".")) continue;
        if (!nodes[cur].is_dir) return RFS_ERR_NOTDIR;
        if (r_streq(comp, "..")) { cur = nodes[cur].parent; continue; }
        int c = lookup_child(cur, comp);
        if (c < 0) return RFS_ERR_NOENT;
        cur = c;
    }
    return cur;
}

static int resolve(const char *path) {
    if (!path || !*path) return RFS_ERR_INVAL;
    return walk(cwd_node, path);
}

static int split_parent(const char *path, int *parent, char *name) {
    if (!path || !*path) return RFS_ERR_INVAL;
    int len = (int)r_strlen(path);
    while (len > 1 && path[len - 1] == '/') len--;
    int i = len;
    while (i > 0 && path[i - 1] != '/') i--;
    int nlen = len - i;
    if (nlen == 0) return RFS_ERR_INVAL;
    if (nlen > RFS_NAME_MAX) return RFS_ERR_NAMETOOLONG;
    for (int k = 0; k < nlen; k++) name[k] = path[i + k];
    name[nlen] = 0;
    if (r_streq(name, ".") || r_streq(name, "..")) return RFS_ERR_INVAL;
    int p;
    if (i == 0) p = cwd_node;
    else {
        char pp[RFS_PATH_MAX];
        if (i >= RFS_PATH_MAX) return RFS_ERR_NAMETOOLONG;
        for (int k = 0; k < i; k++) pp[k] = path[k];
        pp[i] = 0;
        p = walk(cwd_node, pp);
        if (p < 0) return p;
    }
    if (!nodes[p].is_dir) return RFS_ERR_NOTDIR;
    *parent = p;
    return RFS_OK;
}


static int resolve_dest(const char *dst, const char *srcname, int *parent, char *name) {
    if (!dst || !*dst) return RFS_ERR_INVAL;
    int d = walk(cwd_node, dst);
    if (d >= 0) {
        if (!nodes[d].is_dir) return RFS_ERR_EXIST;
        *parent = d;
        r_strcpy(name, srcname);
        return RFS_OK;
    }
    if (d != RFS_ERR_NOENT) return d;
    return split_parent(dst, parent, name);
}


const char *rfs_strerror(int c) {
    switch (c) {
    case RFS_OK:               return "ok";
    case RFS_ERR_NOENT:        return "no such file or directory";
    case RFS_ERR_EXIST:        return "already exists";
    case RFS_ERR_NOTDIR:       return "not a directory";
    case RFS_ERR_ISDIR:        return "is a directory";
    case RFS_ERR_NOSPACE:      return "no space left";
    case RFS_ERR_INVAL:        return "invalid argument";
    case RFS_ERR_NOTEMPTY:     return "directory not empty (use -r)";
    case RFS_ERR_NAMETOOLONG:  return "name too long";
    default:                  return "unknown error";
    }
}

int rfs_chdir(const char *path) {
    int n = resolve(path);
    if (n < 0) return n;
    if (!nodes[n].is_dir) return RFS_ERR_NOTDIR;
    cwd_node = n;
    return RFS_OK;
}

int rfs_getcwd(char *buf, int max) {
    int stack[RFS_MAX_NODES], depth = 0, n = cwd_node;
    while (n != ROOT) { stack[depth++] = n; n = nodes[n].parent; }
    int pos = 0;
    if (max < 2) return RFS_ERR_INVAL;
    if (depth == 0) { buf[0] = '/'; buf[1] = 0; return RFS_OK; }
    while (depth > 0) {
        const char *nm = nodes[stack[--depth]].name;
        if (pos + 1 + (int)r_strlen(nm) >= max) return RFS_ERR_NAMETOOLONG;
        buf[pos++] = '/';
        while (*nm) buf[pos++] = *nm++;
    }
    buf[pos] = 0;
    return RFS_OK;
}

int rfs_mkdir(const char *path) {
    int parent; char name[RFS_NAME_MAX + 1];
    int rc = split_parent(path, &parent, name);
    if (rc < 0) return rc;
    rc = new_node(parent, name, 1);
    return rc < 0 ? rc : RFS_OK;
}

int rfs_touch(const char *path) {
    int parent; char name[RFS_NAME_MAX + 1];
    int rc = split_parent(path, &parent, name);
    if (rc < 0) return rc;
    if (lookup_child(parent, name) >= 0) return RFS_OK;
    rc = new_node(parent, name, 0);
    return rc < 0 ? rc : RFS_OK;
}

static int file_append(int n, const uint8_t *data, uint32_t len) {
    node_t *f = &nodes[n];
    if (f->size + len > RFS_MAX_FILE) return RFS_ERR_NOSPACE;
    uint32_t have = blocks_of(f->size), need = blocks_of(f->size + len);
    if (need > have && need - have > rfs_free_blocks()) return RFS_ERR_NOSPACE;
    for (uint32_t b = have; b < need; b++) {
        int blk = alloc_block();
        f->blocks[b] = (int16_t)blk;
        r_memset(pool[blk], 0, RFS_BLOCK_SIZE);
    }
    uint32_t off = f->size;
    while (len) {
        uint32_t bi = off / RFS_BLOCK_SIZE, bo = off % RFS_BLOCK_SIZE, chunk = RFS_BLOCK_SIZE - bo;
        if (chunk > len) chunk = len;
        r_memcpy(&pool[f->blocks[bi]][bo], data, chunk);
        data += chunk; off += chunk; len -= chunk;
    }
    f->size = off;
    return RFS_OK;
}

int rfs_write(const char *path, const void *data, uint32_t len, int append) {
    int parent; char name[RFS_NAME_MAX + 1];
    int rc = split_parent(path, &parent, name);
    if (rc < 0) return rc;
    int n = lookup_child(parent, name), created = 0;
    if (n >= 0) {
        if (nodes[n].is_dir) return RFS_ERR_ISDIR;
        if (!append) {
            if (len > RFS_MAX_FILE) return RFS_ERR_NOSPACE;
            uint32_t have = blocks_of(nodes[n].size), need = blocks_of(len);
            if (need > have && need - have > rfs_free_blocks()) return RFS_ERR_NOSPACE;
            release_blocks(n);
        }
    } else {
        if (len > RFS_MAX_FILE || blocks_of(len) > rfs_free_blocks()) return RFS_ERR_NOSPACE;
        n = new_node(parent, name, 0);
        if (n < 0) return n;
        created = 1;
    }
    rc = file_append(n, (const uint8_t *)data, len);
    if (rc < 0 && created) { unlink_node(n); free_tree(n); }
    return rc;
}

int rfs_read(const char *path, void *buf, uint32_t max) {
    int n = resolve(path);
    if (n < 0) return n;
    if (nodes[n].is_dir) return RFS_ERR_ISDIR;
    uint32_t len = nodes[n].size < max ? nodes[n].size : max, off = 0;
    uint8_t *out = (uint8_t *)buf;
    while (off < len) {
        uint32_t bi = off / RFS_BLOCK_SIZE, bo = off % RFS_BLOCK_SIZE, chunk = RFS_BLOCK_SIZE - bo;
        if (chunk > len - off) chunk = len - off;
        r_memcpy(out + off, &pool[nodes[n].blocks[bi]][bo], chunk);
        off += chunk;
    }
    return (int)len;
}

int rfs_remove(const char *path, int recursive) {
    int n = resolve(path);
    if (n < 0) return n;
    if (n == ROOT) return RFS_ERR_INVAL;
    if (is_ancestor(n, cwd_node)) return RFS_ERR_INVAL;   /* nao apaga a pasta atual */
    if (nodes[n].is_dir && nodes[n].first_child >= 0 && !recursive) return RFS_ERR_NOTEMPTY;
    unlink_node(n);
    free_tree(n);
    return RFS_OK;
}

int rfs_rename(const char *oldp, const char *newp) {
    int n = resolve(oldp);
    if (n < 0) return n;
    if (n == ROOT) return RFS_ERR_INVAL;
    int np; char name[RFS_NAME_MAX + 1];
    int rc = resolve_dest(newp, nodes[n].name, &np, name);
    if (rc < 0) return rc;
    if (nodes[n].is_dir && is_ancestor(n, np)) return RFS_ERR_INVAL;
    if (np == nodes[n].parent && r_streq(name, nodes[n].name)) return RFS_OK;
    if (lookup_child(np, name) >= 0) return RFS_ERR_EXIST;
    unlink_node(n);
    r_strcpy(nodes[n].name, name);
    link_child(np, n);
    return RFS_OK;
}

int rfs_copy(const char *src, const char *dst) {
    int n = resolve(src);
    if (n < 0) return n;
    if (nodes[n].is_dir) return RFS_ERR_ISDIR;
    int np; char name[RFS_NAME_MAX + 1];
    int rc = resolve_dest(dst, nodes[n].name, &np, name);
    if (rc < 0) return rc;
    if (lookup_child(np, name) >= 0) return RFS_ERR_EXIST;
    uint32_t need = blocks_of(nodes[n].size);
    if (need > rfs_free_blocks()) return RFS_ERR_NOSPACE;
    int c = new_node(np, name, 0);
    if (c < 0) return c;
    for (uint32_t b = 0; b < need; b++) {
        int blk = alloc_block();
        nodes[c].blocks[b] = (int16_t)blk;
        r_memcpy(pool[blk], pool[nodes[n].blocks[b]], RFS_BLOCK_SIZE);
    }
    nodes[c].size = nodes[n].size;
    return RFS_OK;
}

static void fill_info(rfs_info_t *o, int n) {
    r_strcpy(o->name, nodes[n].name);
    o->is_dir = nodes[n].is_dir;
    o->size = nodes[n].size;
}

int rfs_stat(const char *path, rfs_info_t *out) {
    int n = resolve(path);
    if (n < 0) return n;
    fill_info(out, n);
    return RFS_OK;
}

int rfs_readdir(const char *path, int index, rfs_info_t *out) {
    int n = resolve(path);
    if (n < 0) return n;
    if (!nodes[n].is_dir) return RFS_ERR_NOTDIR;
    int c = nodes[n].first_child;
    while (c >= 0 && index > 0) { c = nodes[c].next_sibling; index--; }
    if (c < 0) return RFS_ERR_END;
    fill_info(out, c);
    return RFS_OK;
}

static int info_before(const rfs_info_t *a, const rfs_info_t *b) {
    if (a->is_dir != b->is_dir) return a->is_dir > b->is_dir;
    return r_strcmp(a->name, b->name) < 0;
}

int rfs_list(const char *path, rfs_info_t *out, int max) {
    int n = resolve(path);
    if (n < 0) return n;
    if (!nodes[n].is_dir) return RFS_ERR_NOTDIR;
    int count = 0;
    for (int c = nodes[n].first_child; c >= 0 && count < max; c = nodes[c].next_sibling) {
        rfs_info_t tmp;
        fill_info(&tmp, c);
        int j = count++;
        while (j > 0 && info_before(&tmp, &out[j - 1])) { 
            r_strcpy(out[j].name, out[j - 1].name);
            out[j].is_dir = out[j - 1].is_dir;
            out[j].size = out[j - 1].size;
            j--;
        }
        r_strcpy(out[j].name, tmp.name);
        out[j].is_dir = tmp.is_dir;
        out[j].size = tmp.size;
    }
    return count;
}
