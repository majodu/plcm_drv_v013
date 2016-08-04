#include "menu_objs.h"
#include <sys/file.h>
#include <stdio.h>
#include <string.h>
#include "plcm_ioctl.h"


int main(int argc, char *argv[])
{
	unsigned char Keypad_Value = 0;
	unsigned char detect_dir;
	unsigned char Cur_Display = 0x08; // Current Display On/Off Ctrl
	unsigned char Ctrl = 0;

	if(argc == 2)
        {
                if(strcmp("-stop", argv[1]) == 0)
                {
                        printf("plcm_drv thread has been stopped.\n");
                        printf("sled_drv : PLCM_IOCTL_STOP_THREAD\n");
		        goto out;
        		}
		}
	// creates all the menus and items in the menu_objs c file
	initialize_menus_and_items();
    current_menu = main_menu;
   
    devfd = open("/dev/plcm_drv", O_RDWR);
	if(devfd == -1)
	{
		printf("Can't open /dev/plcm_drv\n");
		return -1;
	}

	// stops cursor from blinking 
	ioctl(devfd, PLCM_IOCTL_DISPLAY_B, 0);
	ioctl(devfd, PLCM_IOCTL_DISPLAY_C, 0);

    
	// shows the titles of each of the menu items
	//ioctl(devfd, PLCM_IOCTL_SET_LINE, 1);
	show_menu(current_menu);

// this  do while loop checks for button preses and then executes the functions on each menu item
 do{
    Keypad_Value = ioctl(devfd, PLCM_IOCTL_GET_KEYPAD, 0);
    detect_dir=(Keypad_Value & 0x68);
    sleep(1);
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
out:
	close(devfd);
	return 0;
}