//
// driver for qemu's virtio disk device.
// uses qemu's mmio interface to virtio.
//
// qemu ... -drive file=fs.img,if=none,format=raw,id=x0 -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0
//

#include "types.h"
#include "riscv.h"
#include "memlayout.h"
#include "virtio.h"
#include "port.h"
#include "disk.h"
#include "string.h"
#include "mem.h"
#include "console.h"

// the address of virtio mmio register r.
#define R(r) ((volatile uint32 *)(VIRTIO0 + (r)))

static struct disk
{
  // a set (not a ring) of DMA descriptors, with which the
  // driver tells the device where to read and write individual
  // disk operations. there are NUM descriptors.
  // most commands consist of a "chain" (a linked list) of a couple of
  // these descriptors.
  struct virtq_desc *desc;

  // a ring in which the driver writes descriptor numbers
  // that the driver would like the device to process.  it only
  // includes the head descriptor of each chain. the ring has
  // NUM elements.
  struct virtq_avail *avail;

  // a ring in which the device writes descriptor numbers that
  // the device has finished processing (just the head of each chain).
  // there are NUM used ring entries.
  struct virtq_used *used;

  // our own book-keeping.
  char free[NUM];  // is a descriptor free?
  uint16 used_idx; // we've looked this far in used[2..NUM].

  // track info about in-flight operations,
  // for use when completion interrupt arrives.
  // indexed by first descriptor index of chain.
  struct
  {
    char mode;
    unsigned int blockid;
    unsigned int data_port;
    unsigned int msg_port;
    int status;
  } info[NUM];

  // disk command headers.
  // one-for-one with descriptors, for convenience.
  struct virtio_blk_req ops[NUM];

  // We need a set of disk buffers for pulling information
  // in and out of ports. The reason for this is that port data
  // may not necessarily align with the beginning of their internal
  // array. We will use these buffers in alignment with the info
  // array. So the id will index this buffer as well.
  char buffer[NUM][BSIZE];
} disk;

/*
 * Disk message to command the driver.
 */
struct disk_msg
{
  char mode;
  unsigned int blockid;
  unsigned int data_port;
  unsigned int msg_port;
};

/*
 * Read the next disk message from the PORT_DISKCMD port.
 * The message is formatted as follows:
 *   BLOCKID  - 7 Characters
 *   Data Port - 4 Characters
 *   Msg Port  - 4 Characters
 * Returns: The message (if there is one), otherwise a message with mode 'N'
 */
static struct disk_msg
get_disk_msg()
{
  struct disk_msg msg;

  // This function should read a message from the disk port. If there
  // is no message, it should return a message with 'N' mode.
  // To do this, you must do the following:
  // 1. Check to see if there are at least 16 characters in the port.
  //    If there are not at least 16 characters, return a message with
  //    mode 'N'.
  // 2. Read the message from the port, and parse it into the message struct.
  // HINT: The atoi function in string.h may be useful for converting
  //       strings to integers.
  // YOUR CODE HERE

  return msg;
}

/* Write a response to the disk command from the driver's
 * info array.
 * Parameters:
 *  status - The status of the disk command
 *  id - The id of the disk command for retrieving message info
 */
static void
write_disk_response(char status, int id)
{
  // This function should write a response to the disk command.
  // The response should be formatted as follows:
  //   MODE  - 1 Character
  //   STATUS - 1 Character
  //   BLOCKID - 7 Characters
  // The response shoudl be written to the message port specified
  // in disk.info[id].
  // HINT: The pprintf function specified in console.h will come in
  //       handy here!
  // YOUR CODE HERE
}

/*
 * Initialize the virtio disk device.
 */
void virtio_disk_init(void)
{
  // This function should initialize the virtio disk device.
  // This is largely identical to the corresponding function in xv6
  // with a few differences:
  //   - The name of our allocator for memory pages is vm_page_alloc
  //   - Our kernel does not run on multiple cores, so we do not have
  //     the synchronization or spinlocks. This code should be ommitted.
  //   - We are not using the block cache from xv6, so that means the disk
  //     structure is a little different. Take a moment now to
  //     study the structure specified at the top of this file and compare it
  //     to that of xv6. This won't have an impact on this function, but
  //     now is a good time to look it over.
  // Above all, be sure to read the corresponding code and chapters in xv6
  // to understand what is happening here.
  // YOUR CODE HERE
}

// find a free descriptor, mark it non-free, return its index.
static int
alloc_desc()
{
  for (int i = 0; i < NUM; i++)
  {
    if (disk.free[i])
    {
      disk.free[i] = 0;
      return i;
    }
  }
  return -1;
}

// mark a descriptor as free.
static void
free_desc(int i)
{
  if (i >= NUM)
    panic("free_desc 1");
  if (disk.free[i])
    panic("free_desc 2");
  disk.desc[i].addr = 0;
  disk.desc[i].len = 0;
  disk.desc[i].flags = 0;
  disk.desc[i].next = 0;
  disk.free[i] = 1;
}

// free a chain of descriptors.
static void
free_chain(int i)
{
  while (1)
  {
    int flag = disk.desc[i].flags;
    int nxt = disk.desc[i].next;
    free_desc(i);
    if (flag & VRING_DESC_F_NEXT)
      i = nxt;
    else
      break;
  }
}

// free an array of up to 3 descriptors
static void
free3_desc(int *idx)
{
  for (int i = 0; i < 3; i++)
  {
    if (idx[i] < 0)
      continue;
    free_desc(idx[i]);
  }
}

// allocate three descriptors (they need not be contiguous).
// disk transfers always use three descriptors.
static int
alloc3_desc(int *idx)
{
  for (int i = 0; i < 3; i++)
  {
    idx[i] = alloc_desc();
    if (idx[i] < 0)
    {
      free3_desc(idx);
      return -1;
    }
  }
  return 0;
}

// start processing disk messages
void virtio_disk_start()
{
  // This function should start processing disk message if there is a message
  // and if the disk has available descriptors. To do this, you must do the
  // following:
  //   1. Check to see if there is a disk message waiting for us. You will
  //      want to do this by checking the PORT_DISKCMD port. Messages are
  //      16 characters long. If there is no pending message, return.
  //   2. Attempt to allocate three descriptors. If you cannot allocate
  //      three descriptors, return.
  //   3. Extract the disk message from the PORT_DISKCMD port.
  //   4. Verify that the message is valid. A valid message has a mode
  //      of 'W' or 'R' and the data port has the correct number of bytes
  //      (BSIZE) for a write operation or is empty for a read operation.
  //      If the message is invalid, write a failure response to the message
  //      port, free the descriptors, and return
  //   5. Calculate the sector from the disk message
  //   6. Format the three descriptors. Use xv6 as a reference for how to do
  //      this. There are a few things to keep in mind:
  //        - We are using the buffers defined in the disk structure for our
  //          disk memory interactions. The index of the buffer you should use
  //          is the first index in the chain of descriptors.
  //        - The info[idx[0]] struct should be filled out with the mode,
  //          blockid, data_port, and msg_port from the disk message.
  //        - Since we are running a single-core kernel, we will not have
  //          synchronization or spin locks.
  //   7. Tell the device the first index in our chain of descriptors, and
  //      tell the device another avail ring entry is available.
  // YOUR CODE HERE
}

void virtio_disk_intr()
{
  // This function gets called in the interrupt handler
  // when the disk has signalled an interrupt. It should do the following:
  //   1. Acknowledge the interrupt
  //   2. Process all completed disk operations. For each completed operation:
  //      1. If the disk info status is not 0, write a failure response to the
  //         message port.
  //      2. If the disk info mode is 'R', and it succeeded,
  //         write the data to the data port.
  //      3. If the message succeeded, write a success response to the
  //         message port.
  //      4. Free the chain of descriptors. and update the used_idx.
  //  3. Start the disk again.
  //  HINT: This is somewhat similar to the xv6 disk interrupt, with the
  //        differences being in the handling of errors and reporting back to
  //        the message ports.Be sure to read the corresponding code in xv6
  //        to understand what is happening.
  // YOUR CODE HERE
}
