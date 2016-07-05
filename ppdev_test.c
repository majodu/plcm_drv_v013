/*
 * Lanner Paralle LCM Driver Test Program
 */
#include <sys/file.h>
#include <stdio.h>
#include <string.h>
#include <linux/ioctl.h>
#include <linux/ppdev.h>

#define DataPort 0x378
#define StatusPort 0x379
#define ControlPort 0x37A
#define ENABLE 0x02

/*
 *  * Device Depend Variables
 *   */
unsigned char Backlight = 0; // Backlight ON
unsigned char Cur_Line = 1; // Current Line#
unsigned char Cur_EntryMode = 0x04; // Current Entry Mode Set CMD
unsigned char Cur_Display = 0x08; // Current Display On/Off Ctrl
unsigned char Cur_Shift = 0x10; // Current Cursor/Dsiplay Shift Ctrl

/* global data */
int devfd;
//int Backlight=0;

void ppdev_outb(unsigned char data, unsigned long port)
{
	unsigned long args;
	int ret;
	
	args=(unsigned long)data;

	switch (port) {
		case ControlPort:
			ret=ioctl(devfd, PPWCONTROL, &args);
			break;
				
		case DataPort:
			ret=ioctl(devfd, PPWDATA, &args);
			break;

		default:
			printf("Un-support port\n");
			break;
	}
	
	return;
}

unsigned char ppdev_inb(unsigned long port)
{
	unsigned long args;
	int ret;
	
	args=0;
	switch (port) {
		case DataPort:
			ret=ioctl(devfd, PPRDATA, &args);
			break;

		case StatusPort:
			ret=ioctl(devfd, PPRSTATUS, &args);
			break;

		case ControlPort:
			ret=ioctl(devfd, PPRCONTROL, &args);
			break;
				
		default:
			printf("Un-support port\n");
			break;
	}
	ret=ioctl(devfd, PPRDATA, &args);
	
	return (unsigned char)args;
}

void LCM_Command(unsigned char RS, unsigned char RWn, unsigned char CMD, unsigned int uDelay, unsigned char *Ret)
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
                ppdev_outb(CMD, DataPort); // LCM Data Write
        }
        ppdev_outb(Ctrl | ENABLE, ControlPort); // Set RS and RWn, E = 0
        usleep(uDelay);
        ppdev_outb(Ctrl & ~ENABLE, ControlPort); // E = 1
        usleep(10);
        if((RWn == 1) && (Ret != NULL))
        {
                *Ret = ppdev_inb(DataPort); // LCM Data Read
        }
        /* For IT8xxx support-io, set CR[5] to 1 is requests for keypad function */
    //    ppdev_outb(Ctrl | 0x20 | ENABLE, ControlPort); // E = 0
        ppdev_outb(Ctrl | ENABLE, ControlPort); // E = 0
        usleep(uDelay + 1);
        return;
}

static void LCM_Backlight(void)
{
	unsigned long Ctrl = 0;
	int ret;

        if(Backlight == 1)
        {
                Ctrl |= 0x01;
        }
        else
        {
                Ctrl &= ~0x01;
        }
	ret=ioctl(devfd, PPWCONTROL, &Ctrl);
        return;
}


int main(int argc, char *argv[])
{
	unsigned char LCM_Message1[40] = "Lanner@Taiwan                          ";
	unsigned char Ret_Message1[40] = "";
	unsigned char LCM_Message2[40] = "2013-11-05                             ";
	unsigned char Ret_Message2[40] = "";
	unsigned char LCM_Message3[40] = "";
	int i = 0;
	unsigned char Keypad_Value = 0, Pre_Value = 0, Counter = 0;
	unsigned char Keypad_Message[40] = "";
	unsigned char Keypad_Message1[40] = "";
	unsigned char Up[]    = "Up   ";
	unsigned char Down[]  = "Down ";
	unsigned char Left[]  = "Left ";
	unsigned char Right[] = "Right"; 
	unsigned char detect_press;
	unsigned char detect_dir;
	int MaxTestTime = 15 * 10; // 30 Sec
	int ret=0;
	unsigned long args;
	unsigned char data, status, control, dd_addr;

	printf("Lanner LCM Test Program by ppdev \n");
	devfd = open("/dev/parport0", O_RDWR);
	if(devfd == -1)
	{
		printf("Can't open /dev/parport0\n");
		return -1;
	}
	/* 1. claim parport device first */
	ret=ioctl(devfd, PPCLAIM, 0);
	if  ( ret ) 
	{
		printf("Fail to claim device\n");
		goto out;
	}

	/* 2. get data/status/control register */
	args = 0;
	ret=ioctl(devfd, PPRDATA, &args);
	printf("DATA=%x\n", (unsigned char)args);
	args = 0;
	ret=ioctl(devfd, PPRSTATUS, &args);
	printf("STATUS=%x\n", (unsigned char)args);
	args = 0;
	ret=ioctl(devfd, PPRCONTROL, &args);
	printf("CONTROL=%x\n", (unsigned char)args);
	
	/* 3. send initial command */
        LCM_Command(0, 0, 0x38, 8000, NULL); // Function Set
        LCM_Command(0, 0, 0x38,  300, NULL); // Function Set
        LCM_Command(0, 0, 0x38,  300, NULL); // Function Set
        LCM_Command(0, 0, 0x38,  300, NULL); // Function Set
        LCM_Command(0, 0, 0x0F,  300, NULL); // Display On/OFF
        LCM_Command(0, 0, 0x01, 3000, NULL); // Display Clear

	/* 4. print first line */
	dd_addr = 0x80;
        LCM_Command(0, 0, dd_addr, 300, NULL);
        for(i = 0; i < 40; i++)
        {
                LCM_Command(1, 0, ' ', 46, NULL);
        }
	/* 5. print second line */
	dd_addr = 0xc0;
        LCM_Command(0, 0, dd_addr, 300, NULL);
        
        for(i = 0; i < 40; i++)
        {
                LCM_Command(1, 0, ' ', 46, NULL);
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

        printf("IOCTRL Testing...\n");
        if(argc == 2)
        {
                if(strcmp("-stop", argv[1]) == 0)
                {
                        printf("plcm_drv thread has been stopped.\n");
                        printf("sled_drv : PLCM_IOCTL_STOP_THREAD\n");
		        goto out;
                }
                if(strcmp("-On", argv[1]) == 0)
                {
                        Backlight = 0;  
                        LCM_Backlight();
                        sleep(3);
		        goto out;
                }
                if(strcmp("-Off", argv[1]) == 0)
                {
                        Backlight = 1;  
                        LCM_Backlight();
                        sleep(3);
		        goto out;
                }
                if(strcmp("-LCM1", argv[1]) == 0)
                {
                        LCM_Command(0, 0, 0x80, 300, NULL);
                        for(i = 0; i < 40; i++)
                        {
                             LCM_Command(1, 0, LCM_Message1[i], 46, NULL);
                        }
		        goto out;
                }
                if(strcmp("-LCM2", argv[1]) == 0)
                {
                        LCM_Command(0, 0, 0xC0, 300, NULL);
                        for(i = 0; i < 40; i++)
                        {
                             LCM_Command(1, 0, LCM_Message2[i], 46, NULL);
                        }
		        goto out;
                }
                if(strcmp("-Keypad", argv[1]) == 0)
                {
                        printf("Keypad Testing....\n");
                        printf("  You only have 15 second to test it.\n");
                        printf("  Or press Ctrl+C to exit.\n");
                        LCM_Command(0, 0, 0x0f, 300, NULL);
                        LCM_Command(0, 0, 0x80, 300, NULL);
                        do{
                                args=0;
                                ret=ioctl(devfd, PPRSTATUS, &args);
                                Keypad_Value=args;
                                if(Pre_Value != Keypad_Value)
                                {
                                   LCM_Command(0, 0, 0x01, 1640, NULL);
                                   LCM_Command(0, 0, 0x02, 1640, NULL);
                                }     
                                detect_press=(Keypad_Value & 0x40);
                                detect_dir=(Keypad_Value & 0x28);
                                switch(detect_dir)
                                {
                                        case 0x00:
                                                strcpy(Keypad_Message,"Up-");
                                                break;
                                        case 0x20:
                                                strcpy(Keypad_Message,"Left-");
                                                break;
                                        case 0x08:
                                                strcpy(Keypad_Message,"Right-");
                                                break;
                                        case 0x28:
                                                strcpy(Keypad_Message,"Down-");
                                                break;
                                }
                                switch(detect_press)
                                {
                                        case 0x40:
                                                strcat(Keypad_Message,"Press     ");
                                                break;
                                        case 0x00:
                                                strcat(Keypad_Message,"Release     ");
                                                break;
                                }
                                   LCM_Command(0, 0, 0x01, 1640, NULL);
                                   LCM_Command(0, 0, 0x02, 1640, NULL);
                                 for(i = 0; i < 40; i++)
                                 {   
                                     if((40 - i) > 0)
                                     {
                                        if(Keypad_Message[i] == 0)
                                        {
                                            Keypad_Message[i] = ' ';
                                        } 
                                      } 
                                   LCM_Command(1, 0, Keypad_Message[i], 46, NULL);
                                }
                                Pre_Value = Keypad_Value;
                                usleep(100000); // 100 msec
                                MaxTestTime--;
                                if(MaxTestTime == 0)
                                {
                                        printf("Keypad Testing Finished.\n");
                                        break;
                                }
                        }while(1);
		        goto out;
                }
                printf("Invalid command.\n");
                printf("plcm_test [-stop|-On|-Off|-LCM1|-LCM2|-Keypad]\n");
		goto out;
        }
        printf("  Display Control - Fill Black Character\n");
        Cur_Display |= 0x04;
        LCM_Command(0, 0, Cur_Display, 300, NULL);                   
        for(i=0;i<40;i++) LCM_Message3[i] = 0xff;
        LCM_Command(0, 0, 0x80, 300, NULL);                   
        for(i = 0; i < 40; i++)                                 //write LCM_Message3
        {
            LCM_Command(1, 0, LCM_Message3[i], 46, NULL);
        }
        LCM_Command(0, 0, 0xC0, 300, NULL);                     //write LCM_Message3
        for(i = 0; i < 40; i++)
        {
            LCM_Command(1, 0, LCM_Message3[i], 46, NULL);
        }
        sleep(2);
        printf("  Backlight Control - Backlight Off\n");
        Backlight = 1;
        LCM_Backlight();
        sleep(2);
        printf("  Backlight Control - Backlight On\n");
        Backlight = 0;
        LCM_Backlight();
        sleep(2);
        printf("  Display Control - Display Off\n");
        Cur_Display &= ~0x04;
        LCM_Command(0, 0, Cur_Display, 300, NULL);
        sleep(2);
        LCM_Command(0, 0, 0x01, 1640, NULL);                   
        Cur_Display |= 0x04;
        LCM_Command(0, 0, Cur_Display, 300, NULL);
        Cur_Shift &= ~0x08;
        LCM_Command(0, 0, Cur_Shift, 300, NULL);
        LCM_Command(0, 0, 0x80, 300, NULL);                   
        printf("  Display Control - Cursor Off\n");
        Cur_Display &= ~0x01;
        LCM_Command(0, 0, Cur_Display, 300, NULL);
        Cur_Display &= ~0x02;
        LCM_Command(0, 0, Cur_Display, 300, NULL);
        sprintf(Keypad_Message,"Cursor Off:");
        for(i = 0; i < 40; i++)                                 //write Keypad_Message
        {
            if((40 - i) > 0)
            {
               if(Keypad_Message[i] == 0)
                {
                   Keypad_Message[i] = ' ';
                }
             }
            LCM_Command(1, 0, Keypad_Message[i], 46, NULL);
        }
        sleep(2);
        printf("  Display Control - Cursor On\n");
        Cur_Display |= 0x02;
        LCM_Command(0, 0, Cur_Display, 300, NULL);
        sprintf(Keypad_Message,"Cursor On:");
        LCM_Command(0, 0, 0x01, 1640, NULL);
        LCM_Command(0, 0, 0x02, 1640, NULL);
        for(i = 0; i < 40; i++)                                 //write Keypad_Message
        {
            if((40 - i) > 0)
            {
               if(Keypad_Message[i] == 0)
                {
                   Keypad_Message[i] = ' ';
                }
             }
            LCM_Command(1, 0, Keypad_Message[i], 46, NULL);
        }
        sleep(2);
        printf("  Display Control - Blinking On\n");
        Cur_Display |= 0x01;
        LCM_Command(0, 0, Cur_Display, 300, NULL);
        sprintf(Keypad_Message,"Blinking On:");
        LCM_Command(0, 0, 0x01, 1640, NULL);
        LCM_Command(0, 0, 0x02, 1640, NULL);
        for(i = 0; i < 40; i++)                                 //write Keypad_Message
        {
            if((40 - i) > 0)
            {
               if(Keypad_Message[i] == 0)
                {
                   Keypad_Message[i] = ' ';
                }
             }
            LCM_Command(1, 0, Keypad_Message[i], 46, NULL);
        }
        sleep(2);
        printf("  Display Control - Blinking Off\n");
        Cur_Display &= ~0x01;
        LCM_Command(0, 0, Cur_Display, 300, NULL);
        sprintf(Keypad_Message,"Blinking Off:");
        LCM_Command(0, 0, 0x01, 1640, NULL);
        LCM_Command(0, 0, 0x02, 1640, NULL);
        for(i = 0; i < 40; i++)                                 //write Keypad_Message
        {
            if((40 - i) > 0)
            {
               if(Keypad_Message[i] == 0)
                {
                   Keypad_Message[i] = ' ';
                }
             }
            LCM_Command(1, 0, Keypad_Message[i], 46, NULL);
        }
        sleep(2);
        printf("Write and Read Testing...\n");
        LCM_Command(0, 0, 0x80, 300, NULL);
        for(i = 0; i < 40; i++)                                 //write Keypad_Message
        {
            LCM_Command(1, 0, LCM_Message1[i], 46, NULL);
            args = 0;
            ret=ioctl(devfd, PPRDATA, &args);
            Ret_Message1[i]=args;
        }
        printf("  Write: %s\n", LCM_Message1);
        printf("  Read: %s\n", Ret_Message1);
        sleep(2);
        LCM_Command(0, 0, 0xC0, 300, NULL);
        for(i = 0; i < 40; i++)                                 //write Keypad_Message
        {
            LCM_Command(1, 0, LCM_Message2[i], 46, NULL);
            args = 0;
            ret=ioctl(devfd, PPRDATA, &args);
            Ret_Message2[i]=args;
        }
        printf("  Write: %s\n", LCM_Message2);
        printf("  Read: %s\n", Ret_Message2);
        sleep(2);
        // In order to hide the cursor, so pan left one word //
        Cur_Shift |= 0x08;
        LCM_Command(0, 0, Cur_Shift, 300, NULL);
       
        printf("CGRAM Test...\n");
        LCM_Command(0, 0, 0x80, 300, NULL);
        sprintf(Keypad_Message," CGRAM Testing:         ");
        for(i = 0; i < 40; i++)                                 //write LCM_Message3
        {
            LCM_Command(1, 0, Keypad_Message[i], 46, NULL);
        }
        LCM_Command(0, 0, 0xC0, 300, NULL);
        for(i=0;i<40;i++) LCM_Message3[i] = i%8;
        for(i = 0; i < 40; i++)                                 //write LCM_Message3
        {
            LCM_Command(1, 0, LCM_Message3[i], 46, NULL);
        }
        sleep(5);
        Counter=0;
        printf("Keypad Testing....\n");
        LCM_Command(0, 0, 0x80, 300, NULL);
        sprintf(Keypad_Message," Keypad Testing      ");
        for(i = 0; i < 40; i++)                                 //write Keypad_Message
        {
            LCM_Command(1, 0, Keypad_Message[i], 46, NULL);
        }
        LCM_Command(0, 0, 0xc0, 300, NULL);
        strcpy(Keypad_Message,"  Press the 4 buttons");
        for(i = 0; i < 40; i++)                                 //write Keypad_Message
        {
            LCM_Command(1, 0, Keypad_Message[i], 46, NULL);
        }
        args=0;
        ret=ioctl(devfd, PPRSTATUS, &args);
        Pre_Value=args;

        printf("  You only have 15 second to test it.\n");
        printf("  Or press Ctrl+C to exit.\n");
        LCM_Command(0, 0, 0x80, 300, NULL);
        do{
                  args=0;
                  ret=ioctl(devfd, PPRSTATUS, &args);
                  Keypad_Value=args;     
                if(Pre_Value != Keypad_Value)
                {
                        detect_press=(Keypad_Value & 0x40);
                        detect_dir=(Keypad_Value & 0x28);
                        switch(detect_dir){
                            case 0x00:
                                strcpy(Keypad_Message,"  Up    ");
                                Counter |= 0x10 | (detect_press>>6);
                                break;
                            case 0x20:
                                strcpy(Keypad_Message,"  Left  ");
                                Counter |= 0x20 | (detect_press>>5);
                                break;
                            case 0x08:
                                strcpy(Keypad_Message,"  Right ");
                                Counter |= 0x40 | (detect_press>>4);
                                break;
                            case 0x28:
                                strcpy(Keypad_Message,"  Down  ");
                                Counter |= 0x80 | (detect_press>>3);
                                break;
                        }
                        switch(detect_press){
                            case 0x40:
                                strcat(Keypad_Message,"Press         ");
                                break;
                            case 0x00:
                                strcat(Keypad_Message,"Release       ");
                                break;
                        }
                        LCM_Command(0, 0, 0xc0, 300, NULL);
                        for(i = 0; i < 40; i++)                                 //write Keypad_Message
                        {
                             if((40 - i) > 0)
                             {
                                if(Keypad_Message[i] == 0)
                                  {
                                     Keypad_Message[i] = ' ';
                                   }
                              }
                             LCM_Command(1, 0, Keypad_Message[i], 46, NULL);
                        }
                        Pre_Value = Keypad_Value;
                }
                if((MaxTestTime%10) == 0){
                        LCM_Command(0, 0, 0x80, 300, NULL);
                        sprintf(Keypad_Message," Keypad Testing....%02d",
                                MaxTestTime/10);
                        for(i = 0; i < 40; i++)                                 //write Keypad_Message
                        {
                            if((40 - i) > 0)
                             {
                                if(Keypad_Message[i] == 0)
                                  {
                                     Keypad_Message[i] = ' ';
                                   }
                              }
                             LCM_Command(1, 0, Keypad_Message[i], 46, NULL);
                        }
                }
                MaxTestTime--;
                if(MaxTestTime == 0 || Counter==0xff)
                {
                        printf("Keypad Testing Finished.\n");
                        LCM_Command(0, 0, 0x80, 300, NULL);
                        sprintf(Keypad_Message," Keypad Test Finished");
                        for(i = 0; i < 40; i++)                                 //write Keypad_Message
                        {
                             LCM_Command(1, 0, Keypad_Message[i], 46, NULL);
                        }
                        break;
                }
                usleep(100000); // 100 msec
        }while(1);

out:
	ret=ioctl(devfd, PPRELEASE, 0);
	
	close(devfd);
	return ret;
}

