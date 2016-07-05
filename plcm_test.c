/*
 * Lanner Paralle LCM Driver Test Program
 */
#include <sys/file.h>
#include <stdio.h>
#include <string.h>
#include "plcm_ioctl.h"

int main(int argc, char *argv[])
{
	int devfd;
	unsigned char LCM_Message1[40] = "Lanner@Taiwan";
	unsigned char Ret_Message1[40] = "";
	unsigned char LCM_Message2[40] = "2013-11-05";
	unsigned char Ret_Message2[40] = "";
	unsigned char LCM_Message3[40] = "";
	int i = 0;
	unsigned char Keypad_Value = 0, Pre_Value = 0, Counter = 0;
	unsigned char Keypad_Message[40] = "";
	unsigned char Up[]    = "Up   ";
	unsigned char Down[]  = "Down ";
	unsigned char Left[]  = "Left ";
	unsigned char Right[] = "Right"; 
	unsigned char detect_press;
	unsigned char detect_dir;
	int MaxTestTime = 15 * 10; // 30 Sec

	printf("Lanner Parallel LCM Test Program\n");
	devfd = open("/dev/plcm_drv", O_RDWR);
	if(devfd == -1)
	{
		printf("Can't open /dev/plcm_drv\n");
		return -1;
	}
	printf("IOCTRL Testing...\n");
	if(argc == 2)
	{
		if(strcmp("-stop", argv[1]) == 0)
		{
			ioctl(devfd, PLCM_IOCTL_STOP_THREAD, 0);
			printf("plcm_drv thread has been stopped.\n");
			close(devfd);
			return 0;
		}
		if(strcmp("-On", argv[1]) == 0)
		{
			ioctl(devfd, PLCM_IOCTL_BACKLIGHT, 1);
			sleep(3);
			close(devfd);
			return 0;
		}
		if(strcmp("-Off", argv[1]) == 0)
		{
			ioctl(devfd, PLCM_IOCTL_BACKLIGHT, 0);
			sleep(3);
			close(devfd);
			return 0;
		}
		if(strcmp("-LCM1", argv[1]) == 0)
		{
			ioctl(devfd, PLCM_IOCTL_SET_LINE, 1);
			write(devfd, LCM_Message1, strlen(LCM_Message1));
			close(devfd);
			return 0;
		}
		if(strcmp("-LCM2", argv[1]) == 0)
		{
			ioctl(devfd, PLCM_IOCTL_SET_LINE, 2);
			write(devfd, LCM_Message2, strlen(LCM_Message2));
			close(devfd);
			return 0;
		}
		if(strcmp("-Keypad", argv[1]) == 0)
		{
			printf("Keypad Testing....\n");
			printf("  You only have 15 second to test it.\n");
			printf("  Or press Ctrl+C to exit.\n");
			ioctl(devfd, PLCM_IOCTL_SET_LINE, 1);
			do{
				Keypad_Value = ioctl(devfd, PLCM_IOCTL_GET_KEYPAD, 0);
				if(Pre_Value != Keypad_Value)
				{
					ioctl(devfd, PLCM_IOCTL_CLEARDISPLAY, 0);
					ioctl(devfd, PLCM_IOCTL_RETURNHOME, 0);
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
				write(devfd, Keypad_Message, strlen(Keypad_Message));
				Pre_Value = Keypad_Value;
				usleep(100000); // 100 msec
				MaxTestTime--;
				if(MaxTestTime == 0)
				{
					printf("Keypad Testing Finished.\n");
					break;
				}
			}while(1);
			close(devfd);
			return 0;
		}
		printf("Invalid command.\n");
		printf("plcm_test [-stop|-On|-Off|-LCM1|-LCM2|-Keypad]\n");
		close(devfd);
		return 0;	
	}

	printf("  Display Control - Fill Black Character\n");
	ioctl(devfd, PLCM_IOCTL_DISPLAY_D, 1);
	for(i=0;i<40;i++) LCM_Message3[i] = 0xff;
	ioctl(devfd, PLCM_IOCTL_SET_LINE, 1);
	write(devfd, LCM_Message3, 40);
	ioctl(devfd, PLCM_IOCTL_SET_LINE, 2);
	write(devfd, LCM_Message3, 40);
	sleep(2);
	printf("  Backlight Control - Backlight Off\n");
	ioctl(devfd, PLCM_IOCTL_BACKLIGHT, 0);
	sleep(2);
	printf("  Backlight Control - Backlight On\n");
	ioctl(devfd, PLCM_IOCTL_BACKLIGHT, 1);
	sleep(2);
	printf("  Display Control - Display Off\n"); 
	ioctl(devfd, PLCM_IOCTL_DISPLAY_D, 0);
	sleep(2);

	ioctl(devfd, PLCM_IOCTL_CLEARDISPLAY, 0);
	ioctl(devfd, PLCM_IOCTL_DISPLAY_D, 1);
	ioctl(devfd, PLCM_IOCTL_SHIFT_SC, 0);
	ioctl(devfd, PLCM_IOCTL_SET_LINE, 1);

	printf("  Display Control - Cursor Off\n"); 
	ioctl(devfd, PLCM_IOCTL_DISPLAY_B, 0);
	ioctl(devfd, PLCM_IOCTL_DISPLAY_C, 0);
	sprintf(Keypad_Message,"Cursor Off:");
	write(devfd, Keypad_Message, strlen(Keypad_Message));
	sleep(2);
	printf("  Display Control - Cursor On\n");
	ioctl(devfd, PLCM_IOCTL_DISPLAY_C, 1);
	sprintf(Keypad_Message,"Cursor On:");
	write(devfd, Keypad_Message, strlen(Keypad_Message));
	sleep(2);
	printf("  Display Control - Blinking On\n"); 
	ioctl(devfd, PLCM_IOCTL_DISPLAY_B, 1);
	sprintf(Keypad_Message,"Blinking On:");
	write(devfd, Keypad_Message, strlen(Keypad_Message));
	sleep(2);
	printf("  Display Control - Blinking Off\n"); 
	ioctl(devfd, PLCM_IOCTL_DISPLAY_B, 0);
	sprintf(Keypad_Message,"Blinking Off:");
	write(devfd, Keypad_Message, strlen(Keypad_Message));
	sleep(2);
	printf("Write and Read Testing...\n");
	ioctl(devfd, PLCM_IOCTL_SET_LINE, 1);
	write(devfd, LCM_Message1, strlen(LCM_Message1));
	printf("  Write: %s\n", LCM_Message1);
	read(devfd, Ret_Message1, 40);
	printf("  Read: %s\n", Ret_Message1);
	sleep(2);
	ioctl(devfd, PLCM_IOCTL_SET_LINE, 2);
	write(devfd, LCM_Message2, strlen(LCM_Message2));
	printf("  Write: %s\n", LCM_Message2);
	read(devfd, Ret_Message2, 40);
	printf("  Read: %s\n", Ret_Message2);
	sleep(2);

	/* In order to hide the cursor, so pan left one word */
	ioctl(devfd, PLCM_IOCTL_SHIFT_SC, 1);

	printf("CGRAM Test...\n");
	ioctl(devfd, PLCM_IOCTL_SET_LINE, 1);
	sprintf(Keypad_Message," CGRAM Testing:");
	write(devfd, Keypad_Message, strlen(Keypad_Message));
	ioctl(devfd, PLCM_IOCTL_SET_LINE, 2);
	for(i=0;i<40;i++) LCM_Message3[i] = i%8;
	write(devfd, LCM_Message3, sizeof(LCM_Message3));
	sleep(5);

	Counter=0;
	printf("Keypad Testing....\n");
	ioctl(devfd, PLCM_IOCTL_SET_LINE, 1);
	sprintf(Keypad_Message," Keypad Testing");
	write(devfd, Keypad_Message, strlen(Keypad_Message));
	ioctl(devfd, PLCM_IOCTL_SET_LINE, 2);
	strcpy(Keypad_Message,"  Press the 4 buttons");
	write(devfd, Keypad_Message, strlen(Keypad_Message));
	ioctl(devfd, PLCM_IOCTL_GET_KEYPAD, 0); //clear previous keypad status
	Pre_Value = ioctl(devfd, PLCM_IOCTL_GET_KEYPAD, 0);
	printf("  You only have 15 second to test it.\n");
	printf("  Or press Ctrl+C to exit.\n");
	ioctl(devfd, PLCM_IOCTL_SET_LINE, 1);
	do{
		Keypad_Value = ioctl(devfd, PLCM_IOCTL_GET_KEYPAD, 0);
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
				strcat(Keypad_Message,"Press");
				break;
			    case 0x00:
				strcat(Keypad_Message,"Release");
				break;
			}
			ioctl(devfd, PLCM_IOCTL_SET_LINE, 2);
			write(devfd, Keypad_Message, strlen(Keypad_Message));
			Pre_Value = Keypad_Value;
		}
		if((MaxTestTime%10) == 0){
			ioctl(devfd, PLCM_IOCTL_SET_LINE, 1);
			sprintf(Keypad_Message," Keypad Testing....%02d",
				MaxTestTime/10);
			write(devfd, Keypad_Message, strlen(Keypad_Message));
		}
		MaxTestTime--;
		if(MaxTestTime == 0 || Counter==0xff)
		{
			printf("Keypad Testing Finished.\n");
			ioctl(devfd, PLCM_IOCTL_SET_LINE, 1);
			sprintf(Keypad_Message," Keypad Test Finished");
			write(devfd, Keypad_Message, strlen(Keypad_Message));
			break;
		}
		usleep(100000); // 100 msec
	}while(1);
	close(devfd);
	return 0;
}

