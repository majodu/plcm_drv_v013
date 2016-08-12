struct menu_item{
    char title[9];
    void (*action) (void);
    struct menu *route_to;
};

struct menu{
    struct menu_item item1;
    struct menu_item item2;
    struct menu_item item3;
    struct menu_item item4;
};

// menus
struct menu main_menu;
struct menu current_menu;
struct menu ip_menu;
struct menu are_you_sure;
// making menus and items
void initialize_menus_and_items();

// menu operations 
// print menu items is mostly for debugging
void print_menu_items(struct menu m);
// this is for displaying the actuall menu
void show_menu(struct menu m);
// called every button press to test if new submenu or action is called
void on_btn_press(struct menu_item i);

//pclm variables
int devfd;

