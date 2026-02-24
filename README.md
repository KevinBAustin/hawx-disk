Introduction
============
In this assignment, you will be implementing the hard disk driver for the HAWX
operating system. Recall the basic layout of the HAWX system:

    +------------+                +-----------+
    |  Computer  |     UART       | SERIAL    |
    |    CPU     | <------------> |  TERMINAL |
    |    RAM     |                |           |
    +------------+                +-----------+
          ^                      / .:::::::. /
          | VIRTIO               ------------
          |
        __V___ 
       | HARD |
       | DISK | 
        ------
  
Because HAWX runs in a qemu simulated machine, it uses simulated hardware.
QEMU uses the virtio virtualization for its disk standard. Detailed information
about this standard can be found at: 
https://docs.oasis-open.org/virtio/virtio/v1.1/virtio-v1.1.html

As you have no doubt noticed, this document is not the most user friendly
document. Reading it poses a real challenge. It's a great workout to the 
attention span!  In fact, reading this document without some sort of context
would be pretty much useless. They are really just a reference for the device's
registers and their meanings. To provide that context, we will turn to the xv6
source code, in particular this file:

  - `kernel/virtio_disk.c`

Reading over this code, and then looking up corresponding registers in the manual,
will make your life easier while doing this assignment!


Device Interrupts
=================
The disk is inherently slow. It takes a few milliseconds for 
this device to respond to a command. Because this is an eternity in CPU
time-scales, we don't want to wait for each device transaction to complete.
Instead, we want to do something that looks more like this:

  1. Send a command to the device.
  2. Move on to other work.
  3. Let the device interrupt us when it is done.

The device interrupts are handled by the kernel's trap handler, which we 
cover in another assignment. For now, we will just assume that the kernel
responds to these interrupts by calling the appropriate device driver
function. This is similar to the behavior of the UART driver.

Implementing the VIRTIO Block Device Driver
===========================================
The VIRTIO block device is a little more complicated than the UART device,
though the description here will be a little simpler. The reason for this
is that the disk is pretty much an "all or nothing" proposition. Instead,
let's focus on a narrative of what the driver should do, and then you
can set about grabbing and modifying the code to make it happen!

The key things to watch out for are the differences between our driver 
and the xv6 driver. xv6 uses a buffer-cache scheme which folds the 
file system into the kernel. We are not doing that. Instead, we are using
the `PORT_DISKCMD` port to send commands to the disk. The message format 
is as follows:

    +-+-------+----+----+
    |M|BLOCKID|DATA|MSG |
    +-+-------+----+----+
    M        - 1 Character R for read, W for write
    Block ID - 7 Characters Decimal Block ID to operate on
    Data     - 4 Characters Decimal Port to use as block buffer
    Message  - 4 Characters Decimal Port to write response message

This message is processed by the `get_disk_msg` function. It may be a good idea
to go ahead and implement this now.

The disk driver will respond to these messages as each command completes. The
response messages have the following format:

   +-+-+-------+
   |M|S|BLOCKID|
   +-+-+-------+
   M        - 1 Character, echoing the specified mode.
   S        - 1 Character, S for success, F for failure
   Block ID - 7 Characters Decimal Block ID to operate on

These messages are written by the `write_disk_response` function. Now, 
the question you should be asking yourself is "From where do we get the
information for the message?" The answer is from the `disk.info` array. The
`id` parameter is your index into this array. Go ahead and implement this 
function.

Now, let's go ahead and implement the rest of the driver. Here is the
order I suggest you explore these functions. 

1. `virtio_disk_init`
2. `virtio_disk_start`
3. `virtio_disk_intr` 

Be sure to read the code and comments very carefully so that you can get the
correct behavior out of the driver. Pay close attention to the differences
in this file versus what you will see in xv6. This will give you strong
hints about how to proceed!

When you get it working, you should see the following when you run the 
system:
```
$ make qemu
...

HAWX kernel is booting

UART initialization test...PASSED
UART flush test...PASSED
Interrupt driven output test...PASSED
Type the word "PASSED" and press enter: PASSED
Interrupt driven input test...PASSED
Writing to disk...PASSED
Reading from disk...PASSED
Empty port disk write...PASSED
Partial port disk write...PASSED
Non-empty port disk read...PASSED
Descriptor exhaustion test...PASSED
panic: All done! For now...
```

The first two disk tests are about the basic ability to successfully read 
and write to and from the disk. The others test details. For example, you
need to verify the ports you are using in read and write operations. You 
also need to make sure that you deallocate allocated descriptors for failed
operations. If the last test hangs, you are probably not deallocating
the descriptors properly.
