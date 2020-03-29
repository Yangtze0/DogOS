
#include "dogos.h"

void itoa(unsigned int n, char *buf);
int atoi(char *pstr);
void xtoa(unsigned int n, char *buf);
int isDigit(unsigned char c);
int isLetter(unsigned char c);

#define va_arg(ap, type) ((type *) (ap += sizeof(type)))[-1]

// 格式化存储字符串 %(d c x s %) %0n(d x s)
int sprintf(char *str, const char *fmt, ...) {
    int count = 0;
    char c;
    char *s;
    int n;

    int index = 0;
    int ret = 2;
    
    char buf[65];
    char digit[16];
    int num = 0;
    int len = 0;
    
    memset(buf, 0, sizeof(buf));
    memset(digit, 0, sizeof(digit));

    char *ap = (char *)(&fmt+1);

    while(*fmt) {
        if(*fmt == '%') {
            fmt++;
            switch(*fmt) {
            case 'd':   // 整型
                n = va_arg(ap, int);
                if(n < 0) {
                    *str++ = '-';
                    n = -n;
                }
                itoa(n, buf);
                memcpy(str, buf, strlen(buf));
                str += strlen(buf);
                break;
            case 'c':   // 字符型
                c = va_arg(ap, int);
                *str++ = c;
                break;
            case 'x':   // 16进制
                n = va_arg(ap, int);
                xtoa(n, buf);
                memcpy(str, buf, strlen(buf));
                str += strlen(buf);
                break;
            case 's':   // 字符串
                s = va_arg(ap, char *);
                memcpy(str, s, strlen(s));
                str += strlen(s);
                break;
            case '%':   // 输出%
                *str++ = '%';
                break;
            case '0':   // 位不足的左补0
                index = 0;
                num = 0;
                memset(digit, 0, sizeof(digit));
                while(1) {
                    fmt++;
                    ret = isDigit(*fmt);
                    if(ret == 1) {  // 是数字
                        digit[index] = *fmt;
                        index++;
                    } else {
                        num = atoi(digit);
                        break;
                    }
                }
                switch(*fmt) {
                case 'd':   // 整型
                    n = va_arg(ap, int);
                    if(n < 0) {
                        *str = '-';
                        str++;
                        n = -n;
                    }    
                    itoa(n, buf);
                    len = strlen(buf);
                    if(len >= num) {
                        memcpy(str, buf, strlen(buf));
                        str += strlen(buf);
                    } else {
                        memset(str, '0', num-len);
                        str += num-len;
                        memcpy(str, buf, strlen(buf));
                        str += strlen(buf);
                    }
                    break;
                case 'x':   // 16进制
                    n = va_arg(ap, int);
                    xtoa(n, buf);
                    len = strlen(buf);
                    if(len >= num) {
                        memcpy(str, buf, len);
                        str += len;
                    } else {
                        memset(str, '0', num-len);
                        str += num-len;
                        memcpy(str, buf, len);
                        str += len;
                    }
                    break;
                case 's': // 字符串
                    s = va_arg(ap, char *);
                    len = strlen(s);
                    if(len >= num) {
                        memcpy(str, s, strlen(s));
                        str += strlen(s);
                    } else {
                        memset(str, '0', num-len);
                        str += num-len;
                        memcpy(str, s, strlen(s));
                        str += strlen(s);
                    }
                    break;
                default:
                    break;
                }
            default:
                break;
            }
        } else {
            *str = *fmt;
            str++;
        }
        fmt++;
    }

    *str = 0;
    return count;
}

// int -> str(10)
void itoa(unsigned int n, char * buf) {
    int i;
    if(n < 10) {
        buf[0] = n + '0';
        buf[1] = '\0';
        return;
    }
    itoa(n / 10, buf);
    for(i=0; buf[i]!='\0'; i++);
    buf[i] = (n % 10) + '0';
    buf[i+1] = '\0';
}

// str(10) -> int
int atoi(char* pstr) {
    int int_ret = 0;
    int int_sign = 1;
    if(pstr == '\0') return -1;

    while(((*pstr) == ' ') || ((*pstr) == '\n') || ((*pstr) == '\t') || ((*pstr) == '\b'))
        pstr++;

    if(*pstr == '-') int_sign = -1;
    if(*pstr == '-' || *pstr == '+') pstr++;

    while(*pstr >= '0' && *pstr <= '9') {
        int_ret = int_ret * 10 + *pstr - '0';
        pstr++;
    }

    int_ret = int_sign * int_ret;
    return int_ret;
}

// int -> str(hex)
void xtoa(unsigned int n, char *buf) {
    int i;
    if(n < 16) {
        if(n < 10) {
            buf[0] = n + '0';
        } else {
            buf[0] = n - 10 + 'a';
        }
        buf[1] = '\0';
        return;
    }
    xtoa(n / 16, buf);    
    for(i = 0; buf[i] != '\0'; i++);    
    if((n % 16) < 10) {
        buf[i] = (n % 16) + '0';
    } else {
        buf[i] = (n % 16) - 10 + 'a';
    }
    buf[i + 1] = '\0';
}

int isDigit(unsigned char c) {
    if (c >= '0' && c <= '9') return 1;
    else return 0;
}

int isLetter(unsigned char c) {
    if (c >= 'a' && c <= 'z') return 1;
    else if (c >= 'A' && c <= 'Z') return 1;
    else return 0;
}

void *memset(void *s, int c, unsigned long len) {
    char *xs = (char *)s;
    while (len--) *xs++ = c;
    return s;
}

void *memcpy(void *dest, const void *src, unsigned long count) {
    char *d = dest, *s = (char *)src;
    for(unsigned long i = 0; i < count; i++) *d++ = *s++;
    return dest;
}

char *strcpy(char *dest, const char *src) {
    char *tmp = dest;
    while ((*dest++ = *src++));
    return tmp;
}

int strcmp(const char *src, const char *dst) {
    int ret = 0;
    while(!(ret=*src-*dst) && *dst) {
        src++;
        dst++;
    }
    return ret;
}

unsigned long strlen(const char *s) {
    const char *sc;
    for (sc=s; *sc; ++sc);
    return sc-s;
}
