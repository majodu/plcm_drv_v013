/*
 * Lanner Paralle LCM Driver Test Program - for shift cursor & update single text
 */
#include <sys/file.h>
#include <stdio.h>
#include <string.h>
#include "plcm_ioctl.h"

void printf_usage()
{
     printf("=====================\n");
	 printf("[1]insert line:\n");
     printf("[2]move cursor right:\n");
     printf("[3]move cursor left:\n");
     printf("[4]add a char:\n");
     printf("[5]clean display \n");
	 printf("[6]leave \n");
     printf("=====================\n");     
}

int main(int argc, char *argv[])
{
	int devfd;
	char lcm_char ; //for single text
	char temp;
	int input=0;
    int line_choose=1;
	char clean_buffer[20];
	int count=0;
	
	memset(clean_buffer,0x20,sizeof(clean_buffer));

	iopl(3);
	printf("Lanner Parallel LCM Test Program for cursor & char :\n");
	devfd = open("/dev/plcm_drv", O_RDWR);
	if(devfd == -1)
	{
		printf("Can't open /dev/plcm_drv\n");
		return -1;
	}
	/***************/
	printf("  Display Control - Display Off\n"); 
	ioctl(devfd, PLCM_IOCTL_DISPLAY_D, 0);
	sleep(2);
	printf("  Display Control - Display On\n"); 
	ioctl(devfd, PLCM_IOCTL_DISPLAY_D, 1);
	sleep(2);
	printf("  Display Control - Blinking off \n"); 
	ioctl(devfd, PLCM_IOCTL_DISPLAY_B, 0);
	sleep(2);
	printf("  Display Control - Cursor On\n");
	ioctl(devfd, PLCM_IOCTL_DISPLAY_C, 1);
	sleep(2);
	ioctl(devfd, PLCM_IOCTL_SET_LINE, 1);
	while(input !=6 )
    {
        printf_usage();
        printf("please input one mode : ");
        scanf("%d",&input);
		if(input ==1)
		{
			printf("[1] select line 1 , [2] select line 2 :");
			scanf("%d",&line_choose);
			if(line_choose==1 )
			{
				ioctl(devfd, PLCM_IOCTL_SET_LINE, 1);
			}
			else if(line_choose==2)
			{
				ioctl(devfd, PLCM_IOCTL_SET_LINE, 2);
			}
			else
			{
				printf("fail line_choose !!\n");
			}
		}
        else if(input == 2)
        {
            printf("cursor right\n");
			ioctl(devfd, PLCM_IOCTL_SHIFT_RL,1);
            
        }
        else if(input == 3)
        {
            printf("cursor left\n");
			ioctl(devfd, PLCM_IOCTL_SHIFT_RL,0);
        }
        else if(input == 4)
        {
            
			printf("input a char :");
			scanf("%c",&temp);
			scanf("%c",&lcm_char);
            if(line_choose==1 )
			{
				printf("line is %d\n",line_choose);
				//ioctl(devfd, PLCM_IOCTL_SET_LINE, 1);
				ioctl(devfd, PLCM_IOCTL_INPUT_CHAR,(unsigned long)lcm_char);
			}
			else if(line_choose==2)
			{
				printf("line is %d\n",line_choose);
				//ioctl(devfd, PLCM_IOCTL_SET_LINE, 2);
				ioctl(devfd, PLCM_IOCTL_INPUT_CHAR,(unsigned long)lcm_char);
			}
			else
			{
				printf("fail insert !! ");
			}
        }
		else if(input ==5)
		{
			//clean display
			printf("clear display : \n");
			ioctl(devfd,PLCM_IOCTL_SET_LINE,1);
			write(devfd, clean_buffer, sizeof(clean_buffer));
			ioctl(devfd,PLCM_IOCTL_SET_LINE,2);
			write(devfd, clean_buffer, sizeof(clean_buffer));
			ioctl(devfd, PLCM_IOCTL_RETURNHOME, 0);
		}
		else if(input ==6)
		{
			printf("leave testing : \n");
		}
        else
        {
            printf("fail entry mode\n");
        }
    }
	/***************/
	close(devfd);
	return 0;
}

