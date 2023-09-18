#include <stdio.h>
#include <arch/i686/io.h>

#include <stdarg.h>
#include <stdbool.h>

#include <hal/vfs.h>

int cursorX = 0, cursorY = 0;
const uint8 sw = 80,sh = 25,sd = 2;
                                                 

void clearLine(uint8 from,uint8 to)
{
        uint16 i = sw * from * sd;
        string vidmem=(string)0xb8000;
        for(i;i<(sw*to*sd);i++)
        {
                vidmem[i] = 0x0;
        }
}
void updateCursor()
{
    unsigned temp;

    temp = cursorY * sw + cursorX;                                                      // Position = (y * width) +  x

    outportb(0x3D4, 14);                                                                // CRT Control Register: Select Cursor Location
    outportb(0x3D5, temp >> 8);                                                         // Send the high byte across the bus
    outportb(0x3D4, 15);                                                                // CRT Control Register: Select Send Low byte
    outportb(0x3D5, temp);                                                              // Send the Low byte of the cursor location
}
void clearScreen()
{
        clearLine(0,sh-1);
        cursorX = 0;
        cursorY = 0;
        updateCursor();
}

void scrollUp(uint8 lineNumber)
{
        string vidmem = (string)0xb8000;
        uint16 i = 0;
        clearLine(0,lineNumber-1);                                            //updated
        for (i;i<sw*(sh-1)*2;i++)
        {
                vidmem[i] = vidmem[i+sw*2*lineNumber];
        }
        clearLine(sh-1-lineNumber,sh-1);
        if((cursorY - lineNumber) < 0 ) 
        {
                cursorY = 0;
                cursorX = 0;
        } 
        else 
        {
                cursorY -= lineNumber;
        }
        updateCursor();
}


void newLineCheck()
{
        if(cursorY >=sh-1)
        {
                scrollUp(1);
        }
}

void printch(char c)
{
    string vidmem = (string) 0xb8000;     
    switch(c)
    {
        case (0x08):
                if(cursorX > 0) 
                {
	                cursorX--;									
                        vidmem[(cursorY * sw + cursorX)*sd]=0x00;	                              
	        }
	        break;
        case (0x09):
                cursorX = (cursorX + 8) & ~(8 - 1); 
                break;
        case ('\r'):
                cursorX = 0;
                break;
        case ('\n'):
                cursorX = 0;
                cursorY++;
                break;
        default:
                vidmem [((cursorY * sw + cursorX))*sd] = c;
                vidmem [((cursorY * sw + cursorX))*sd+1] = 0x0F;
                cursorX++; 
                break;
	
    }
    if(cursorX >= sw)                                                                   
    {
        cursorX = 0;                                                                
        cursorY++;                                                                    
    }
    updateCursor();
    newLineCheck();
}


void fputc(char c, fd_t file)
{
    VFS_Write(file, &c, sizeof(c));
}

void fputs(const char* str, fd_t file)
{
    while(*str)
    {
        fputc(*str, file);
        str++;
    }
}

#define PRINTF_STATE_NORMAL         0
#define PRINTF_STATE_LENGTH         1
#define PRINTF_STATE_LENGTH_SHORT   2
#define PRINTF_STATE_LENGTH_LONG    3
#define PRINTF_STATE_SPEC           4

#define PRINTF_LENGTH_DEFAULT       0
#define PRINTF_LENGTH_SHORT_SHORT   1
#define PRINTF_LENGTH_SHORT         2
#define PRINTF_LENGTH_LONG          3
#define PRINTF_LENGTH_LONG_LONG     4

const char g_HexChars[] = "0123456789abcdef";

void fprintf_unsigned(fd_t file, unsigned long long number, int radix)
{
    char buffer[32];
    int pos = 0;

    // convert number to ASCII
    do 
    {
        unsigned long long rem = number % radix;
        number /= radix;
        buffer[pos++] = g_HexChars[rem];
    } while (number > 0);

    // print number in reverse order
    while (--pos >= 0)
        fputc(buffer[pos], file);
}

void fprintf_signed(fd_t file, long long number, int radix)
{
    if (number < 0)
    {
        fputc('-', file);
        fprintf_unsigned(file, -number, radix);
    }
    else fprintf_unsigned(file, number, radix);
}

void vfprintf(fd_t file, const char* fmt, va_list args)
{
    int state = PRINTF_STATE_NORMAL;
    int length = PRINTF_LENGTH_DEFAULT;
    int radix = 10;
    bool sign = false;
    bool number = false;

    while (*fmt)
    {
        switch (state)
        {
            case PRINTF_STATE_NORMAL:
                switch (*fmt)
                {
                    case '%':   state = PRINTF_STATE_LENGTH;
                                break;
                    default:    fputc(*fmt, file);
                                break;
                }
                break;

            case PRINTF_STATE_LENGTH:
                switch (*fmt)
                {
                    case 'h':   length = PRINTF_LENGTH_SHORT;
                                state = PRINTF_STATE_LENGTH_SHORT;
                                break;
                    case 'l':   length = PRINTF_LENGTH_LONG;
                                state = PRINTF_STATE_LENGTH_LONG;
                                break;
                    default:    goto PRINTF_STATE_SPEC_;
                }
                break;

            case PRINTF_STATE_LENGTH_SHORT:
                if (*fmt == 'h')
                {
                    length = PRINTF_LENGTH_SHORT_SHORT;
                    state = PRINTF_STATE_SPEC;
                }
                else goto PRINTF_STATE_SPEC_;
                break;

            case PRINTF_STATE_LENGTH_LONG:
                if (*fmt == 'l')
                {
                    length = PRINTF_LENGTH_LONG_LONG;
                    state = PRINTF_STATE_SPEC;
                }
                else goto PRINTF_STATE_SPEC_;
                break;

            case PRINTF_STATE_SPEC:
            PRINTF_STATE_SPEC_:
                switch (*fmt)
                {
                    case 'c':   fputc((char)va_arg(args, int), file);
                                break;

                    case 's':   
                                fputs(va_arg(args, const char*), file);
                                break;

                    case '%':   fputc('%', file);
                                break;

                    case 'd':
                    case 'i':   radix = 10; sign = true; number = true;
                                break;

                    case 'u':   radix = 10; sign = false; number = true;
                                break;

                    case 'X':
                    case 'x':
                    case 'p':   radix = 16; sign = false; number = true;
                                break;

                    case 'o':   radix = 8; sign = false; number = true;
                                break;

                    // ignore invalid spec
                    default:    break;
                }
                if (number)
                {
                    if (sign)
                    {
                        switch (length)
                        {
                        case PRINTF_LENGTH_SHORT_SHORT:
                        case PRINTF_LENGTH_SHORT:
                        case PRINTF_LENGTH_DEFAULT:     fprintf_signed(file, va_arg(args, int), radix);
                                                        break;

                        case PRINTF_LENGTH_LONG:        fprintf_signed(file, va_arg(args, long), radix);
                                                        break;

                        case PRINTF_LENGTH_LONG_LONG:   fprintf_signed(file, va_arg(args, long long), radix);
                                                        break;
                        }
                    }
                    else
                    {
                        switch (length)
                        {
                        case PRINTF_LENGTH_SHORT_SHORT:
                        case PRINTF_LENGTH_SHORT:
                        case PRINTF_LENGTH_DEFAULT:     fprintf_unsigned(file, va_arg(args, unsigned int), radix);
                                                        break;
                                                        
                        case PRINTF_LENGTH_LONG:        fprintf_unsigned(file, va_arg(args, unsigned  long), radix);
                                                        break;

                        case PRINTF_LENGTH_LONG_LONG:   fprintf_unsigned(file, va_arg(args, unsigned  long long), radix);
                                                        break;
                        }
                    }
                }
                // reset state
                state = PRINTF_STATE_NORMAL;
                length = PRINTF_LENGTH_DEFAULT;
                radix = 10;
                sign = false;
                number = false;
                break;
        }

        fmt++;
    }
}
void fprintf(fd_t file, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf(file, fmt, args);
    va_end(args);
}
void fprint_buffer(fd_t file, const char* msg, const void* buffer, uint32_t count)
{
    const uint8_t* u8Buffer = (const uint8_t*)buffer;
    
    fputs(msg, file);
    for (uint16_t i = 0; i < count; i++)
    {
        fputc(g_HexChars[u8Buffer[i] >> 4], file);
        fputc(g_HexChars[u8Buffer[i] & 0xF], file);
    }
    fputs("\n", file);
}
void putc(char c)
{
    fputc(c, VFS_FD_STDOUT);
}
void puts(const char* str)
{
    fputs(str, VFS_FD_STDOUT);
}
void printf(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf(VFS_FD_STDOUT, fmt, args);
    va_end(args);
}

void print_buffer(const char* msg, const void* buffer, uint32_t count)
{
    fprint_buffer(VFS_FD_STDOUT, msg, buffer, count);
}

void debugc(char c)
{
    fputc(c, VFS_FD_DEBUG);
}

void debugs(const char* str)
{
    fputs(str, VFS_FD_DEBUG);
}

void debugf(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf(VFS_FD_DEBUG, fmt, args);
    va_end(args);
}

void debug_buffer(const char* msg, const void* buffer, uint32_t count)
{
    fprint_buffer(VFS_FD_DEBUG, msg, buffer, count);
}
