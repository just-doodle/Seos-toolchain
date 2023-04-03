#include "keyboard.h"

static keyboard_handler_t khandler = NULL;

void init_keyboard()
{

    register_interrupt_handler(0x21, keyboard_callback);
    while(inb(KEYBOARD_COMMAND_PORT) & 0x1)
        inb(KEYBOARD_DATA_PORT);
    outb(KEYBOARD_COMMAND_PORT, 0xAE);
    outb(KEYBOARD_COMMAND_PORT, 0x20);
    uint8_t status = (inb(KEYBOARD_DATA_PORT) | 1) & ~0x10;
    outb(KEYBOARD_COMMAND_PORT, 0x60);
    outb(KEYBOARD_DATA_PORT, status);

    outb(KEYBOARD_DATA_PORT, 0xF4);
}

void change_keyboard_handler(keyboard_handler_t handler)
{
    if(handler == NULL)
    {
        printf("No keyboard handler given\n");
        handler = NULL;
        return;
    }
    else
    {
        khandler = handler;
    }
}

void keyboard_callback(registers_t *reg)
{
    uint8_t scancode;
    for(int i = 1000; i > 0; i++)
    {
        if((inb(KEYBOARD_COMMAND_PORT) & 1) == 0) continue;
        scancode = inb(KEYBOARD_DATA_PORT);
        break;
    }

    if(khandler != NULL)
    {
        khandler(scancode);
    }
    else
    {
        printf("No keyboard handler\n");
    }
}

bool isCTRL = 0;
bool isALT = 0;
bool isShift = 0;

char kcodeTochar(uint8_t scancode)
{
    char key = 0;
    if (scancode < 80)
    {
        switch (scancode)
        {
        case 0xFA:
            break;
        case 0x45:
        case 0xC5:
            break;
        case 0x1D:
            isCTRL = 1;
            break;
        case 0x38:
            isALT = 1;
            break;
        case 0x2A:
        case 0x36:
            isShift = 1;
            break;
        case 0x0E:
            key = '\b';
            break;
        case 0x0F:
            key = '\t';
            break;
        case 0x29:
            if (!isShift)
                key = '`';
            else
                key = '~';
            break;
        case 0x02:
            if (!isShift)
                key = '1';
            else
                key = '!';
            break;
        case 0x03:
            if (!isShift)
                key = '2';
            else
                key = '@';
            break;
        case 0x04:
            if (!isShift)
                key = '3';
            else
                key = '#';
            break;
        case 0x05:
            if (!isShift)
                key = '4';
            else
                key = '$';
            break;
        case 0x06:
            if (!isShift)
                key = '5';
            else
                key = '%';
            break;
        case 0x07:
            if (!isShift)
                key = '6';
            else
                key = '^';
            break;
        case 0x08:
            if (!isShift)
                key = '7';
            else
                key = '&';
            break;
        case 0x09:
            if (!isShift)
                key = '8';
            else
                key = '*';
            break;
        case 0x0A:
            if (!isShift)
                key = '9';
            else
                key = '(';
            break;
        case 0x0B:
            if (!isShift)
                key = '0';
            else
                key = ')';
            break;
        case 0x0C:
            if (!isShift)
                key = '-';
            else
                key = '_';
            break;
        case 0x0D:
            if (!isShift)
                key = '=';
            else
                key = '+';
            break;

        case 0x10:
            if (!isShift)
                key = 'q';
            else
                key = 'Q';
            break;
        case 0x11:
            if (!isShift)
                key = 'w';
            else
                key = 'W';
            break;
        case 0x12:
            if (!isShift)
                key = 'e';
            else
                key = 'E';
            break;
        case 0x13:
            if (!isShift)
                key = 'r';
            else
                key = 'R';
            break;
        case 0x14:
            if (!isShift)
                key = 't';
            else
                key = 'T';
            break;
        case 0x15:
            if (!isShift)
                key = 'y';
            else
                key = 'Y';
            break;
        case 0x16:
            if (!isShift)
                key = 'u';
            else
                key = 'U';
            break;
        case 0x17:
            if (!isShift)
                key = 'i';
            else
                key = 'I';
            break;
        case 0x18:
            if (!isShift)
                key = 'o';
            else
                key = 'O';
            break;
        case 0x19:
            if (!isShift)
                key = 'p';
            else
                key = 'P';
            break;
        case 0x1A:
            if (!isShift)
                key = '[';
            else
                key = '{';
            break;
        case 0x1B:
            if (!isShift)
                key = ']';
            else
                key = '}';
            break;
        case 0x1C:
            if (!isShift)
                key = '\n';
            else
                key = '\n';
            break;

        case 0x1E:
            if (!isShift)
                key = 'a';
            else
                key = 'A';
            break;
        case 0x1F:
            if (!isShift)
                key = 's';
            else
                key = 'S';
            break;
        case 0x20:
            if (!isShift)
                key = 'd';
            else
                key = 'D';
            break;
        case 0x21:
            if (!isShift)
                key = 'f';
            else
                key = 'F';
            break;
        case 0x22:
            if (!isShift)
                key = 'g';
            else
                key = 'G';
            break;
        case 0x23:
            if (!isShift)
                key = 'h';
            else
                key = 'H';
            break;
        case 0x24:
            if (!isShift)
                key = 'j';
            else
                key = 'J';
            break;
        case 0x25:
            if (!isShift)
                key = 'k';
            else
                key = 'K';
            break;
        case 0x26:
            if (!isShift)
                key = 'l';
            else
                key = 'L';
            break;
        case 0x27:
            if (!isShift)
                key = ';';
            else
                key = ':';
            break;

        case 0x2C:
            if (!isShift)
                key = 'z';
            else
                key = 'Z';
            break;
        case 0x2D:
            if (!isShift)
                key = 'x';
            else
                key = 'X';
            break;
        case 0x2E:
            if (!isShift)
                key = 'c';
            else
                key = 'C';
            break;
        case 0x2F:
            if (!isShift)
                key = 'v';
            else
                key = 'V';
            break;
        case 0x30:
            if (!isShift)
                key = 'b';
            else
                key = 'B';
            break;
        case 0x31:
            if (!isShift)
                key = 'n';
            else
                key = 'N';
            break;
        case 0x32:
            if (!isShift)
                key = 'm';
            else
                key = 'M';
            break;
        case 0x33:
            if (!isShift)
                key = ',';
            else
                key = '<';
            break;
        case 0x34:
            if (!isShift)
                key = '.';
            else
                key = '>';
            break;
        case 0x35:
            if (!isShift)
                key = '/';
            else
                key = '?';
            break;
        case 0x39:
            if (!isShift)
                key = ' ';
            else
                key = ' ';
            break;
        case 0x2B:
            if (!isShift)
                key = '\\';
            else
                key = '|';
            break;
        case 0x3A:
            if (isShift)
            {
                isShift = false;
            }
            else
            {
                isShift = true;
            }
            break;
        default:
            key = 0;
            break;
        }
    }
    return key;
}