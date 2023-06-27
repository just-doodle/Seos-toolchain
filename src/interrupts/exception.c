#include "isr.h"
#include "vga_text.h"
#include "debug.h"
#include "process.h"
#include "logdisk.h"

int isLessMSG = 0;

#define NO_KERNEL_OOPS 0

char* exception_messages[] =
{
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",
    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};

int maxE = 0;

void exception_handler(registers_t cps)
{
    if(cps.ino < 32)
    {
        printf("\033[31m");
#if __ENABLE_DEBUG_SYMBOL_LOADING__
#if NO_KERNEL_OOPS
        if(current_process != NULL)
        {
            text_clear();
            printf("\n\nKERNEL OOPS. EXCEPTION OCCURRED %d: %s IN PROCESS %d:[%s]\n", cps.ino, exception_messages[cps.ino], current_process->pid, current_process->filename);
            printf("Extended stack pointer = 0x%06x\n", cps.esp);
            printf("Extended instruction pointer  = 0x%06x\n", cps.eip);
            printf("Error code = 0x%06x\n", cps.ecode);
            uint32_t faulting_addr;
            asm volatile("mov %%cr2, %0" : "=r" (faulting_addr));
            printf("CR2 = 0x%06x\n", faulting_addr);
            printf("EBX = 0x%06x\n", cps.ebx);
            printf("Killing process: %d\n\n", current_process->pid);
            serialprintf("\n\nKERNEL OOPS. EXCEPTION OCCURRED %d: %s IN PROCESS %d:[%s]\n", cps.ino, exception_messages[cps.ino], current_process->pid, current_process->filename);
            serialprintf("Extended stack pointer = 0x%06x\n", cps.esp);
            serialprintf("Extended instruction pointer  = 0x%06x\n", cps.eip);
            serialprintf("Error code = 0x%06x\n", cps.ecode);
            serialprintf("CR2 = 0x%06x\n", faulting_addr);
            serialprintf("EBX = 0x%06x\n", cps.ebx);
            serialprintf("Killing process: %d\n\n", current_process->pid);
            backtrace();
            if(maxE >= 3)
                kernel_panic("Exception has occurred in kernel space. Please reboot.");
            maxE++;
            exit(12);
            goto exce_exit;
            return;
        }
#endif
#endif


        if(isLessMSG)
        {
            text_chcolor(VGA_RED, VGA_BLACK);
            printf("KERNEL PANIC. EXCEPTION OCCURRED %d: %s\n", cps.ino, exception_messages[cps.ino]);
            printf("Extended stack pointer = 0x%06x\n", cps.esp);
            printf("Extended instruction pointer  = 0x%06x\n", cps.eip);
            serialprintf("%s\n", exception_messages[cps.ino]);
            return;
        }
        else
        {
            text_chcolor(VGA_RED, VGA_BLACK);
            logdisk_change_policy(LOG_OFF);
            printf("KERNEL PANIC. EXCEPTION OCCURRED %d: %s\n", cps.ino, exception_messages[cps.ino]);
            printf("Extended stack pointer = 0x%06x\n", cps.esp);
            printf("Extended instruction pointer  = 0x%06x\n", cps.eip);
            printf("Code segment selector = 0x%06x\n", cps.cs);
            printf("Extended flags = 0x%06x\n", cps.eflags);
            printf("Error code = 0x%06x\n", cps.ecode);
            printf("\n");
            printf("Registers:\n");
            printf("EAX = 0x%06x\n", cps.eax);
            printf("EBX = 0x%06x\n", cps.ebx);
            printf("ECX = 0x%06x\n", cps.ecx);
            printf("EDX = 0x%06x\n", cps.edx);
            printf("ESI = 0x%06x\n", cps.esi);
            printf("EDI = 0x%06x\n", cps.edi);
            printf("EBP = 0x%06x\n", cps.ebp);
            ldprintf("KERNEL", LOG_ERR, "KERNEL PANIC. EXCEPTION OCCURRED %d: %s", cps.ino, exception_messages[cps.ino]);
            ldprintf("KERNEL", LOG_ERR, "Extended stack pointer = 0x%06x", cps.esp);
            ldprintf("KERNEL", LOG_ERR, "Extended instruction pointer  = 0x%06x", cps.eip);
            ldprintf("KERNEL", LOG_ERR, "Code segment selector = 0x%06x", cps.cs);
            ldprintf("KERNEL", LOG_ERR, "Extended flags = 0x%06x", cps.eflags);
            ldprintf("KERNEL", LOG_ERR, "Error code = 0x%06x", cps.ecode);
            ldprintf("KERNEL", LOG_ERR, "Registers:");
            ldprintf("KERNEL", LOG_ERR, "EAX = 0x%06x", cps.eax);
            ldprintf("KERNEL", LOG_ERR, "EBX = 0x%06x", cps.ebx);
            ldprintf("KERNEL", LOG_ERR, "ECX = 0x%06x", cps.ecx);
            ldprintf("KERNEL", LOG_ERR, "EDX = 0x%06x", cps.edx);
            ldprintf("KERNEL", LOG_ERR, "ESI = 0x%06x", cps.esi);
            ldprintf("KERNEL", LOG_ERR, "EDI = 0x%06x", cps.edi);
            ldprintf("KERNEL", LOG_ERR, "EBP = 0x%06x", cps.ebp);
            serialprintf("KERNEL PANIC. EXCEPTION OCCURRED %d: %s\n", cps.ino, exception_messages[cps.ino]);
            serialprintf("Extended stack pointer = 0x%06x\n", cps.esp);
            serialprintf("Extended instruction pointer  = 0x%06x\n", cps.eip);
            serialprintf("Code segment selector = 0x%06x\n", cps.cs);
            serialprintf("Extended flags = 0x%06x\n", cps.eflags);
            serialprintf("Error code = 0x%06x\n", cps.ecode);
            serialprintf("\n");
            serialprintf("Registers:\n");
            serialprintf("EAX = 0x%06x\n", cps.eax);
            serialprintf("EBX = 0x%06x\n", cps.ebx);
            serialprintf("ECX = 0x%06x\n", cps.ecx);
            serialprintf("EDX = 0x%06x\n", cps.edx);
            serialprintf("ESI = 0x%06x\n", cps.esi);
            serialprintf("EDI = 0x%06x\n", cps.edi);
            serialprintf("EBP = 0x%06x\n", cps.ebp);
            if(maxE >= 3)
                kernel_panic("Exception has occurred in kernel space. Please reboot.");
            maxE++;
            backtrace();
            serialprintf("%s\n", exception_messages[cps.ino]);
            compositor_background_fill();
            window_drawall();
            ifb_refresh();
        }
        printf("\033[m");
    }

exce_exit:
    if(interrupt_handlers[cps.ino] != 0)
    {
        isr_t handler = interrupt_handlers[cps.ino];
        handler(&cps);
    }
    else if(cps.ino == 0x80)
    {
        printf("[ISR] Syscall Handler is not yet initialized.\n");
    }
    else
    {
        kernel_panic("[ISR] Unhandled exception!");
    }
}

void less_exception()
{
    isLessMSG = !isLessMSG;
}