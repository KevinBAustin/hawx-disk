#ifndef CONSOLE_H
#define CONSOLE_H

/*
 * Initialize the UART driver.
 * Parameters: None
 * Returns: None
 */
void uartinit(void);

/*
 * Handle a UART interrupt, raised because input has
 * arrived, or the UART is ready for more output, or
 * both. Called from trap.c.
 * Parameters: None
 * Returns: None
 */
void uartintr(void);

/*
 * If the UART is idle, and a chracter is waiting in the
 * console out port, send it. 
 */
void uartstart(void);

/*
 * Transmit a character via the UART, waiting for it to transmit completely.
 * Parameters:
 *  - c: The character to transmit.
 * Returns: None
 */
void uartputc(int c);

/*
 * Transmit all of the characters in the PORT_CONSOLEOUT port.
 * Send them one character at a time using uartputc.
 * Parameters: None
 * Returns: None
 */
void uartflush(void);

/*
 * Print to the console. Only understands %d, %x, %p, %s.
 * Parameters:
 *  - fmt: The format string.
 *  - ...: The values to format according to the format string.
 * Returns: None
 */
void printf(char *fmt, ...);

/*
 * Print to the specified port
 * Parameters:
 * - port: The port number to print to.
 * - fmt: The format string.
 * - ...: The values to format according to the format string.
 * Returns: None
 */
void pprintf(int port, char *fmt, ...);

/*
 * Panic! Print one last plea for help and then hardlock the kernel.
 * Parameters:
 *  - s: The plea for help message.
 * Returns: This function never returns.
 */
void panic(char *s) __attribute__((noreturn));

/*
 * Handy debug function for unit tests to print if a test passed.
 * Parameters:
 *  - passed: The result of the test, 1 if passed, 0 if failed.
 * Returns: None
 */
void print_pass(int passed);

#endif
