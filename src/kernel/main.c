#include <stdint.h>
#include "stdio.h"
#include "memory.h"
#include <hal/hal.h>
#include <arch/i686/irq.h>
#include <debug.h>


#include "kbio.h"

extern uint8_t __bss_start;
extern uint8_t __end;

void crash_me();

void timer(Registers* regs)
{
    printf(".");
}

void __attribute__((section(".entry"))) start(uint16_t bootDrive)
{
    memset(&__bss_start, 0, (&__end) - (&__bss_start));

    HAL_Initialize();

    /*log_debug("Main", "This is a debug msg!");
    log_info("Main", "This is an info msg!");
    log_warn("Main", "This is a warnibng msg!");
    log_err("Main", "This is an error msg!");
    log_crit("Main", "This is a critical msg!");
    printf("Nanobyte OS v0.1\n");*/
    printf("This operating system is under construction.\n");
    while (1)
       {
                printf("\nNIDOS> ");
                
                string ch = readStr();
                if(strEql(ch,"cmd"))
                {
                        printf("\nYou are allready in cmd\n");
                }
                else if(strEql(ch,"clear"))
                {
                        clearScreen();
                }
               
                
                else
                {
                        printf("\nBad command!\n");
                }
                
                printf("\n");        
       }
}

