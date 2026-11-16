/* ramfs.h - sistema de arquivos em RAM (sem libc, sem malloc) para kernel freestanding */
#ifndef RAMFS_H
#define RAMFS_H

#include <stdint.h>

#define RFS_NAME_MAX    28
#define RFS_PATH_MAX    128
#define RFS_MAX_NODES   128         
#define RFS_BLOCK_SIZE  512
#define RFS_NUM_BLOCKS  256          
#define RFS_DIRECT      8            
#define RFS_MAX_FILE    (RFS_BLOCK_SIZE * RFS_DIRECT)   

#define RFS_OK               0
#define RFS_ERR_NOENT       -1
#define RFS_ERR_EXIST       -2
#define RFS_ERR_NOTDIR      -3
#define RFS_ERR_ISDIR       -4
#define RFS_ERR_NOSPACE     -5
#define RFS_ERR_INVAL       -6
#define RFS_ERR_NOTEMPTY    -7
#define RFS_ERR_NAMETOOLONG -8
#define RFS_ERR_END         -9      

typedef struct {
    char     name[RFS_NAME_MAX + 1];
    uint8_t  is_dir;
    uint32_t size;
} rfs_info_t;

void        rfs_init(void);
const char *rfs_strerror(int code);

int rfs_chdir(const char *path);
int rfs_getcwd(char *buf, int max);

int rfs_mkdir(const char *path);
int rfs_touch(const char *path);
int rfs_write(const char *path, const void *data, uint32_t len, int append);
int rfs_read(const char *path, void *buf, uint32_t max);   
int rfs_remove(const char *path, int recursive);
int rfs_rename(const char *oldp, const char *newp);        
int rfs_copy(const char *src, const char *dst);            

int rfs_stat(const char *path, rfs_info_t *out);
int rfs_list(const char *path, rfs_info_t *out, int max);   
int rfs_readdir(const char *path, int index, rfs_info_t *out); 

uint32_t rfs_free_blocks(void);
uint32_t rfs_used_nodes(void);

#endif
