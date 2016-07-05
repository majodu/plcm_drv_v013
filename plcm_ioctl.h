#define PLCM_IOCTL_STOP_THREAD  0x00
#define PLCM_IOCTL_BACKLIGHT    0x01
// Arg 1 = On, 0 = Off
#define PLCM_IOCTL_SET_LINE     0x0D
// Arg 1 = LINE#1, 2 = LINE#2
#define PLCM_IOCTL_CLEARDISPLAY 0x03
#define PLCM_IOCTL_RETURNHOME   0x04
#define PLCM_IOCTL_ENTRYMODE_ID 0x05
// Arg for I/D - Increment or Descrement
#define PLCM_IOCTL_ENTRYMODE_SH 0x06
// Arg for SH - Shift
#define PLCM_IOCTL_DISPLAY_D    0x07
// Arg for Display
#define PLCM_IOCTL_DISPLAY_C    0x08
// Arg for Cursor
#define PLCM_IOCTL_DISPLAY_B    0x09
// Arg for Cursor Blinking
#define PLCM_IOCTL_SHIFT_SC     0x0A
// Arg for S/C - Display Shift/Cursor Shift
#define PLCM_IOCTL_SHIFT_RL     0x0B
//Arg for R/L - Right/Left
#define PLCM_IOCTL_GET_KEYPAD   0x0C
//Input char
#define PLCM_IOCTL_INPUT_CHAR  0x0E
