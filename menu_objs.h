struct menu_item{
    char title[10];
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

// making menus and items
void initialize_menus_and_items();

// menu operations 
void print_menu_items(struct menu m);

//pclm variables
int devfd;
