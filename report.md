# HMAC-MD5 设计文档

## MD5 原理概述

MD5 是一个种密码散列函数，输入一个任意长度的信息，输出固定长度128-bit的消息摘要。

**MD5运算流程**

1. 通过在消息后附加值的方式，将原始消息长度 (单位：bit) 补充至与 448 模 512 同余。
    1. 即使原本已模 512 得 448，仍要增补(此时需要增补 512 位)。
    2. 增补的目的是使消息长度最终成为 512-bit 的整数倍留下了两个字 (word, 1 word = 32 bit) 的长度，将在下一步被使用。
    3. 这表示了 MD5 算法中的块大小是 512-bit ，即 64 bytes、16 words. 后面的运算将会以块为单位进行。
    4. 增补的方式：附加的第一位是 1，其后全部是 0。
    5. 注意 X86 的数据存放模式为小端模式，如果要手动在数组中实现这一增补的过程，请将增补的低位放在低位地址。例如当往字节数组 C[0..2] 中增补 0x10 00 00 时，应该把处于高位的 0x10 放在 C[2] 而不是直观上认为的 C[0]。
2. 在消息后增补原消息的长度(以 bit 为单位，例如消息 "abcd"，长度为 4 * 8 = 32 = 0x20 需要在后面增补 0x00 00 00 00 00 00 00 20). 
    1. 同样的，注意小端模式。
    2. 通过 memcpy 等方式直接复制数据可避免大小端问题。
3. 初始化 4 个大小为一个字的寄存器，这 4 个寄存器的值将会进行多轮(轮数等于块数 x 4)的计算，最终将 4个寄存器的值连接起来即为得到的 128 位的信息摘要.
    ```
    A := 0x67452301
    B := 0xEFCDAB89
    C := 0x98BADCFE
    D := 0x10325476
    ```
4. 对每一个块，进行 4 轮计算，每轮包含 16 次计算。以下参考了 [RFC-1321](https://tools.ietf.org/html/rfc1321) 文档中描述的此过程的伪代码，做了一定修改。其中 M 为增补后的消息，以字为单位。
    ```c
    /* Define leftrotate */
    x <<< n = (x << n) | (x >> (32-n))

    /* Define 4 functions */
    F(X,Y,Z) = XY v not(X) Z
    G(X,Y,Z) = XZ v Y not(Z)
    H(X,Y,Z) = X xor Y xor Z
    I(X,Y,Z) = Y xor (X v not(Z))

    /* Define T as the following precomputed table */
    T[ 0.. 3] := { 0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee }
    T[ 4.. 7] := { 0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501 }
    T[ 8..11] := { 0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be }
    T[12..15] := { 0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821 }
    T[16..19] := { 0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa }
    T[20..23] := { 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8 }
    T[24..27] := { 0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed }
    T[28..31] := { 0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a }
    T[32..35] := { 0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c }
    T[36..39] := { 0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70 }
    T[40..43] := { 0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05 }
    T[44..47] := { 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665 }
    T[48..51] := { 0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039 }
    T[52..55] := { 0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1 }
    T[56..59] := { 0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1 }
    T[60..63] := { 0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391 }

    /* Process each 16-word block. */
    For i = 0 to N/16-1 do

        /* Copy block i into X. */
        For j = 0 to 15 do
        Set X[j] to M[i*16+j].
        end /* of loop on j */

        /* Save A as AA, B as BB, C as CC, and D as DD. */
        AA = A
        BB = B
        CC = C
        DD = D

        /* Round 1. */
        /* Let [abcd k s i] denote the operation
            a = b + ((a + F(b,c,d) + X[k] + T[i]) <<< s). */
        /* Do the following 16 operations. */
        [ABCD  0  7  1]  [DABC  1 12  2]  [CDAB  2 17  3]  [BCDA  3 22  4]
        [ABCD  4  7  5]  [DABC  5 12  6]  [CDAB  6 17  7]  [BCDA  7 22  8]
        [ABCD  8  7  9]  [DABC  9 12 10]  [CDAB 10 17 11]  [BCDA 11 22 12]
        [ABCD 12  7 13]  [DABC 13 12 14]  [CDAB 14 17 15]  [BCDA 15 22 16]

        /* Round 2. */
        /* Let [abcd k s i] denote the operation
            a = b + ((a + G(b,c,d) + X[k] + T[i]) <<< s). */
        /* Do the following 16 operations. */
        [ABCD  1  5 17]  [DABC  6  9 18]  [CDAB 11 14 19]  [BCDA  0 20 20]
        [ABCD  5  5 21]  [DABC 10  9 22]  [CDAB 15 14 23]  [BCDA  4 20 24]
        [ABCD  9  5 25]  [DABC 14  9 26]  [CDAB  3 14 27]  [BCDA  8 20 28]
        [ABCD 13  5 29]  [DABC  2  9 30]  [CDAB  7 14 31]  [BCDA 12 20 32]

        /* Round 3. */
        /* Let [abcd k s t] denote the operation
            a = b + ((a + H(b,c,d) + X[k] + T[i]) <<< s). */
        /* Do the following 16 operations. */
        [ABCD  5  4 33]  [DABC  8 11 34]  [CDAB 11 16 35]  [BCDA 14 23 36]
        [ABCD  1  4 37]  [DABC  4 11 38]  [CDAB  7 16 39]  [BCDA 10 23 40]
        [ABCD 13  4 41]  [DABC  0 11 42]  [CDAB  3 16 43]  [BCDA  6 23 44]
        [ABCD  9  4 45]  [DABC 12 11 46]  [CDAB 15 16 47]  [BCDA  2 23 48]

        /* Round 4. */
        /* Let [abcd k s t] denote the operation
            a = b + ((a + I(b,c,d) + X[k] + T[i]) <<< s). */
        /* Do the following 16 operations. */
        [ABCD  0  6 49]  [DABC  7 10 50]  [CDAB 14 15 51]  [BCDA  5 21 52]
        [ABCD 12  6 53]  [DABC  3 10 54]  [CDAB 10 15 55]  [BCDA  1 21 56]
        [ABCD  8  6 57]  [DABC 15 10 58]  [CDAB  6 15 59]  [BCDA 13 21 60]
        [ABCD  4  6 61]  [DABC 11 10 62]  [CDAB  2 15 63]  [BCDA  9 21 64]

        /* Then perform the following additions. (That is increment each
        of the four registers by the value it had before this block
        was started.) */
        A = A + AA
        B = B + BB
        C = C + CC
        D = D + DD

    end /* of loop on i */
    ```
5. 输出的消息摘要即为最终的 `A append B append C append D`
    - 转换为16进制字符串时注意 X86 的小端模式。例如将 32 位的寄存器 A 视为有 4 个元素的字节数组，应该按A[3]、A[2]、A[1]、A[0] 的顺序输出。

## HMAC 原理概述

HMAC 是基于密码散列函数的消息认证码( MAC )。例如 HMAC-MD5 是使用 MD-5 作为散列函数的消息认证码。

使用上，HMAC-MD5 比 MD5 多需要一个密钥 key，经过简单的计算得到 128-bit 的摘要。

**伪代码**
```c
function hmac is
    input:
        key:        Bytes    // Array of bytes
        message:    Bytes    // Array of bytes to be hashed
        hash:       Function // The hash function to use (e.g. SHA-1)
        blockSize:  Integer  // The block size of the hash function (e.g. 64 bytes for SHA-1)
        outputSize: Integer  // The output size of the hash function (e.g. 20 bytes for SHA-1)
 
    // Keys longer than blockSize are shortened by hashing them
    if (length(key) > blockSize) then
        key ← hash(key) // key is outputSize bytes long

    // Keys shorter than blockSize are padded to blockSize by padding with zeros on the right
    if (length(key) < blockSize) then
        key ← Pad(key, blockSize) // Pad key with zeros to make it blockSize bytes long

    o_key_pad ← key xor [0x5c * blockSize]   // Outer padded key
    i_key_pad ← key xor [0x36 * blockSize]   // Inner padded key

    return hash(o_key_pad ∥ hash(i_key_pad ∥ message))
```

## 数据结构分析

- 此项目中没有复杂的数据结构，实现的关键在于理解基本数据类型的内部表示。

- 程序中直观地定义了两种数据类型：

```c
// byte 字节 : 8位
// word 字   : 32位
typedef unsigned char byte;
typedef unsigned word;
```

- 程序中需要用到的表格数据以全局变量的方式直接给出。

## 总体架构设计

为方便使用，源代码全部在一个.h文件中( [HMAC-MD5.H](src/HMAC-MD5.H) )。非强制性地为使用者推荐了两个接口，即两个函数:

```c
byte* MD5(byte *msg)
byte* HMAC_MD5(byte *key, byte *msg)
```

这两个函数接收的参数均为字符串表示的消息或密钥，例如 *"abc"* 。返回的均为十六进制字符串表示的信息摘要，例如 *"93ec1c6a0bc2889f1e87da3f88f0fca6"*。


## 模块分解

- MD5
    - 数据预处理模块
        - `void msgToWords(byte *msg, int n, word **words, int *N)`
    - 计算模块
        - `void processInBlocks(word *A, word *B, word *C, word *D, word *words, int N)`
        - 辅助函数
            - `word F(word x, word y, word z)`
            - `word G(word x, word y, word z)`
            - `word H(word x, word y, word z)`
            - `word I(word x, word y, word z)`
            - `word leftRotate(word r, word N)`
            - `void round1(word *a, word b, word c, word d, word k, word s, word i)`
            - `void round2(word *a, word b, word c, word d, word k, word s, word i)`
            - `void round3(word *a, word b, word c, word d, word k, word s, word i)`
            - `void round4(word *a, word b, word c, word d, word k, word s, word i)`
        - 依赖的数据
            - `word S[4][16]`
            - `word T[4][16]`
            - `word K[4][16]`
            - `word X[16]`
- HMAC-MD5
    - 只有一个模块、一个函数:`byte* HMAC_MD5(byte *key, byte *msg)`

## 运行与测试

写了一个测试文件 `test.c`，其中包含了来自维基百科、两个算法的官方文档等的测例。

**test.c**

```c
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
```

**运行结果**

```sh
---Test of MD5 start---
Expected  : d41d8cd98f00b204e9800998ecf8427e
Got       : d41d8cd98f00b204e9800998ecf8427e
Asserted equal!

Expected  : 9e107d9d372bb6826bd81d3542a419d6
Got       : 9e107d9d372bb6826bd81d3542a419d6
Asserted equal!

Expected  : 0cc175b9c0f1b6a831c399e269772661
Got       : 0cc175b9c0f1b6a831c399e269772661
Asserted equal!

Expected  : 900150983cd24fb0d6963f7d28e17f72
Got       : 900150983cd24fb0d6963f7d28e17f72
Asserted equal!

Expected  : f96b697d7cb7938d525a2f31aaf161d0
Got       : f96b697d7cb7938d525a2f31aaf161d0
Asserted equal!

Expected  : c3fcd3d76192e4007dfb496cca67e13b
Got       : c3fcd3d76192e4007dfb496cca67e13b
Asserted equal!

Expected  : d174ab98d277d9f5a5611c2c9f419d9f
Got       : d174ab98d277d9f5a5611c2c9f419d9f
Asserted equal!

Expected  : 57edf4a22be3c955ac49da2e2107b67a
Got       : 57edf4a22be3c955ac49da2e2107b67a
Asserted equal!

---Test of MD5 end---

---Test of HMAC_MD5 start---
Expected : eb01ff92f00d651abcdd1f56f1a74725
Got      : eb01ff92f00d651abcdd1f56f1a74725
Asserted equal!

Expected : 80070713463e7749b90c2dc24911e275
Got      : 80070713463e7749b90c2dc24911e275
Asserted equal!

Expected : 93ec1c6a0bc2889f1e87da3f88f0fca6
Got      : 93ec1c6a0bc2889f1e87da3f88f0fca6
Asserted equal!

Expected : 5d84b4bd8ce8797ffea3869fc0097e70
Got      : 5d84b4bd8ce8797ffea3869fc0097e70
Asserted equal!

Expected : 02021d12d54c48fcfaf832345ed8904d
Got      : 02021d12d54c48fcfaf832345ed8904d
Asserted equal!
```