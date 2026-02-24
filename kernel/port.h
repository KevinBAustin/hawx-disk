#ifndef PORT_H
#define PORT_H

#include "types.h"

// Ports for IPC
#define NPORT 256          // Number of ports
#define PORT_BUF_SIZE 1024 // Buffer size per port

// Predefined ports
#define PORT_CONSOLEIN  0 // Serial input
#define PORT_CONSOLEOUT 1 // Serial output
#define PORT_DISKCMD    2 // Disk command port

// Possible port uses
#define PORT_TYPE_FREE 0   // Port is free to allocate
#define PORT_TYPE_KERNEL 1 // Port is used by kernel


/*
 * Initialize the ports.
 * Parameters: None
 * Returns: None
 */
void port_init(void);

/*
 * Close the port.
 * Parameters:
 *  - port: The port number to close.
 * Returns: None
 */
void port_close(int port);

/*
 * Acquire a port for a process.
 * If the specified port number is -1, allocate the next available port.
 * Parameters:
 *  - port: The port number to acquire (-1 for any port).
 *  - proc_id: ID of the process that is acquiring the port.
 * Returns:
 *  - The port number on success, -1 on failure.
 */
int port_acquire(int port, procid_t proc_id);

/*
 * Write data to a port.
 * If the port is not open, the function returns -1.
 * If the buffer fills up before n bytes are written, the function returns.
 * Parameters:
 *  - port: The port number to write to.
 *  - buf: Pointer to the data buffer to write.
 *  - n: Number of bytes to write.
 * Returns:
 *  - The number of bytes actually written, -1 on failure.
 */
int port_write(int port, char *buf, int n);

/*
 * Read data from a port.
 * If the port is not open, the function returns -1.
 * If the port contents are exhausted before n bytes are read, the function
 * returns. Parameters:
 *  - port: The port number to read from.
 *  - buf: Pointer to the buffer to store the read data.
 *  - n: Number of bytes to read.
 * Returns:
 *  - The number of bytes actually read, -1 on failure.
 */
int port_read(int port, char *buf, int n);

// Define the Port struct with buffer, head, tail, etc.
struct port {
  int free;                   // Is port free?
  int owner;                  // ID of the process owning the port
  int type;                   // Type of the port (free or kernel)
  int head, tail;             // Indexes for the circular buffer
  int count;                  // Number of items in buffer
  char buffer[PORT_BUF_SIZE]; // Data buffer
};

// The ports array
extern struct port ports[];

#endif // PORT_H
