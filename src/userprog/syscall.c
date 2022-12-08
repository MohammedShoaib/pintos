#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "pagedir.h"
#include "devices/shutdown.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  printf ("system call!\n");

  struct thread *t = thread_current ();
  int esp = (int) pagedir_get_page (t->pagedir, f->esp);

  switch (* (int *) esp)
  {
    case SYS_HALT:
      syscall_halt ();
      break;
    case SYS_EXIT:
      syscall_exit (0);
    default:
      printf("System call not found!\n");
      break;
  }

  thread_exit ();
}

void
syscall_halt ()
{
  shutdown_power_off ();
}

void
syscall_exit (int status)
{
  struct thread *t = thread_current ();

  thread_exit ();
}

// int
// getpage_ptr(const void *vaddr)
// {
//   void *ptr = pagedir_get_page(thread_current()->pagedir, vaddr);
//   if (!ptr)
//   {
//     syscall_exit(ERROR);
//   }
//   return (int)ptr;
// }