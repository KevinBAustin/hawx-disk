#ifndef DISK_H
#define DISK_H

/* 
 * Initialize the disk driver.
 */
void virtio_disk_init(void);

/*
 * Handle a disk interrupt, which is the result of a disk operation.
 */
void virtio_disk_intr(void);

/*
 * Start a disk operation to read or write a block.
 */
void virtio_disk_start(void);

// Some disk definitions
#define BSIZE 1024  // block size

#endif
