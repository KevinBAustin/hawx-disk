#ifndef SCHEDULER_H
#define SCHEDULER_H
// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run.
//  - swtch to start running that process.
//  - eventually that process transfers control
//    via swtch back to the scheduler.
/*
 * Run the scheduler.
 * This function is called by each CPU after setting itself up.
 * Does not return.
 * Parameters:
 *   - None
 * Returns:
 *   - This function does not return.
 */
void scheduler(void);

// Give up the CPU for one scheduling round.
/*
 * Yield the CPU.
 * Give up the CPU for one scheduling round.
 * Parameters:
 *   - None
 * Returns:
 *   - None
 */
void yield(void);
#endif