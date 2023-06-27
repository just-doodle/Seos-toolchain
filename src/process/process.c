#include "process.h"
#include "elf_loader.h"
#include "shell.h"
#include "compositor.h"
#include "keyboard.h"

// TODO: Fix a bug which occurs when changing from one process to other
// TODO: Add threading support
// TODO: Make Preemptive multitasking system

list_t *process_list;
pcb_t *current_process;
pcb_t *last_process;

char p__cwd[512];

pid_t curr_pid;
registers_t saved_context;

pid_t alloc_pid()
{
    return curr_pid++;
}

void context_switch(registers_t *p_regs, context_t *n_regs)
{
    if (last_process && last_process != current_process)
    {
        last_process->regs.eax = p_regs->eax;
        last_process->regs.ebx = p_regs->ebx;
        last_process->regs.ecx = p_regs->ecx;
        last_process->regs.edx = p_regs->edx;
        last_process->regs.esi = p_regs->esi;
        last_process->regs.edi = p_regs->edi;
        last_process->regs.ebp = p_regs->ebp;
        last_process->regs.esp = p_regs->useresp;
        last_process->regs.eflags = p_regs->eflags;
        last_process->regs.eip = p_regs->eip;
        serialprintf("LAST_PROCESS_EIP: 0x%x\n", p_regs->eip);
        asm volatile("mov %%cr3, %0" : "=r"(last_process->regs.cr3));
    }

    if (((page_directory_t *)n_regs->cr3) != NULL)
    {
        switch_page_dir((page_directory_t *)n_regs->cr3, 1);
    }

    pic_eoi(0);
    last_process = current_process;

    if (current_process->type == TASK_TYPE_KERNEL)
    {
        kernel_regs_switch(n_regs);
    }
    else if (current_process->type == TASK_TYPE_USER)
    {
        user_regs_switch(n_regs);
    }
}

pcb_t* get_process_by_pid(pid_t pid)
{
    foreach(t, process_list)
    {
        pcb_t *pcb = (pcb_t *)t->val;

        if(pcb->pid == pid)
        {
            return pcb;
        }
    }
    return NULL;
}

pid_t getpid()
{
    return current_process->pid;
}

int kill(pid_t pid, uint32_t sig)
{
    if(pid == current_process->pid)
    {
        exit(sig);
        return;
    }

    pcb_t* p = get_process_by_pid(pid);
    printf("[PMGR] Process %d killed with code %d.\n", pid, sig);
    serialprintf("[PMGR] Process %d killed with code %d.\n", pid, sig);

    window_close_by_pid(pid);

    p->state = TASK_STOPPED;

    if(p->page_dir != NULL)
    {
        kfree(p->page_dir);
    }

    list_remove_node(process_list, p->self);

    kfree(p);

    last_process = NULL;

    return 0;
}

void change_process(pid_t pid)
{
    pcb_t *pcb = get_process_by_pid(pid);

    if(pid == 0)
    {
        if(pcb == NULL)
        {
            kernel_panic("[PMGR] No process left! Never exit userspace init process...\n");
        }
    }

    while(pcb == NULL)
    {
        pcb = get_process_by_pid(pid--);
    }

    if(pcb->state == TASK_STOPPED)
    {
        pcb = get_process_by_pid((pid == 0 ? 0 : (pid--)));
    }

    current_process = pcb;

    context_switch(&saved_context, &(pcb->regs));
}

void exit(uint32_t ret)
{
    pid_t pid = current_process->pid;

    printf("[PMGR] Process %d exited with code %d.\n", pid, ret);
    serialprintf("[PMGR] Process %d exited with code %d.\n", pid, ret);

    current_process->state = TASK_STOPPED;

    window_close_by_pid(pid);

    for (size_t i = 0; i < current_process->args.argc; i++)
    {
        free(current_process->args.argv[i]);
    }

    free(current_process->args.argv);
    

    if(current_process->page_dir != NULL)
    {
        serialprintf("PDA: 0x%06x\n", current_process->page_dir_addr);
        kfree((void*)current_process->page_dir_addr);
    }

    list_remove_node(process_list, current_process->self);

    kfree(current_process);

    last_process = NULL;

    change_process((pid == 0 ? 0 : (pid-1)));
}

void create_process_from_routine(char *name, void *entrypoint, uint32_t type)
{
    pcb_t *pcb = ZALLOC_TYPES(pcb_t);

    strcpy(pcb->filename, name);
    pcb->pid = alloc_pid();

    pcb->entrypoint = (uint32_t)entrypoint;

    pcb->regs.esp = 0xC0000000;
    pcb->regs.eflags = 0x206;
    pcb->regs.eip = pcb->entrypoint;

    pcb->page_dir = kmalloc_a(sizeof(page_directory_t));
    memset(pcb->page_dir, 0, sizeof(page_directory_t));
    pcb->page_dir_addr = (uint32_t)pcb->page_dir;

    copy_page_dir(pcb->page_dir, kernel_page_dir);
    alloc_region(pcb->page_dir, 0xC0000000 - 4 * PAGE_SIZE, 0xC0000000, 0, 0, 1);
    pcb->regs.cr3 = (uint32_t)virt2phys(kernel_page_dir, pcb->page_dir);

    pcb->type = type;
    pcb->state = TASK_CREATED;

    pcb->handler = 0;

    pcb->self = list_insert_front(process_list, pcb);

    printf("[PMGR] Created process %d: %s.\n", pcb->pid, pcb->filename);
    serialprintf("[PMGR] Created process %d: %s.\n", pcb->pid, pcb->filename);

    use_handler(0);

    change_process(pcb->pid);
}

void execve(char* file, char** argv, char** env)
{
    if(current_process->execve_return == 0)
    {
        FILE* j = file_open(file, 0);
        if(j == NULL)
            return;
        pcb_t * p1 = ZALLOC_TYPES(pcb_t);
        p1->pid = alloc_pid();
        p1->regs.eip = (uint32_t)load_elf;
        p1->regs.eflags = 0x206;
        p1->self = list_insert_front(process_list, p1);
        strcpy(p1->filename, file);

        p1->type = TASK_TYPE_KERNEL;

        p1->stack = (void*)0xC0000000;
        p1->regs.esp = (0xC0000000 - 4 * 1024);

        p1->page_dir = kmalloc_a(sizeof(page_directory_t));
        memset(p1->page_dir, 0, sizeof(page_directory_t));
        p1->page_dir_addr = (uint32_t)p1->page_dir;
        copy_page_dir(p1->page_dir, kernel_page_dir);
        p1->regs.cr3 = (uint32_t)virt2phys(kernel_page_dir, p1->page_dir);

        p1->handler = 0;

        for(int j = 0; argv[j] != NULL; j++)
        {
            p1->args.argc++;
        }
        p1->args.argc++;
        p1->args.argv = zalloc(sizeof(uint32_t) * (p1->args.argc + 1));

        int i;
        for(i = 0; i < p1->args.argc; i++)
        {
            if(argv[i] == NULL)
                break;
            p1->args.argv[i+1] = strdup(argv[i]); 
        }
        p1->args.argv[i+1] = NULL;
        p1->args.argv[0] = strdup(p1->filename);
        p1->args.argv[i+2] = NULL;

        p1->state = TASK_CREATED;

        use_handler(0);

        current_process->execve_return = 1;

#if __ENABLE_DEBUG_SYMBOL_LOADING__
        FILE* fl = file_open(file, 0);
        uint32_t sz = vfs_getFileSize(fl);
        uint8_t* f = zalloc(sz);
        vfs_read(fl, 0, sz, f);
        load_symbol_table(f, sz, p1);
        vfs_close(fl);
#endif

        change_process(p1->pid);
    }
    else
    {
        current_process->execve_return = 0;
        return;
    }
}

pargs_t a;

pargs_t* get_args()
{
    memset(&a, 0, sizeof(pargs_t));
    a = current_process->args;
    return &a;
}

void getcwd(char* buf, uint32_t sz)
{
    strncpy(buf, current_process->cwd, sz);
}

int chdir(char* path)
{
    int y = sizeof(pcb_t);
    if(path == NULL)
    {
        return -1;
    }
    current_process->cwdlen = strlen(path);
    if(current_process->cwd != NULL)
        free(current_process->cwd);
    current_process->cwd = strdup(path);
    return 0;
}

void create_process(char* file)
{
    pcb_t * p1 = ZALLOC_TYPES(pcb_t);
    p1->pid = alloc_pid();
    p1->regs.eip = (uint32_t)load_elf;
    p1->regs.eflags = 0x206;
    p1->self = list_insert_front(process_list, p1);
    strcpy(p1->filename, file);

    p1->type = TASK_TYPE_KERNEL;

    p1->stack = (void*)0xC0000000;
    p1->regs.esp = (0xC0000000 - 4 * 1024);

    p1->page_dir = kmalloc_a(sizeof(page_directory_t));
    memset(p1->page_dir, 0, sizeof(page_directory_t));
    p1->page_dir_addr = (uint32_t)p1->page_dir;
    copy_page_dir(p1->page_dir, kernel_page_dir);
    p1->regs.cr3 = (uint32_t)virt2phys(kernel_page_dir, p1->page_dir);
    p1->state = TASK_CREATED;

    change_process(p1->pid);
}

void attach_handler(process_kbhandler_t handler)
{
    current_process->handler = handler;
}

void init_processManager()
{
    process_list = list_create();
    init_keyboard();
    //change_keyboard_handler(process_kbh);
    printf("[ PMGR ] Process manager initialized...\n");
}

void list_process()
{
    printf("No. of processes: %d\n", curr_pid);
    foreach (t, process_list)
    {
        pcb_t *p = t->val;
        printf("%s%d : Name: %s, Type: %s, State: %s, EIP: 0x%06x\n", (current_process->pid == p->pid ? ">" : " "), p->pid, p->filename, (p->type == TASK_TYPE_KERNEL ? "Kernel task" : "Userspace task"), (p->state == TASK_CREATED ? "Created" : (p->state == TASK_RUNNING ? "Running" : (p->state == TASK_LOADING ? "Loading" : "Unknown"))), p->regs.eip);
    }
}

int isPause = 0;

int window_n = 0;

void process_kbh(uint8_t scancode)
{
    char key;
    int isCTRL = 0;
    int isALT = 0;

    if (scancode < 80)
    {
        switch (scancode)
        {
        case 0x46: //Scroll lock
            isPause = !isPause;
            break;
        case 0x4B:
            if(isPause)
            {
                // window_t* f = get_focused_window();
                // if(f->self->prev != NULL)
                //     window_focus(f->self->prev->val);
                // window_drawall();
                compositor_focus_previous();
                window_drawall();
            }break;
        case 0x4D:
            if(isPause)
            {
                // window_t* f = get_focused_window();
                // if(f->self->next != NULL)
                //     window_focus(f->self->next->val);
                // window_drawall();

                compositor_focus_next();
                window_drawall();
            }break;
        case 0xFA:
            break;
        case 0x3B:
            if(isPause)
            {
                printf("Reboot!\n");
                serialprintf("Reboot!\n");
                reboot();
                isPause = false;
            }
            break;
        case 0x3C:
            if(isPause)
            {
                printf("[PMGR] Stopping process %d\n", current_process->pid);
                isPause = false;
                exit(12);
            }
            break;
        case 0x3D:
            if (isPause)
            {
                list_process();
                isPause = false;
            }
            break;
        case 0x3E:
            if(isPause)
            {
                less_exception();
                isPause = false;
            }
            break;
        case 0x3F:
            if(isPause)
            {
                list_descriptors();
                isPause = false;
            }
            break;
        case 0x40:
            if(isPause)
            {
                ifb_screenshot();
                isPause = false;
            }
            break;
        case 0x41:
            if(isPause)
            {
                printtime();
                isPause = false;
            }
            break;
        case 0x42:
            if(isPause)
            {
                int sz = get_allocated_size();
                int max = get_heap_size();
                serialprintf("[KHEAP] Heap used: %dMB/%dMB\n", sz/MB, max/MB);
                printf("[KHEAP] Heap used: %dMB/%dMB\n", sz/MB, max/MB);
                isPause = false;
            }
            break;
        case 0x43:
            if(isPause)
            {
                if(current_process->pid+1 != curr_pid-1)
                    change_process(0);
                else
                    change_process(current_process->pid+1);
                isPause = false;
            }
            break;
        case 0x44:
            if(isPause)
            {
                minimize_focused();
                isPause = false;
            }
            break;
        };
    }

    key = kcodeTochar(scancode);

    process_kbhandler_t handler = current_process->handler;

    if (current_process->handler != 0)
    {
        current_process->handler(key, isCTRL, isALT, scancode);
    }
}

uint32_t process_get_ticks()
{
    return current_process->ticks_since_boot;
}

uint32_t getuid()
{
    return current_process->uid;
}

int setuid(uint32_t new_uid)
{
    if(current_process->uid == USER_ID_ROOT)
    {
        current_process->uid = new_uid;
        return 0;
    }
    return -1;
}