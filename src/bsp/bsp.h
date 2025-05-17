#include <stdint.h>

void bsp_IO_config(uint8_t dbg);
int32_t bsp_IO_load();
void bsp_IO_store(int32_t data);
void bsp_timebase_preset(void (*pISR)(void), uint32_t freq);
void bsp_timebase_reload(void);
void bsp_debug(void);
void bsp_performance(void);