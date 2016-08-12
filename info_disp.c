#include "menu_objs.h"
#include <sys/file.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "plcm_ioctl.h"



int main(int argc, char *argv[])
{
	unsigned char Keypad_Value = 0;
	unsigned char detect_dir;
	unsigned char detect_press;
	char Keypad_Message[19] = "";
	unsigned char Pre_Value = 0;
	int btn1_state = 0;
	int btn2_state = 0;
	int btn3_state = 0;
	int btn4_state = 0;
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
	//clear previous keypad status
	ioctl(devfd, PLCM_IOCTL_GET_KEYPAD, 0); 

	// shows the titles of each of the menu items
	show_menu(current_menu);

    Pre_Value = ioctl(devfd, PLCM_IOCTL_GET_KEYPAD, 0);

// this  do while loop checks for button preses and then executes the functions on each menu item
 do{
    Keypad_Value = ioctl(devfd, PLCM_IOCTL_GET_KEYPAD, 0);
	if(Pre_Value != Keypad_Value)
		{
			detect_press=(Keypad_Value & 0x40);
			detect_dir=(Keypad_Value & 0x28);
			if(detect_press == 0x40){
				switch(detect_dir){
			    	case 0x00: // left
						btn2_state = 0;
						btn3_state = 0;
						btn4_state = 0;
						if(btn1_state == 1){
							show_menu(current_menu);
							btn1_state = 0;
						}else{
							on_btn_press(current_menu.item1);
							btn1_state = 1;
						}
					break;
			    	case 0x20: // up
						btn1_state = 0;
						btn3_state = 0;
						btn4_state = 0;
						if(btn2_state == 1){
							show_menu(current_menu);
							btn2_state = 0;
						}else{
							on_btn_press(current_menu.item2);
							btn2_state = 1;
						} 
					break;
			    	case 0x08: // down
						btn1_state = 0;
						btn2_state = 0;
						btn4_state = 0;
						if(btn3_state == 1){
							show_menu(current_menu);
							btn3_state = 0;
						}else{
							on_btn_press(current_menu.item3);
							btn3_state = 1;
						} 
					break;
					case 0x28: // right
						btn1_state = 0;
						btn2_state = 0;
						btn3_state = 0;
						if(btn4_state == 1){
							show_menu(current_menu);
							btn4_state = 0;
						}else{
							on_btn_press(current_menu.item4);
							btn4_state = 1;
						}
					break;
				}
				
			}
			
			Pre_Value = Keypad_Value;
		}
		usleep(100000);
}while(1);
out:
	close(devfd);
	return 0;
}