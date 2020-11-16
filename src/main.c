#include <stdio.h>
#include "MD5.h"
int main()
{
    //byte *msg = "jdfgsdhfsdfsd 156445dsfsd7fg/*/+bfjsdgf%$^";
    printf("%s\n", MD5("jdfgsdhfsdfsd 156445dsfsd7fg/*/+bfjsdgf%$^"));
    //printf("%s\n", HMAC_MD5("key","The quick brown fox jumps over the lazy dog"));
}