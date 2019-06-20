# PlayStation 2 input/output processor (IOP) modules

The [input/output processor (IOP)](https://en.wikipedia.org/wiki/PlayStation_2_technical_specifications#I/O_processor) is a [MIPS R3000A](https://en.wikipedia.org/wiki/R3000), or in later PlayStation 2 models a [PowerPC 405GP](https://en.wikipedia.org/wiki/PowerPC_400#PowerPC_405) emulating a MIPS R3000A. This processor provides a number of services, for example handling of USB OHCI interrupts. These are implemented as IOP modules that the Linux kernel loads as firmware. IOP modules are IRX objects based on the [executable and linkable format](https://en.wikipedia.org/wiki/Executable_and_Linkable_Format) (ELF).

## Building

A `mipsr5900el-unknown-linux-gnu` target GCC cross-compiler is recommended, with for example the command `make CROSS_COMPILE=mipsr5900el-unknown-linux-gnu-`.

## Modules

Currently two modules are implemented: [printk](module/printk.c) and [irqrelay](module/irqrelay.c).

## Tools

There are two main tools:

`iopmod-info` prints information specific to IRX type of ELF objects, for example module name, version, and imported and exported libraries:

```
% tool/iopmod-info module/printk.irx
iopmod id addr		0x2090
iopmod id name		printk
iopmod id version	0x0100
iopmod entry addr	0x0
iopmod unknown addr	0x22d0
iopmod text size	8332
iopmod data size	0
iopmod bss size		0
iopmod version		0x0100
iopmod name		printk
import library	0x0102	intrman
import  0	23	QueryIntrContext	intrman_in_irq
import library	0x0101	loadcore
import  0	 6	loadcore_register_library
import library	0x0101	loadcore
import  0	 7	loadcore_release_library
import library	0x0101	sifcmd
import  0	12	sifcmd_send_cmd	sifcmd_send_cmd
import library	0x0101	sifcmd
import  0	13	isceSifSendCmd	sifcmd_send_cmd_irq
import library	0x0101	sifman
import  0	 8	sceSifDmaStat	__sifman_dma_stat
export library	0x0100	printk
export  0	0x00b4	printk
```

`iopmod-info` can also list modules, libraries and functions including versions that are compiled into the tool. For example:

```
% tool/iopmod-info --list-modules --list-libraries
              dmacman 0x0101  dmacman 0x0101
    Interrupt_Manager 0x0102  intrman 0x0102
             loadcore 0x0101 loadcore 0x0101
               printk 0x0100   printk 0x0100
IOP_SIF_rpc_interface 0x0101   sifcmd 0x0101
      IOP_SIF_manager 0x0101   sifman 0x0101
                Stdio 0x0102    stdio 0x0102
 Multi_Thread_Manager 0x0101   thbase 0x0101
 Multi_Thread_Manager 0x0101  thevent 0x0101
 Multi_Thread_Manager 0x0101  thfpool 0x0101
 Multi_Thread_Manager 0x0101  thmsgbx 0x0101
 Multi_Thread_Manager 0x0101  thrdman 0x0102
 Multi_Thread_Manager 0x0101  thsemap 0x0101
 Multi_Thread_Manager 0x0101  thvpool 0x0101
```

The other tool is `iopmod-link` that is used to create IRX objects that are suitable for the IOP.
