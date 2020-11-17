#include <stdio.h>
#include "HMAC-MD5.h"

// 对 MD5 的测例来自维基百科与 RFC-1321 文档
byte *MD5_cases[] = {
    "",
    "The quick brown fox jumps over the lazy dog",
    "a",
    "abc",
    "message digest",
    "abcdefghijklmnopqrstuvwxyz",
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
    "12345678901234567890123456789012345678901234567890123456789012345678901234567890"
};

byte *MD5_expected[] = {
    "d41d8cd98f00b204e9800998ecf8427e",
    "9e107d9d372bb6826bd81d3542a419d6",
    "0cc175b9c0f1b6a831c399e269772661",
    "900150983cd24fb0d6963f7d28e17f72",
    "f96b697d7cb7938d525a2f31aaf161d0",
    "c3fcd3d76192e4007dfb496cca67e13b",
    "d174ab98d277d9f5a5611c2c9f419d9f",
    "57edf4a22be3c955ac49da2e2107b67a"
};

void test_MD5()
{
    printf("---Test of MD5 start---\n");
    for(int i = 0; i < 8; i++)
    {
        byte *myResult = MD5(MD5_cases[i]);
        printf("Expected  : %s\n", MD5_expected[i]);
        printf("Got       : %s\n", myResult);
        if(strcmp(myResult, MD5_expected[i]) == 0)
            printf("Asserted equal!\n");
        else
            printf("Not equal!\n");
        printf("\n");
    }
    printf("---Test of MD5 end---\n\n");
}


// 对 HMAC-MD5 的测例来自维基百科和 HMAC Generator
byte *HMAC_MD5_cases_keys[] = {
    "key",
    "key",
    "key",
    "12345678901234567890123456789012345678901234567890123456789012345678901234567890",
    "12345678901234567890123456789012345678901234567890123456789012345678901234567890",
};

byte *HMAC_MD5_cases_data[] = {
    "Hi There",
    "The quick brown fox jumps over the lazy dog",
    "Test Using Larger Than Block-Size Key and Larger Than One Block-Size Data",
    "Hi There",
    "The quick brown fox jumps over the lazy dog",
};

byte *HMAC_MD5_expected[] = {
    "eb01ff92f00d651abcdd1f56f1a74725",
    "80070713463e7749b90c2dc24911e275",
    "93ec1c6a0bc2889f1e87da3f88f0fca6",
    "5d84b4bd8ce8797ffea3869fc0097e70",
    "02021d12d54c48fcfaf832345ed8904d",
};

void test_HMAC_MD5()
{
    printf("---Test of HMAC_MD5 start---\n");
    for(int i = 0; i < 6; i++)
    {
        byte *key = HMAC_MD5_cases_keys[i];
        byte *data = HMAC_MD5_cases_data[i];
        byte *expeted = HMAC_MD5_expected[i];
        byte *myResult = HMAC_MD5(key, data);
        printf("Expected : %s\n", expeted);
        printf("Got      : %s\n", myResult);
        if(strcmp(myResult, expeted) == 0)
            printf("Asserted equal!\n");
        else
            printf("Not equal!\n");
        printf("\n");
    }
    printf("---Test of HMAC_MD5 end---\n");
}

int main()
{
    test_MD5();
    test_HMAC_MD5();
}