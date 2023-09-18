#pragma once
#include <stdint.h>
#include <stdarg.h>
#include <hal/vfs.h>



typedef signed char int8;
typedef unsigned char uint8;

typedef signed short int16;
typedef unsigned short uint16;

typedef signed int int32;
typedef unsigned int uint32;

typedef signed long long int64;
typedef unsigned long long uint64;

typedef char* string; 


void clearLine(uint8 from,uint8 to);
void updateCursor();
void clearScreen();
void newLineCheck();
void printch(char c);//new ones

void scrollUp(uint8 lineNumber);
void fputc(char c, fd_t file);
void fputs(const char* str, fd_t file);
void vfprintf(fd_t file, const char* fmt, va_list args);
void fprintf(fd_t file, const char* fmt, ...);
void fprint_buffer(fd_t file, const char* msg, const void* buffer, uint32_t count);

void putc(char c);
void puts(const char* str);
void printf(const char* fmt, ...);
void print_buffer(const char* msg, const void* buffer, uint32_t count);

void debugc(char c);
void debugs(const char* str);
void debugf(const char* fmt, ...);
void debug_buffer(const char* msg, const void* buffer, uint32_t count);