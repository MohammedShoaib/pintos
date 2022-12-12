#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"
#include <user/syscall.h>
#include "devices/input.h"
#include "devices/shutdown.h"
#include "filesys/file.h"
#include "filesys/filesys.h"

#define MAX_ARGS 3


static void syscall_handler (struct intr_frame *);
void get_args (struct intr_frame *f, int *arg, int num_of_args);
void syscall_halt (void);
int syscall_open(const char * file_name);
void validate_ptr (const void* vaddr);
void validate_str (const void* str);

bool FILE_LOCK_INIT = false;

/*
 * System call initializer
 * It handles the set up for system call operations.
 */
void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

/*
 * This method handles for various case of system command.
 * This handler invokes the proper function call to be carried
 * out base on the command line.
 */
static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  if (!FILE_LOCK_INIT)
  {
    lock_init(&file_system_lock);
    FILE_LOCK_INIT = true;
  }
  
  int arg[MAX_ARGS];
  int esp = getpage_ptr((const void *) f->esp);
  
  switch (* (int *) esp)
  {
    case SYS_HALT:
      syscall_halt();
      break;
      
    case SYS_EXIT:
      // fill arg with the amount of arguments needed
      get_args(f, &arg[0], 1);
      syscall_exit(arg[0]);
      break;
      
    case SYS_OPEN:
      // fill arg with amount of arguments needed
      get_args(f, &arg[0], 1);
      
      /* Check if command line is valid.
       * We do not want to open junk which can cause a crash 
       */
       validate_str((const void*)arg[0]);
     
     // get page pointer
      arg[0] = getpage_ptr((const void *)arg[0]);
      
      /* syscall_open(int filedes) */
      f->eax = syscall_open((const char *)arg[0]);  // open this file
      break;
      
    default:
      break;
  }
}

/* halt */
void
syscall_halt (void)
{
  shutdown_power_off(); // from shutdown.h
}

/* get arguments from stack */
void
get_args (struct intr_frame *f, int *args, int num_of_args)
{
  int i;
  int *ptr;
  for (i = 0; i < num_of_args; i++)
  {
    ptr = (int *) f->esp + i + 1;
    validate_ptr((const void *) ptr);
    args[i] = *ptr;
  }
}

/* System call exit 
 * Checks if the current thread to exit is a child.
 * If so update the child's parent_tid information accordingly.
 */
void
syscall_exit (int status)
{
  struct thread *cur = thread_current();
  if (is_thread_alive(cur->parent_tid) && cur->child_ptr)
  {
    if (status < 0)
    {
      status = -1;
    }
    cur->child_ptr->status = status;
  }
  printf("%s: exit(%d)\n", cur->name, status);
  thread_exit();
}

/* syscall_open */
int
syscall_open(const char *file_name)
{
    lock_acquire(&file_system_lock);
    struct file *file_ptr = filesys_open(file_name); // from filesys.h
    if (!file_ptr)
    {
        lock_release(&file_system_lock);
        return ERROR;
    }
    int filedes = add_file(file_ptr);
    lock_release(&file_system_lock);
    return filedes;
}

/* function to check if pointer is valid */
void
validate_ptr (const void *vaddr)
{
    if (vaddr < USER_VADDR_BOTTOM || !is_user_vaddr(vaddr))
    {
      // virtual memory address is not reserved for us (out of bound)
      syscall_exit(ERROR);
    }
}

/* function to check if string is valid */
void
validate_str (const void* str)
{
    for (; * (char *) getpage_ptr(str) != 0; str = (char *) str + 1);
}

/* get the pointer to page */
int
getpage_ptr(const void *vaddr)
{
  void *ptr = pagedir_get_page(thread_current()->pagedir, vaddr);
  if (!ptr)
  {
    syscall_exit(ERROR);
  }
  return (int)ptr;
}

/* find a child process based on pid */
struct child_proc* find_child_process(int pid)
{
  struct thread *t = thread_current();
  struct list_elem *e;
  struct list_elem *next;
  
  for (e = list_begin(&t->child_proc_list); e != list_end(&t->child_proc_list); e = next)
  {
    next = list_next(e);
    struct child_proc *child_ptr = list_entry(e, struct child_proc, elem);
    if (pid == child_ptr->pid)
    {
      return child_ptr;
    }
  }
  return NULL;
}

/* remove a specific child process */
void
remove_child_process (struct child_proc *child_ptr)
{
  list_remove(&child_ptr->elem);
  free(child_ptr);
}

/* remove all child processes for a thread */
void remove_all_child_processes (void) 
{
  struct thread *t = thread_current();
  struct list_elem *next;
  struct list_elem *e = list_begin(&t->child_proc_list);
  
  for (;e != list_end(&t->child_proc_list); e = next) {
    next = list_next(e);
    struct child_proc *child_ptr = list_entry(e, struct child_proc, elem);
    list_remove(&child_ptr->elem); //remove child process
    free(child_ptr);
  }
}

/* add file to file list and return file descriptor of added file*/
int
add_file (struct file *file_name)
{
    struct process_file *process_file_ptr = malloc(sizeof(struct process_file));
    if (!process_file_ptr)
    {
        return ERROR;
    }
    process_file_ptr->file = file_name;
    process_file_ptr->fd = thread_current()->fd;
    thread_current()->fd++;
    list_push_back(&thread_current()->file_list, &process_file_ptr->elem);
    return process_file_ptr->fd;

}

/* close the desired file descriptor */
void
process_close_file (int file_descriptor)
{
  struct thread *t = thread_current();
  struct list_elem *next;
  struct list_elem *e = list_begin(&t->file_list);
  
  for (;e != list_end(&t->file_list); e = next)
  {
    next = list_next(e);
    struct process_file *process_file_ptr = list_entry (e, struct process_file, elem);
    if (file_descriptor == process_file_ptr->fd || file_descriptor == CLOSE_ALL_FD)
    {
      file_close(process_file_ptr->file);
      list_remove(&process_file_ptr->elem);
      free(process_file_ptr);
      if (file_descriptor != CLOSE_ALL_FD)
      {
        return;
      }
    }
  }
}
