/*
 * Lanner Parallel LCM driver for Kernel 2.4.x, 2.6.x and 3.1.x
 */

/* Standard in kernel modules */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include "plcm_ioctl.h"

#if defined(OLDKERNEL)
#define printk //printk issue in 2.4.22
#endif

#ifndef LINUX_VERSION_CODE
#include <linux/version.h>
#else
#define KERNEL_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))
#endif

/*
 * Driver Version Control
 */
static unsigned char Driver_Version[] = "0.1.2"; // 2014-08-20

/*
 * Device Major Number
 */
#define PLCM_MAJOR 248

/*
 * The DISPLAY_CAREFUL_MODE is just usable when the mode of parallel 
 * port is set to SPP(Standard Parallel Port) in BIOS
 */
//#define DISPLAY_CAREFUL_MODE

/*
 * Is the device opened right now?
 * Prevent to access the device in the same time
 */
static int Device_Open = 0;
static int stop_thread = 0;

/*
 * Device Depend Function Prototypes
 */
static void LCM_Init(void);
static void LCM_Command(unsigned char RS, unsigned char RWn, unsigned char CMD, unsigned int uDelay, unsigned char *Ret);
static void LCM_Backlight(void);

/*
 * Device Depend Definition
 */
#define LPT1 0x378
#define LPT2 0x278
#define LPT3 0x3BC

/*
 * Device Depend Variables
 */
static unsigned int  Port_Addr = 0; // LPTx Port Address
static unsigned int  DataPort = 0;
static unsigned int  StatusPort = 0;
static unsigned int  ControlPort = 0;
static unsigned char Backlight = 0; // Backlight ON
static unsigned char Cur_Line = 1; // Current Line#
static unsigned char Cur_EntryMode = 0x04; // Current Entry Mode Set CMD
static unsigned char Cur_Display = 0x08; // Current Display On/Off Ctrl
static unsigned char Cur_Shift = 0x10; // Current Cursor/Dsiplay Shift Ctrl

static unsigned int row = 0; // count row

static void LCM_Init(void)
{
	unsigned int i = 0;
	unsigned char ctl;

	ctl = inb(LPT1+2);
	outb( ctl&0xdf, LPT1+2 );
	outb(0x01, LPT1);
	if(inb(LPT1) == 0x01) {
		Port_Addr = LPT1;
	} else {
		ctl = inb(LPT2+2);
		outb( ctl&0xdf, LPT2+2 );
		outb(0x01, LPT2);
		if(inb(LPT2) == 0x01) {
			Port_Addr = LPT2;
		} else {
			ctl = inb(LPT3+2);
			outb( ctl&0xdf, LPT3+2 );
			outb(0x01, LPT3);
			if(inb(LPT3) == 0x01) {
				Port_Addr = LPT3;
			} else {
				Port_Addr = 0;
			}
		}
	}

	if(Port_Addr == 0)
	{
		printk("plcm_drv: Can not find any LPTx to use...\n");
	}
	else
	{
		printk("plcm_drv: LPTx Address = %x\n", Port_Addr);
		DataPort = Port_Addr;
		StatusPort = Port_Addr + 1;
		ControlPort = Port_Addr + 2;
		LCM_Command(0, 0, 0x38, 8000, NULL); // Function Set
		LCM_Command(0, 0, 0x38,  300, NULL);
		LCM_Command(0, 0, 0x38,  300, NULL);
		LCM_Command(0, 0, 0x38,  300, NULL);
		LCM_Command(0, 0, 0x0F,  300, NULL); Cur_Display=0x0F;// Display On/OFF
		LCM_Command(0, 0, 0x01, 3000, NULL); // Display Clear
		LCM_Command(0, 0, 0x06,  300, NULL); // Entry Mode Set
		LCM_Command(0, 0, 0x80,  300, NULL); // Set DDRAM Address	
		for(i = 0; i < 20; i++) // Range: 0x00~0x27
		{
			LCM_Command(1, 0, ' ', 300, NULL); // Write Data
			//count++;
		}
		LCM_Command(0, 0, 0xC0, 300, NULL); // Set DDRAM Address
		for(i = 0; i < 20; i++) // Range: 0x40~0x67
		{
			LCM_Command(1, 0, ' ', 300, NULL); // Write Data
			//count++;
		}
		// Add character in CGRAM as below.
		// 11111 
		// 10001
		// 10101
		// 10101
		// 10101
		// 10001
		// 11111
		// 00000
		for(i = 0; i < 8; i++)
		{
			LCM_Command(0, 0, 0x40+i*8+0, 300, NULL);
			LCM_Command(1, 0, 0x1F, 300, NULL);
			LCM_Command(0, 0, 0x40+i*8+1, 300, NULL);
			LCM_Command(1, 0, 0x11, 300, NULL);
			LCM_Command(0, 0, 0x40+i*8+2, 300, NULL);
			LCM_Command(1, 0, 0x15, 300, NULL);
			LCM_Command(0, 0, 0x40+i*8+3, 300, NULL);
			LCM_Command(1, 0, 0x15, 300, NULL);
			LCM_Command(0, 0, 0x40+i*8+4, 300, NULL);
			LCM_Command(1, 0, 0x15, 300, NULL);
			LCM_Command(0, 0, 0x40+i*8+5, 300, NULL);
			LCM_Command(1, 0, 0x11, 300, NULL);
			LCM_Command(0, 0, 0x40+i*8+6, 300, NULL);
			LCM_Command(1, 0, 0x1F, 300, NULL);
			LCM_Command(0, 0, 0x40+i*8+7, 300, NULL);
			LCM_Command(1, 0, 0x00, 300, NULL);
		}
	}
	return;
}

#define ENABLE 0x02
static void LCM_Command(unsigned char RS, unsigned char RWn, unsigned char CMD, unsigned int uDelay, unsigned char *Ret)
{
	unsigned char Ctrl = 0;

	Ctrl |= Backlight;
	if(RS == 0)
	{
		Ctrl |= 0x08; // RS: Real RS = ~RS
	}
	if(RWn == 1)
	{
		Ctrl |= 0x24; // RWn: Read = 1, Write = 0
	}else{
		outb(CMD, DataPort); // LCM Data Write
	}
	outb(Ctrl | ENABLE, ControlPort); // Set RS and RWn, E = 0
	udelay(uDelay);
	outb(Ctrl & ~ENABLE, ControlPort); // E = 1 
	udelay(10);
	if((RWn == 1) && (Ret != NULL))
	{
		*Ret = inb(DataPort); // LCM Data Read
	}
	/* For IT8xxx support-io, set CR[5] to 1 is requests for keypad function */
	outb(Ctrl | 0x20 | ENABLE, ControlPort); // E = 0
	udelay(uDelay + 1);
	return;
}

#ifdef DISPLAY_CAREFUL_MODE
static int check_busy(unsigned char dd_addr)
{
	unsigned char Ctrl = 0;
	int busy =0, cnt = 0;
	do {
		LCM_Command(0,1, 0, 46, &Ctrl);
		if(Ctrl & 0x80){
			if(!busy) printk("PLCM CR: 0x%x\n", Ctrl);
			if(cnt > 100){
				printk("[%s:%d] PLCM is Busy\n",
					__FUNCTION__,__LINE__);
				break;
			}
			busy = 1;
		}else if((dd_addr & 0x80) && (Ctrl != (dd_addr&0x7f))){
			printk("PLCM CR: RAM_Address is Incrroct\n");
			busy = 2;
			break;
		}
		cnt++;
		udelay(100);
	}while(Ctrl & 0x80);

	return busy;
}
#endif

static void LCM_Backlight(void)
{
	unsigned char Ctrl = inb(ControlPort);

	if(Backlight == 1)
	{
		Ctrl |= 0x01;
	}
	else
	{
		Ctrl &= ~0x01;
	}
	outb(Ctrl, ControlPort);
	return;
}

#if 0
static int plcm_thread(void *s)
{
	unsigned int cur_val = 0;
	while(1)
	{
		set_current_state(TASK_INTERRUPTIBLE);
		schedule_timeout(10);
		switch(cur_val)
		{
			default:
				break;
		}
		if(stop_thread == 1)
		{
			printk("plcm_drv thread stopped\n");
			return 0;
		}
	}
	return 0;
}
#endif 

#if defined(OLDKERNEL)
static ssize_t plcm_read(struct file *file, char * buffer, size_t length, loff_t * offset)
#else
static ssize_t plcm_read(struct file *file, char __user * buffer, size_t length, loff_t * offset)
#endif
{
	unsigned char dd_addr=0x80, Data;
	int i = 0; 
	if(length != 40)
	{
		return 0;
	}
	if(Cur_Line == 1){
		dd_addr = 0x80;
	}else if(Cur_Line == 2){
		dd_addr = 0xC0;
	}
#ifdef DISPLAY_CAREFUL_MODE
        int err_cnt;
	for(i = 0; i < 40; i++)
	{
		err_cnt = 0;
		while(1){
			if( err_cnt > 10){
				return -ECOMM;
			}
			err_cnt++;
			/* Verify Data */
			LCM_Command(0, 0, dd_addr + i, 46, NULL);
			if(check_busy(dd_addr + i)) continue;
			LCM_Command(1, 1, 0, 46, &Data);
			break;
		}
		put_user(Data, buffer + i); // Copy Data
	}
#else
	LCM_Command(0, 0, dd_addr, 300, NULL);
	for(i = 0; i < 40; i++)
	{
		LCM_Command(1, 1, 0x00, 46, &Data); // Read Data
		put_user(Data, buffer + i); // Copy Data 
	}
#endif
	Data = 0;
	put_user(Data, buffer + i); // Copy Data
	//printk("plcm_drv: Read operation\n");
	return length;
}

#if defined(OLDKERNEL)
static ssize_t plcm_write(struct file *file, const char * buffer, size_t length, loff_t * offset)
#else
static ssize_t plcm_write(struct file *file, const char __user * buffer, size_t length, loff_t * offset)
#endif
{
	unsigned char LCM_Message[40], dd_addr=0x80;
    //    unsigned char Data;
	int i = 0;
    //    int err_cnt;

	if(length > 40)
	{
		printk("[%s] invalid string length\n",__FUNCTION__);
		return 0;
	}

	//printk("plcm_drv: Write %s\n", buffer);
	for(i = 0; i < 40; i++)
	{
		if(i < length)
			get_user(LCM_Message[i], buffer + i);
		else
			LCM_Message[i] = ' ';
	}
	if(Cur_Line == 1){
		dd_addr = 0x80;
	}else if(Cur_Line == 2){
		dd_addr = 0xC0;
	}
#ifdef DISPLAY_CAREFUL_MODE
	/* Careful mode; Confirm each character was printed correctly */
	for(i = 0; i < 40; i++)
	{
		err_cnt = 0;
		while(1){
			if( err_cnt > 10){
				return -ECOMM;
			}
			err_cnt++;
			/* Write Data */
			LCM_Command(0, 0, dd_addr + i, 46, NULL);
			if(check_busy(dd_addr + i)) continue;
			LCM_Command(1, 0, LCM_Message[i], 46, NULL);

			/* Verify Data */
			LCM_Command(0, 0, dd_addr + i, 46, NULL);
			if(check_busy(dd_addr + i)) continue;
			LCM_Command(1, 1, 0, 46, &Data);
			if(Data == LCM_Message[i]) break;
			printk("PLCM DR: RAM_Data is Incrroct\n");
		}
	}
#else
	/* Fast mode; print directly without confirm */
	LCM_Command(0, 0, dd_addr, 300, NULL);
	for(i = 0; i < 40; i++)
	{
		LCM_Command(1, 0, LCM_Message[i], 46, NULL);
	}
#endif

	return 40;
}

#if ( LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36) )
static int plcm_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
#else
static long plcm_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
#endif
{
	switch(cmd)
	{
		case PLCM_IOCTL_STOP_THREAD:
			printk("sled_drv : PLCM_IOCTL_STOP_THREAD\n");
			stop_thread = 1;
			break;
		case PLCM_IOCTL_BACKLIGHT:
			if(arg == 0)
				Backlight = 1;
			else if (arg == 1)
				Backlight = 0;
			LCM_Backlight();
			break;
		case PLCM_IOCTL_SET_LINE:
			if(arg == 1)
			{
				Cur_Line = 1;
				LCM_Command(0, 0, 0x80+row, 300, NULL);
			}
			else if(arg == 2)
			{
				Cur_Line = 2;
				LCM_Command(0, 0, 0xC0+row, 300, NULL);
			}
			break;
		case PLCM_IOCTL_CLEARDISPLAY:
			LCM_Command(0, 0, 0x01, 1640, NULL);
			row = 0;
			break;
		case PLCM_IOCTL_RETURNHOME:
			LCM_Command(0, 0, 0x02, 1640, NULL);
			break;
		case PLCM_IOCTL_ENTRYMODE_ID:
			if(arg == 0)
				Cur_EntryMode &= ~0x02;
			else if(arg == 1)
				Cur_EntryMode |= 0x02;
			LCM_Command(0, 0, Cur_EntryMode, 300, NULL);
			break;
		case PLCM_IOCTL_ENTRYMODE_SH:
			if(arg == 0)
				Cur_EntryMode &= ~0x01;
			else if(arg == 1)
				Cur_EntryMode |= 0x01;
			LCM_Command(0, 0, Cur_EntryMode, 300, NULL);
			break;
		case PLCM_IOCTL_DISPLAY_D:
			if(arg == 0)
				Cur_Display &= ~0x04;
			else if(arg == 1)
				Cur_Display |= 0x04;
			LCM_Command(0, 0, Cur_Display, 300, NULL);
			break;
		case PLCM_IOCTL_DISPLAY_C:
			if(arg == 0)
				Cur_Display &= ~0x02;
			else if(arg == 1)
				Cur_Display |= 0x02;
			LCM_Command(0, 0, Cur_Display, 300, NULL);
			break;
		case PLCM_IOCTL_DISPLAY_B:
			if(arg == 0)
				Cur_Display &= ~0x01;
			else if(arg == 1)
				Cur_Display |= 0x01;
			LCM_Command(0, 0, Cur_Display, 300, NULL);
			break; 
		case PLCM_IOCTL_SHIFT_SC:
			if(arg == 0)
				Cur_Shift &= ~0x08;
			else if(arg == 1)
				Cur_Shift |= 0x08;
			LCM_Command(0, 0, Cur_Shift, 300, NULL);
			break;
		case PLCM_IOCTL_SHIFT_RL:
			if(arg == 0)
			{
				Cur_Shift &= ~0x04;
				if(row > 0 && row < 20)
				{
					LCM_Command(0, 0, Cur_Shift, 300, NULL);
					row--;
				}
			}else if(arg == 1){
				Cur_Shift |= 0x04;
				if(row >= 0 && row < 19)
				{
					LCM_Command(0, 0, Cur_Shift, 300, NULL);
					row++;
				}
			}
			break;
		case PLCM_IOCTL_GET_KEYPAD:
			return inb(StatusPort);
			break;
		case PLCM_IOCTL_INPUT_CHAR:
			/*if(Cur_Line == 1)
			{
				LCM_Command(0, 0, 0x80+row, 300, NULL);
			}
			else if(Cur_Line == 2)
			{
				LCM_Command(0, 0, 0xC0+row, 300, NULL);
			}*/
			LCM_Command(1, 0, (char)arg,  300, NULL);
			row ++;
			break;
		default:
			return -EOPNOTSUPP;
	}
	return 0;
}

/*
 * This function is called whenever a process attempts to
 * open the device file
 */
static int plcm_open(struct inode * inode, struct file * file)
{
	/*
	 * Get the minor device number in case you have more than
	 * one physical device using the driver.
	 */
	printk("Device: %d.%d\n", inode->i_rdev>>8, inode->i_rdev & 0xff);
	/* we don't want to talk to two processes at the same time */
	if(Device_Open) return -EBUSY;
	Device_Open++;
	/* Make sure that the module isn't removed while the file
	 * is open by incrementing the usage count (the number of
	 * opened references to the module,if it's zero emmod will
	 * fail)
	 */
	printk("Lanner Parallel LCM Driver Opened\n");
	return 0;
}

/*
 * This function is called when a process closes the device file.
 */
static int plcm_release(struct inode * inode, struct file * file)
{
	/* ready for next caller */
	Device_Open--;
	/* Decrement the usage count, otherwise once you opened the file
	 * you'll never get rid of the module.
	 */
	printk(KERN_NOTICE "Lanner Parallel LCM Driver Closed\n");
	return 0;
}

/*
 * This structure will hold the functions to be called
 * when a process does something to the device we created.
 * Since a pointer to this structure is kept in the
 * devices table, it can't be local to init_module.
 * NULL is for unimplemented functions.
 */
#if defined(OLDKERNEL)
static struct file_operations plcm_fops ={
	owner:		THIS_MODULE,
	llseek:		NULL,
	read:		plcm_read,
	write:		plcm_write,
	ioctl:		plcm_ioctl,
	open:		plcm_open,
	release:	plcm_release,
};
#else
static const struct file_operations plcm_fops = {
	.read		= plcm_read,
	.write		= plcm_write,
#if ( LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36) )
	.ioctl		= plcm_ioctl,
#else
	.unlocked_ioctl		= plcm_ioctl,
#endif
	.open		= plcm_open,
	.release	= plcm_release,
};
#endif

int plcm_init(void)
{
	/*
	 * Register the character device
	 */
	if(register_chrdev(PLCM_MAJOR, "plcm_drv", &plcm_fops))
	{
		printk("plcm : unable to get major %d\n", PLCM_MAJOR);
		return -EIO;
	}
	printk("Parallel LCM Driver Version %s is loaded\n", Driver_Version);
	LCM_Init();
	if(DataPort == 0)
	{
		printk("plcm_drv: unable to access any LPTx\n");
		unregister_chrdev(PLCM_MAJOR, "plcm_drv");
		return -EIO;
	}
#if 0
	kernel_thread(plcm_thread, (void *)"Parallel LCM Thread", 0);
#endif
	return 0;
}

/*
 * Cleanup - unregister the appropriate file from /proc
 */
void plcm_exit(void)
{
	/* Unregister the device */
	unregister_chrdev(PLCM_MAJOR, "plcm_drv");
	/* If there's an error, report it */
	printk("Parallel LCM Driver Version %s is unloaded\n", Driver_Version);
}

module_init(plcm_init);
module_exit(plcm_exit);

MODULE_AUTHOR("Lannerinc");
MODULE_DESCRIPTION("Lanner Parallel LCM Driver");
MODULE_LICENSE("Dual BSD/GPL");

