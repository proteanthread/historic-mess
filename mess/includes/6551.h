/* 6551 ACIA */

#include "driver.h"

WRITE_HANDLER(acia_6551_w);
READ_HANDLER(acia_6551_r);

void	acia_6551_reset(void);
void	acia_6551_set_irq_callback(void (*callback)(int));