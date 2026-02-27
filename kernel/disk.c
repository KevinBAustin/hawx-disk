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
  if(ports[PORT_DISKCMD].count < 16){
    msg.mode = 'N';
  }else{
    msg.mode = ports[PORT_DISKCMD].buffer[0];
    for(int i = 1; i < 8; i++){
      msg.blockid += atoi(ports[PORT_DISKCMD].buffer[i]);
    }
    for(int i = 8; i < 12; i++){
      msg.data_port += atoi(ports[PORT_DISKCMD].buffer[i]);
    }
    for(int i = 12; i < ports[PORT_DISKCMD].count; i++){
      msg.msg_port += atoi(ports[PORT_DISKCMD].buffer[i]);
    }
  }

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
  // The response should be written to the message port specified
  // in disk.info[id].
  // HINT: The pprintf function specified in console.h will come in
  //       handy here!
  // YOUR CODE HERE
  /*
  disk.info[id].mode = ports[PORT_DISKCMD].buffer[0];
  disk.info[id].status = status;
  for(int i = 2; i < 9; i++){
    disk.info[id].blockid += ports[PORT_DISKCMD].buffer[i];
  }*/
 
  /*
  ports[PORT_DISKCMD].buffer[0] = disk.info[id].mode;
  ports[PORT_DISKCMD].buffer[1] = status;
  for(int i = 2; i < ports[PORT_DISKCMD].count; i++){
    disk.info[id].blockid = ports[PORT_DISKCMD].buffer[i];
  }*/
  disk.info[id].mode = disk.buffer[id][0];
  disk.info[id].status = status;
  for(int i = 2; i < 9; i++){
    disk.info[id].blockid += disk.buffer[id][i];
  }
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
  
  uint32 status = 0;

  if(*R(VIRTIO_MMIO_MAGIC_VALUE) != 0x74726976 ||
     *R(VIRTIO_MMIO_VERSION) != 2 ||
     *R(VIRTIO_MMIO_DEVICE_ID) != 2 ||
     *R(VIRTIO_MMIO_VENDOR_ID) != 0x554d4551){
    panic("could not find virtio disk");
  }
  
  // reset device
  *R(VIRTIO_MMIO_STATUS) = status;

  // set ACKNOWLEDGE status bit
  status |= VIRTIO_CONFIG_S_ACKNOWLEDGE;
  *R(VIRTIO_MMIO_STATUS) = status;

  // set DRIVER status bit
  status |= VIRTIO_CONFIG_S_DRIVER;
  *R(VIRTIO_MMIO_STATUS) = status;

  // negotiate features
  uint64 features = *R(VIRTIO_MMIO_DEVICE_FEATURES);
  features &= ~(1 << VIRTIO_BLK_F_RO);
  features &= ~(1 << VIRTIO_BLK_F_SCSI);
  features &= ~(1 << VIRTIO_BLK_F_CONFIG_WCE);
  features &= ~(1 << VIRTIO_BLK_F_MQ);
  features &= ~(1 << VIRTIO_F_ANY_LAYOUT);
  features &= ~(1 << VIRTIO_RING_F_EVENT_IDX);
  features &= ~(1 << VIRTIO_RING_F_INDIRECT_DESC);
  *R(VIRTIO_MMIO_DRIVER_FEATURES) = features;

  // tell device that feature negotiation is complete.
  status |= VIRTIO_CONFIG_S_FEATURES_OK;
  *R(VIRTIO_MMIO_STATUS) = status;

  // re-read status to ensure FEATURES_OK is set.
  status = *R(VIRTIO_MMIO_STATUS);
  if(!(status & VIRTIO_CONFIG_S_FEATURES_OK))
    panic("virtio disk FEATURES_OK unset");

  // initialize queue 0.
  *R(VIRTIO_MMIO_QUEUE_SEL) = 0;

  // ensure queue 0 is not in use.
  if(*R(VIRTIO_MMIO_QUEUE_READY))
    panic("virtio disk should not be ready");

  // check maximum queue size.
  uint32 max = *R(VIRTIO_MMIO_QUEUE_NUM_MAX);
  if(max == 0)
    panic("virtio disk has no queue 0");
  if(max < NUM)
    panic("virtio disk max queue too short");

  // allocate and zero queue memory.
  disk.desc = vm_page_alloc();
  disk.avail = vm_page_alloc();
  disk.used = vm_page_alloc();
  if(!disk.desc || !disk.avail || !disk.used)
    panic("virtio disk kalloc");
  memset(disk.desc, 0, PGSIZE);
  memset(disk.avail, 0, PGSIZE);
  memset(disk.used, 0, PGSIZE);

  // set queue size.
  *R(VIRTIO_MMIO_QUEUE_NUM) = NUM;

  // write physical addresses.
  *R(VIRTIO_MMIO_QUEUE_DESC_LOW) = (uint64)disk.desc;
  *R(VIRTIO_MMIO_QUEUE_DESC_HIGH) = (uint64)disk.desc >> 32;
  *R(VIRTIO_MMIO_DRIVER_DESC_LOW) = (uint64)disk.avail;
  *R(VIRTIO_MMIO_DRIVER_DESC_HIGH) = (uint64)disk.avail >> 32;
  *R(VIRTIO_MMIO_DEVICE_DESC_LOW) = (uint64)disk.used;
  *R(VIRTIO_MMIO_DEVICE_DESC_HIGH) = (uint64)disk.used >> 32;

  // queue is ready.
  *R(VIRTIO_MMIO_QUEUE_READY) = 0x1;

  // all NUM descriptors start out unused.
  for(int i = 0; i < NUM; i++)
    disk.free[i] = 1;

  // tell device we're completely ready.
  status |= VIRTIO_CONFIG_S_DRIVER_OK;
  *R(VIRTIO_MMIO_STATUS) = status;

  // plic.c and trap.c arrange for interrupts from VIRTIO0_IRQ.
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
  
  if(ports[PORT_DISKCMD].count < 16)//Checks to see if there is a disk msg
    return;

  //allocates three descriptors  
  int idx[3];
  if(alloc3_desc(idx) == -1)
    return;
  
  //gets disk msg
  struct disk_msg msg = get_disk_msg();

  //Verifes the validity of the message
  if(msg.mode == 'N'){
    disk.info[idx[0]].msg_port = "fail";//Probably meant to use a VIRTIO_MMIO..., but can't figure out what it is
    free3_desc(idx);
    return;
  }

  int sector = msg.blockid * (BSIZE / 512);//Calculates sector

  disk.info[idx[0]].blockid = msg.blockid;
  disk.info[idx[0]].data_port = msg.data_port;
  disk.info[idx[0]].msg_port = msg.msg_port;
  disk.info[idx[0]].mode = msg.mode;
  
  //Disk formatting
  struct virtio_blk_req *buf = &disk.ops[idx[0]];
  if(msg.mode == 'W')
    buf->type = VIRTIO_BLK_T_OUT; // write the disk
  else
    buf->type = VIRTIO_BLK_T_IN; // read the disk
  buf->reserved = 0;
  buf->sector = sector;

  disk.desc[idx[0]].addr = (uint64) buf;
  disk.desc[idx[0]].len = sizeof(struct virtio_blk_req);
  disk.desc[idx[0]].flags = VRING_DESC_F_NEXT;
  disk.desc[idx[0]].next = idx[1];

  disk.desc[idx[1]].addr = (uint64) disk.info[idx[1]].data_port;
  disk.desc[idx[1]].len = BSIZE;
  if(msg.mode == 'W')
    disk.desc[idx[1]].flags = 0; // device reads b->data
  else
    disk.desc[idx[1]].flags = VRING_DESC_F_WRITE; // device writes b->data
  disk.desc[idx[1]].flags |= VRING_DESC_F_NEXT;
  disk.desc[idx[1]].next = idx[2];

  disk.info[idx[0]].status = 0xff; // device writes 0 on success
  disk.desc[idx[2]].addr = (uint64) &disk.info[idx[0]].status;
  disk.desc[idx[2]].len = 1;
  disk.desc[idx[2]].flags = VRING_DESC_F_WRITE; // device writes the status
  disk.desc[idx[2]].next = 0;

  // tell the device the first index in our chain of descriptors.
  disk.avail->ring[disk.avail->idx % NUM] = idx[0];

  __sync_synchronize();

  // tell the device another avail ring entry is available.
  disk.avail->idx += 1; // not % NUM ...

  __sync_synchronize();

  *R(VIRTIO_MMIO_QUEUE_NOTIFY) = 0; // value is queue number
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
  *R(VIRTIO_MMIO_INTERRUPT_ACK) = *R(VIRTIO_MMIO_INTERRUPT_STATUS) & 0x3;

  __sync_synchronize();

  while(disk.used_idx != disk.used->idx){
    __sync_synchronize();
    int id = disk.used->ring[disk.used_idx % NUM].id;

    if(disk.info[id].status != 0)
      panic("virtio_disk_intr status");

    if(disk.info[id].mode == 'R' && disk.info[id].status == 'S'){
      disk.info[id].data_port = disk.buffer[id];
      disk.info[id].msg_port = "success";//Probably meant to use a VIRTIO_MMIO..., but can't figure out what it is
    }
    wakeup(disk.info[id]);
    disk.used_idx += 1;
  }
}
