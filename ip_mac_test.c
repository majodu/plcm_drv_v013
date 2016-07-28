#include <sys/file.h>
#include <stdio.h>
#include <string.h>
#include "plcm_ioctl.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
	int devfd;
    int fd;
    struct ifreq ifr;
    char *iface = "eth0";
    unsigned char *mac;
	unsigned char Keypad_Value = 0;
	unsigned char Keypad_Message[40] = "";
	unsigned char detect_dir;
    
    
    devfd = open("/dev/plcm_drv", O_RDWR);

	if(devfd == -1)
	{
		printf("Can't open /dev/plcm_drv\n");
		return -1;
	}
    ioctl(devfd, PLCM_IOCTL_SET_LINE, 1);
    strcpy(Keypad_Message,"b1: MAC b2: IP ");
    write(devfd, Keypad_Message, strlen(Keypad_Message));
  
 do{
    
    Keypad_Value = ioctl(devfd, PLCM_IOCTL_GET_KEYPAD, 0);
    detect_dir=(Keypad_Value & 0x28);
    switch(detect_dir){
	    case 0x08:
		    //07 43 third
		    
			break;
		case 0x20:
			//31 67 first

            
     
            fd = socket(AF_INET, SOCK_DGRAM, 0);
 
            ifr.ifr_addr.sa_family = AF_INET;
            strncpy(ifr.ifr_name , iface , IFNAMSIZ-1);
 
            ioctl(fd, SIOCGIFHWADDR, &ifr);
 
            close(fd);
     
            mac = (unsigned char *)ifr.ifr_hwaddr.sa_data;
     
            //display mac address
             sprintf(Keypad_Message,"%.2x:%.2x:%.2x:%.2x:%.2x:%.2x" , mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    
			break;
		case 0x00:
			//99 35 second
			
 	        fd = socket(AF_INET, SOCK_DGRAM, 0);

 	        /* I want to get an IPv4 IP address */
 	        ifr.ifr_addr.sa_family = AF_INET;

 	        /* I want IP address attached to "eth0" */
 	        strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);

 	        ioctl(fd, SIOCGIFADDR, &ifr);

 	        close(fd);

 	        /* display result */
 	        sprintf(Keypad_Message,"%s", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
			break;
		case 0x28:
			//39 75 fourth
			
			break;
	}    
    ioctl(devfd, PLCM_IOCTL_SET_LINE, 2);
    write(devfd, Keypad_Message, strlen(Keypad_Message));

}while(1);
 return 0;
}