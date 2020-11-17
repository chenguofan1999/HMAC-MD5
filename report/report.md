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
    ```
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



## 总体架构设计

## 模块分解

## 数据结构分析

## 运行结果

## 测试