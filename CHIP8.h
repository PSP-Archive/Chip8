/** Vision8: CHIP8 emulator *************************************************/
/**                                                                        **/
/**                                CHIP8.h                                 **/
/**                                                                        **/
/** This file contains the portable CHIP8 emulation engine definitions     **/
/**                                                                        **/
/** Copyright (C) Marcel de Kogel 1997                                     **/
/**     You are not allowed to distribute this software commercially       **/
/**     Please, notify me, if you make any changes to this file            **/
/****************************************************************************/

#ifndef __CHIP8_H
#define __CHIP8_H

#ifndef EXTERN
#define EXTERN extern
#endif
 
typedef unsigned char byte;                     /* sizeof(byte)==1          */
typedef unsigned short word;                    /* sizeof(word)>=2          */

struct chip8_regs_struct
{
 byte alg[16];                                  /* 16 general registers     */
 byte delay,sound;                              /* delay and sound timer    */
 word i;                                        /* index register           */
 word pc;                                       /* program counter          */
 word sp;                                       /* stack pointer            */
};

EXTERN struct chip8_regs_struct chip8_regs;

#ifdef CHIP8_SUPER
#define CHIP8_WIDTH 128
#define CHIP8_HEIGHT 64
EXTERN byte chip8_super;	/* != 0 if in SCHIP display mode */
#else
#define CHIP8_WIDTH 64
#define CHIP8_HEIGHT 32
#endif

EXTERN byte chip8_iperiod;                      /* number of opcodes per    */
                                                /* timeslice (1/50sec.)     */
EXTERN byte chip8_keys[16];                     /* if 1, key is held down   */
EXTERN byte chip8_display[CHIP8_WIDTH*CHIP8_HEIGHT];/* 0xff if pixel is set,    */
                                                /* 0x00 otherwise           */
EXTERN byte chip8_mem[4096];                    /* machine memory. program  */
                                                /* is loaded at 0x200       */
EXTERN byte chip8_running;                      /* if 0, emulation stops    */

EXTERN void chip8_execute (void);                      /* execute chip8_iperiod    */
                                                /* opcodes                  */
EXTERN void chip8_reset (void);                        /* reset virtual machine    */
EXTERN void chip8 (void);                              /* start chip8 emulation    */

EXTERN void chip8_sound_on (void);                     /* turn sound on            */
EXTERN void chip8_sound_off (void);                    /* turn sound off           */
EXTERN void chip8_interrupt (void);                    /* update keyboard,         */
                                                /* display, etc.            */

#ifdef CHIP8_DEBUG
EXTERN byte chip8_trace;                        /* if 1, call debugger      */
                                                /* every opcode             */
EXTERN word chip8_trap;                         /* if pc==trap, set trace   */
                                                /* flag                     */
EXTERN void chip8_debug (word opcode,struct chip8_regs_struct *regs);
#endif

#endif          /* __CHIP8_H */

