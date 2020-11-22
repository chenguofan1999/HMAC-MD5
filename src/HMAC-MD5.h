#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// byte 字节 : 8位
// word 字   : 32位
typedef unsigned char byte;
typedef unsigned word;

// S[i][j] 表示在第 i 轮中，第 j 次运算中循环左移的位数
word S[4][16] = {
    { 7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22 },
    { 5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20 },
    { 4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23 },
    { 6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21 }};

// T 中的数据可由 sin 函数计算出
// T[i][j] 表示在第 i 轮中，第 j 次运算中使用的数值
word T[4][16] = {
    {
        0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 
        0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501, 
        0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
        0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821
    },{
        0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
        0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
        0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
        0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a
    },{
        0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
        0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
        0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
        0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665
    },{
        0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
        0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
        0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
        0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
    }};

// X 用于在处理每一个块时暂存这个块的 16 个 word
word X[16];

// K[i][j] 表示在第 i 轮中，第 j 次运算中 X 的下标
word K[4][16] = {
    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
    { 1, 6, 11, 0, 5, 10, 15, 4, 9, 14, 3, 8, 13, 2, 7, 12 },
    { 5, 8, 11, 14, 1, 4, 7, 10, 13, 0, 3, 6, 9, 12, 15, 2 }, 
    { 0, 7, 14, 5, 12, 3, 10, 1, 8, 15, 6, 13, 4, 11, 2, 9 }};

word F(word x, word y, word z){ return (x & y) | ((~x) & z); }
word G(word x, word y, word z){ return (x & z) | (y & (~z)); }
word H(word x, word y, word z){ return x ^ y ^ z; }
word I(word x, word y, word z){ return y ^ (x | (~z)); }

word leftRotate(word r, word N)
{
	unsigned  mask = (1 << N) - 1;
	return ((r >> (32 - N)) & mask) | ((r << N) & ~mask);
}

void round1(word *a, word b, word c, word d, int i)
{ *a = b + leftRotate((*a + F(b,c,d) + X[K[0][i]] + T[0][i]), S[0][i]);} 

void round2(word *a, word b, word c, word d, int i)
{ *a = b + leftRotate((*a + G(b,c,d) + X[K[1][i]] + T[1][i]), S[1][i]);} 

void round3(word *a, word b, word c, word d, int i)
{ *a = b + leftRotate((*a + H(b,c,d) + X[K[2][i]] + T[2][i]), S[2][i]);} 

void round4(word *a, word b, word c, word d, int i)
{ *a = b + leftRotate((*a + I(b,c,d) + X[K[3][i]] + T[3][i]), S[3][i]);} 




/**
 * Input 
 * - msg   : 原始的信息  
 * - n     : 信息的字节长度
 * Output
 * - words : 填充至 64 字节整数倍大小的字的数组
 * - N     : words 的长度（单位：字）
 */ 
void msgToWords(byte *msg, int n, word **words, int *N)
{
    /* 填充数据至块大小的整数倍 */
    int paddedBlockNumber = 1 + (n + 8) / 64;
    int paddedByteLen = paddedBlockNumber * 64; 
    byte *paddedMsg = (byte *)malloc(paddedByteLen);

    for(int i = 0; i < n; i++) paddedMsg[i] = msg[i];
    paddedMsg[n] = (byte)0x80;
    for(int i = n + 1; i < paddedByteLen - 8; i++) paddedMsg[i] = 0;

    /* 将原始数据长度填充至最后两个字 */
    // 直接 memcpy 可一定程度上避免大小端问题
    unsigned long long originLenInBits = (unsigned long long)n * 8; 
    memcpy(paddedMsg + paddedByteLen - 8, &originLenInBits, 8);
    
    /* 从字节(byte)的数组转换为字(word)的数组 */
    // 不进行这一转换可避免大小段问题
    int wordsLen = paddedByteLen / 4;
    *N = wordsLen;
    *words = (word *)malloc(paddedByteLen);
    for(int i = 0; i < paddedByteLen; i += 4)
    {
        word thisWord = 0;
        for(int j = i; j <= i + 3; j++)
        {
            // shiftBits 的计算体现小端模式
            int shiftBits = (j - i) * 8;
            thisWord += ((word)paddedMsg[j]) << shiftBits;
        }
        (*words)[i/4] = thisWord;
    }

    free(paddedMsg);
}

/** 
 * processInBlocks 接收转换后的字的数组 words 和元素个数 N,
 * 经过每块 4 轮计算后，结果被写入传入的 4 个寄存器
 */ 
void processInBlocks(word *A, word *B, word *C, word *D, word *words, int N)
{
    // initialize   
    *A = 0x67452301;
    *B = 0xefcdab89;
    *C = 0x98badcfe;
    *D = 0x10325476;

    /* process each 16-word block */ 
    for(int i = 0; i < N / 16; i++)
    {
        for(int j = 0; j < 16; j++) X[j] = words[16 * i + j];

        word a = *A, b = *B, c = *C, d = *D;

        // Round 1 
        round1(&a, b, c, d, 0); 
        round1(&d, a, b, c, 1);
        round1(&c, d, a, b, 2);
        round1(&b, c, d, a, 3);
        round1(&a, b, c, d, 4);
        round1(&d, a, b, c, 5);
        round1(&c, d, a, b, 6);
        round1(&b, c, d, a, 7);
        round1(&a, b, c, d, 8);
        round1(&d, a, b, c, 9);
        round1(&c, d, a, b, 10);
        round1(&b, c, d, a, 11);
        round1(&a, b, c, d, 12);
        round1(&d, a, b, c, 13);
        round1(&c, d, a, b, 14);
        round1(&b, c, d, a, 15);

        //Round 2
        round2(&a, b, c, d, 0);
        round2(&d, a, b, c, 1);
        round2(&c, d, a, b, 2);
        round2(&b, c, d, a, 3);
        round2(&a, b, c, d, 4);
        round2(&d, a, b, c, 5);
        round2(&c, d, a, b, 6);
        round2(&b, c, d, a, 7);
        round2(&a, b, c, d, 8);
        round2(&d, a, b, c, 9);
        round2(&c, d, a, b, 10);
        round2(&b, c, d, a, 11);
        round2(&a, b, c, d, 12);
        round2(&d, a, b, c, 13);
        round2(&c, d, a, b, 14);
        round2(&b, c, d, a, 15);

        //Round 3
        round3(&a, b, c, d, 0);
        round3(&d, a, b, c, 1);
        round3(&c, d, a, b, 2);
        round3(&b, c, d, a, 3);
        round3(&a, b, c, d, 4);
        round3(&d, a, b, c, 5);
        round3(&c, d, a, b, 6);
        round3(&b, c, d, a, 7);
        round3(&a, b, c, d, 8);
        round3(&d, a, b, c, 9);
        round3(&c, d, a, b, 10);
        round3(&b, c, d, a, 11);
        round3(&a, b, c, d, 12);
        round3(&d, a, b, c, 13);
        round3(&c, d, a, b, 14);
        round3(&b, c, d, a, 15);

        //Round 4
        round4(&a, b, c, d, 0);
        round4(&d, a, b, c, 1);
        round4(&c, d, a, b, 2);
        round4(&b, c, d, a, 3);
        round4(&a, b, c, d, 4);
        round4(&d, a, b, c, 5);
        round4(&c, d, a, b, 6);
        round4(&b, c, d, a, 7);
        round4(&a, b, c, d, 8);
        round4(&d, a, b, c, 9);
        round4(&c, d, a, b, 10);
        round4(&b, c, d, a, 11);
        round4(&a, b, c, d, 12);
        round4(&d, a, b, c, 13);
        round4(&c, d, a, b, 14);
        round4(&b, c, d, a, 15);
    
        *A += a;
        *B += b;
        *C += c;
        *D += d;
    }
}

/**
 * 此函数是 msgToWords 与 processInBlocks 的综合
 * 输入需要加密的字符串 msg
 * 输出 msg 的摘要的十六进制形式的字符串
 */
byte* MD5(byte *msg)
{
    word *wordBuffer;
    int wordLen;
    msgToWords(msg, strlen(msg), &wordBuffer, &wordLen);

    word A,B,C,D;
    processInBlocks(&A, &B, &C, &D, wordBuffer, wordLen);

    byte *ans = (byte *)malloc(33);
    ans[0] = '\0';

    byte *a = (byte *)&A, *b = (byte *)&B, 
         *c = (byte *)&C, *d = (byte *)&D;

    byte *regs[] = {a,b,c,d};
    for(int i = 0; i < 4; i++)
    {
        byte s[9];
		sprintf(s, "%02x%02x%02x%02x\0", regs[i][0], regs[i][1], regs[i][2], regs[i][3]);
        strcat(ans, s);
    }

    free(wordBuffer);
    return ans;
}

/**
 * 此函数亦是 msgToWords 与 processInBlocks 的综合
 * 输入需要加密的字符串 msg
 * 输出 msg 的摘要的原始字节流
 * (一般不要用此函数)
 */
byte* MD_5(byte *msg)
{
    word *wordBuffer;
    int wordLen;
    msgToWords(msg, strlen(msg), &wordBuffer, &wordLen);

    word A,B,C,D;
    processInBlocks(&A, &B, &C, &D, wordBuffer, wordLen);

    byte *ans = (byte *)malloc(33);
    ans[0] = '\0';

    byte *a = (byte *)&A, *b = (byte *)&B, 
         *c = (byte *)&C, *d = (byte *)&D;

    byte *regs[] = {a,b,c,d};
    for(int i = 0; i < 4; i++)
    {
        byte s[5];
		sprintf(s, "%c%c%c%c\0", regs[i][0], regs[i][1], regs[i][2], regs[i][3]);
        strcat(ans, s);
    }

    free(wordBuffer);
    return ans;
}

/**
 * 使用 MD5 作为散列函数的 HMAC 算法
 * input
 * - key : 加密密钥 (字符串)
 * - msg : 需要认证的消息
 * output
 * - 返回数据摘要的十六进制形式的字符串
 */
byte* HMAC_MD5(byte *key, byte *msg)
{
    byte *_key = strlen(key) > 64 ? MD_5(key) : key;

    if(strlen(_key) < 64)
    {
        // resize to blockSize
        byte tKey[65];
        for(int i = 0; i < strlen(_key); i++) tKey[i] = _key[i];
        for(int i = strlen(_key); i < 65; i++) tKey[i] = 0x00;
        tKey[64] = '\0';
        _key = tKey;
    }

    /*计算 ipad*/
    byte ipad[65];
    for(int i = 0; i < 64; i++) ipad[i] = 0x36 ^ _key[i];
    ipad[64] = '\0';

    /*计算 opad*/   
    byte opad[65];
    for(int i = 0; i < 64; i++) opad[i] = 0x5c ^ _key[i];
    opad[64] = '\0';

    /*result = MD5(opad ∥ MD5(ipad ∥ message))*/

    // 1. t1 = MD5(ipad ∥ message)
    int len1 = 64 + strlen(msg);
    byte *t1 = (byte *)malloc(len1 + 1);
    t1[0] = '\0';
    strcat(t1, ipad);
    strcat(t1, msg);
    t1 = MD_5(t1);

    // 2. t2 = opad ∥ t1
    byte t2[81];
    t2[0] = '\0';
    strcat(t2, opad);
    strcat(t2, t1);

    // 3. ans = MD5(t2)
    byte *ans = MD5(t2);
    
    // 别忘了释放空间
    free(t1);

    return ans;
}