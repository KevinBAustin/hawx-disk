#ifndef TRAP_H
#define TRAP_H

/*
 * Initialize the trap handling for the kernel.
 * This sets up the necessary state so that the kernel can handle exceptions and
 * traps. Parameters:
 *   - None
 * Returns:
 *   - None
 */
void trapinit(void);

/*
 * Return to user space after a trap.
 * This is called after handling a trap from user space, and sets up the
 * necessary state to safely return control to user space. Parameters:
 *   - None
 * Returns:
 *   - None
 */
void usertrapret(void);

//plicinit.c
void plicinit(void);
int plic_claim(void);
void plic_complete(int);


#endif // TRAP_H
