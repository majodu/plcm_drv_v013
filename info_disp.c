#include "menu_objs.h"
#include <sys/file.h>
#include <stdio.h>
#include <string.h>
#include "plcm_ioctl.h"
// #include <unistd.h>
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <sys/ioctl.h>
// #include <netinet/in.h>
// #include <net/if.h>
// #include <arpa/inet.h>    
// #include <stdlib.h>

int main(int argc, char *argv[])
{
	unsigned char Keypad_Value = 0;
	unsigned char detect_dir;
	// creates all the menus and items in the menu_objs c file
	initialize_menus_and_items();
    current_menu = main_menu;
   
    devfd = open("/dev/plcm_drv", O_RDWR);
	if(devfd == -1)
	{
		printf("Can't open /dev/plcm_drv\n");
		return -1;
	}
	// stopps cursor from blinking 
	//ioctl(devfd, PLCM_IOCTL_DISPLAY_B, 0);

    ioctl(devfd, PLCM_IOCTL_SET_LINE, 1);
	// shows the titles of each of the menu items
	show_menu(current_menu);

// this  do while loop checks for button preses and then executes the functions on each menu item
 do{
    Keypad_Value = ioctl(devfd, PLCM_IOCTL_GET_KEYPAD, 0);
    detect_dir=(Keypad_Value & 0x68);
    
	    if(detect_dir == 0x40){
			if(current_menu.item1.route_to == NULL){
				current_menu.item1.action();
			}else{
				current_menu = *(current_menu.item1.route_to);
				show_menu(current_menu);
			}
		}else if(detect_dir == 0x60 ){
     		if(current_menu.item2.route_to == NULL){
				current_menu.item2.action();
			}else{
				current_menu = *(current_menu.item2.route_to);
				show_menu(current_menu);
			}
           
		}else if(detect_dir == 0x48){
			if(current_menu.item3.route_to == NULL){
				current_menu.item3.action();
			}else{
				current_menu = *(current_menu.item3.route_to);
				show_menu(current_menu);
			}
		}else if(detect_dir == 0x68){
			if(current_menu.item4.route_to == NULL){
				current_menu.item4.action();
			}else{
				current_menu = *(current_menu.item4.route_to);
				show_menu(current_menu);
			}
		}
	}while(1);
	return 0;
}