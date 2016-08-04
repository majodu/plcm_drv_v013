#include "menu_objs.h"
#include <string.h>
#include <stdio.h>
#include <sys/file.h>
#include "plcm_ioctl.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>    
#include <stdlib.h>
//test

struct menu_item make_menu_item(char title[], void (*action) (void), struct menu *route_to);
struct menu make_menu(struct menu_item item1,struct menu_item item2,struct menu_item item3,struct menu_item item4);

// menu items
struct menu_item empty_item;
struct menu_item mac;
struct menu_item ip;
struct menu_item status;
struct menu_item ren_dhcp;
struct menu_item restart;

// actions
void empty_action();
void print_mac();
void print_ip();
void print_status();
void renew_dhcp();
void system_restart(); // dont use broken
// function that makes all the menus and items

void initialize_menus_and_items(){

    // adding titles and actions to menu items use the empty item if you want to have a menu less than 4 items
    // NOTE:only the first 8 characters will be displayed
    empty_item = make_menu_item("",empty_action,NULL);
    mac = make_menu_item("MAC",print_mac,NULL);
    ip = make_menu_item("IP",print_ip,NULL);
    status = make_menu_item("Status",print_status,NULL);
    ren_dhcp = make_menu_item("rnw DHCP",renew_dhcp,NULL);
    //restart = make_menu_item("Restart",system_restart,NULL);

    // making menus
    // make sure to put the empty_item function if you are not using all 4 slots
    main_menu = make_menu(mac,ip,status,ren_dhcp);

}
// menu functions
// debugging function
void print_menu_items(struct menu m){
    printf("%s,%s,%s,%s \n",m.item1.title,m.item2.title,m.item3.title,m.item4.title);
}
// displays the menu on screen
void show_menu(struct menu m){
    unsigned char first[19] = "";
    unsigned char second[19] = "";
    unsigned char p1[9] = "";
    unsigned char p2[9] = "";
    unsigned char p3[9] = "";
    unsigned char p4[9] = "";
    strncpy(p1,m.item1.title,8);
    strncpy(p2,m.item2.title,8);
    strncpy(p3,m.item3.title,8);
    strncpy(p4,m.item4.title,8);
    p1[9] = 0;
    p2[9] = 0;
    p3[9] = 0;
    p4[9] = 0;

    sprintf(first,"%-9s %-9s",p1,p2);
    sprintf(second,"%-9s %-9s",p3,p4);

    ioctl(devfd, PLCM_IOCTL_SET_LINE, 1);
    write(devfd, first, strlen(first));
    ioctl(devfd, PLCM_IOCTL_SET_LINE, 2);
    write(devfd, second, strlen(second));
}
// tests if action or submenu is called
void on_btn_press(struct menu_item i){
    if(i.route_to == NULL){
		i.action();
	}else{
		current_menu = *(i.route_to);
		show_menu(current_menu);
	}
}

// actions
void empty_action(){}

// prints the IP address to the screen
void print_ip(){
    unsigned char Keypad_Message[40] = "IP:";
    int fd;
    struct ifreq ifr;
    //writes "IP:" to the first line
    ioctl(devfd, PLCM_IOCTL_SET_LINE, 1);
    write(devfd, Keypad_Message, strlen(Keypad_Message));

    fd = socket(AF_INET, SOCK_DGRAM, 0);

 	// I want to get an IPv4 IP address 
 	ifr.ifr_addr.sa_family = AF_INET;

 	// I want IP address attached to "eth0" 
 	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);

 	ioctl(fd, SIOCGIFADDR, &ifr);

 	close(fd);
 	// writes the actuall MAC address to the second line
 	sprintf(Keypad_Message,"%s", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
    ioctl(devfd, PLCM_IOCTL_SET_LINE, 2);
    write(devfd, Keypad_Message, strlen(Keypad_Message));
}

// prints the MAC address to the screen
void print_mac(){
    unsigned char Keypad_Message[40] = "MAC:";
    int fd;
    struct ifreq ifr;
    unsigned char *mac;

    //writes "MAC:" to the first line
    ioctl(devfd, PLCM_IOCTL_SET_LINE, 1);
    write(devfd, Keypad_Message, strlen(Keypad_Message));

    fd = socket(AF_INET, SOCK_DGRAM, 0);

 	// I want to get an IPv4 IP address 
    ifr.ifr_addr.sa_family = AF_INET;
	// I want IP address attached to "eth0" 
    strncpy(ifr.ifr_name , "eth0" , IFNAMSIZ-1);
 
    ioctl(fd, SIOCGIFHWADDR, &ifr);
 
    close(fd);
     
    mac = (unsigned char *)ifr.ifr_hwaddr.sa_data;
     		
    // writes the actuall MAC address to the second line
    sprintf(Keypad_Message,"%.2x:%.2x:%.2x:%.2x:%.2x:%.2x" , mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    ioctl(devfd, PLCM_IOCTL_SET_LINE, 2);
    write(devfd, Keypad_Message, strlen(Keypad_Message));
}

// prints the smonitor status to the screen
void print_status(){
   
    FILE *fp;
    char path[1024];
    char Keypad_Message[19] = "Loading...";
    ioctl(devfd, PLCM_IOCTL_SET_LINE, 2);
    write(devfd, Keypad_Message, strlen(Keypad_Message));
    strcpy(Keypad_Message,"Status:");
    ioctl(devfd, PLCM_IOCTL_SET_LINE, 1);
    write(devfd, Keypad_Message, strlen(Keypad_Message));
    
    fp = popen("smonitor getstatus 2>&1", "r");
    if (fp == NULL) {
        printf("Failed to run command\n" );
        exit(1);
    }
    while (fgets(path, sizeof(path), fp) != NULL) {
    }
    pclose(fp);
    if(strlen(path) > 19){
        strncpy(Keypad_Message,path,19);
    }else{
        if(strlen(path) != 0){
            memset(Keypad_Message, '\0', sizeof(Keypad_Message));
            strncpy(Keypad_Message,path,strlen(path)-1);
        }
    }
    ioctl(devfd, PLCM_IOCTL_SET_LINE, 2);
    write(devfd, Keypad_Message, strlen(Keypad_Message));
}

// renew DHCP certificate
void renew_dhcp(){
    system("sudo dhclient -r; sudo dhclient");
}

// restart the lanner box BROKEN
void system_restart(){
    FILE *fp;
    char path[1024];
    unsigned char Keypad_Message[19] = "";
    ioctl(devfd, PLCM_IOCTL_SET_LINE, 1);
    write(devfd, Keypad_Message, strlen(Keypad_Message));
    ioctl(devfd, PLCM_IOCTL_SET_LINE, 2);
    strcpy(Keypad_Message,"Please Wait...");
    write(devfd, Keypad_Message, strlen(Keypad_Message));
    fp = popen("smonitor restart system 2>&1", "r");
    if (fp == NULL) {
        printf("Failed to run command\n" );
        exit(1);
    }
    while (fgets(path, sizeof(path), fp) != NULL) {
    }
    pclose(fp);
    strcpy(Keypad_Message,"All Done");
    write(devfd, Keypad_Message, strlen(Keypad_Message));
    //system("smonitor restart system");
}

// make menu item with a title, function, and route if it leads to another menu otherwise its NULL an the function is empty_action()
struct menu_item make_menu_item(char title[10], void (*action) (void), struct menu *route_to){
    struct menu_item temp;

    strcpy(temp.title,title);
    temp.action = action;
    temp.route_to = route_to;

    return temp;
}
//  make menu with for menu item arguments
struct menu make_menu(struct menu_item item1,struct menu_item item2,struct menu_item item3,struct menu_item item4){
    struct menu temp;

    temp.item1 = item1;
    temp.item2 = item2;
    temp.item3 = item3;
    temp.item4 = item4;

    return temp;
}

