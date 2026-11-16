#ifndef FM_H
#define FM_H

#include <stdint.h>


#define FM_KEY_ENTER      '\n'
#define FM_KEY_BACKSPACE  '\b'
#define FM_KEY_ESC        27
#define FM_KEY_UP         0x100
#define FM_KEY_DOWN       0x101
#define FM_KEY_LEFT       0x102
#define FM_KEY_RIGHT      0x103

#define FM_LINE_MAX  160
#define FM_MAX_ARGS  12

typedef struct {
    void (*putc)(char c); 
    int  (*getkey)(void);                 
    void (*clear)(void);                   
    void (*set_color)(uint8_t fg, uint8_t bg); 
} fm_io_t;

void fm_init(const fm_io_t *io);           
int  fm_exec(const char *line);            
int  fm_dispatch(int argc, char **argv);   
void fm_run_tui(void);                    
void rune_fm_init(void);                   

#endif
