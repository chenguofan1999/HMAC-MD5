#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char byte;
typedef unsigned word;

/**
 * Input 
 * - msg : original message  
 * - n   : length of msg
 * Output
 * - words : padded and converted words
 * - N : length of words
 */ 
void msgToWords(byte *msg, int n, word **words, int *N)
{
    // pad the msg
    int paddedBlockNumber = 1 + (n + 8) / 64;
    int paddedByteLen = paddedBlockNumber * 64; 
    int totalSize = paddedByteLen * 8;
    byte *paddedMsg = (byte *)malloc(totalSize);

    for(int i = 0; i < n; i++) paddedMsg[i] = msg[i];
    paddedMsg[n] = (byte)0x80;
    for(int i = n + 1; i < paddedByteLen - 8; i++) paddedMsg[i] = 0;

    // pad originalSize in the last 2 words
    unsigned long long originLenInBits = (unsigned long long)n * 8; 
    memcpy(paddedMsg + paddedByteLen - 8, &originLenInBits, 8);
    
    // convert from bytes to words
    int wordsLen = paddedByteLen / 4;
    *N = wordsLen;
    *words = (word *)malloc(totalSize);
    for(int i = 0; i < paddedByteLen; i += 4)
    {
        word thisWord = 0;
        for(int j = i; j <= i + 3; j++)
        {
            int shiftBits = (j - i) * 8;
            thisWord += ((word)paddedMsg[j]) << shiftBits;
        }
        (*words)[i/4] = thisWord;
    }

    // (*words)[wordsLen - 2] = (word)(originLenInBits >> 32);
    // (*words)[wordsLen - 1] = (word)(originLenInBits);

    free(paddedMsg);
}


//////////////////////////////////////////////////////////////////

word S[4][16] = {
    { 7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22 },
    { 5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20 },
    { 4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23 },
    { 6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21 }};

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

word X[16];
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


// void round1(word *a, word b, word c, word d, word k, word s, word i)
// { *a = b + leftRotate((*a + F(b,c,d) + X[K[0][k]] + T[0][i]), S[0][s]);} 

void round1(word *a, word b, word c, word d, word k, word s, word i)
{ 
    //*a = b + leftRotate((*a + F(b,c,d) + X[K[0][k]] + T[0][i]), S[0][s]);

    *a += F(b,c,d) + X[K[0][k]] + T[0][i];
    *a = leftRotate(*a, S[0][s]);

    printf("->%lx\n", *a);

    *a += b;

    printf("->%lx\n", *a);
} 

void round2(word *a, word b, word c, word d, word k, word s, word i)
{ *a = b + leftRotate((*a + G(b,c,d) + X[K[1][k]] + T[1][i]), S[1][s]);} 

void round3(word *a, word b, word c, word d, word k, word s, word i)
{ *a = b + leftRotate((*a + H(b,c,d) + X[K[2][k]] + T[2][i]), S[2][s]);} 

void round4(word *a, word b, word c, word d, word k, word s, word i)
{ *a = b + leftRotate((*a + I(b,c,d) + X[K[3][k]] + T[3][i]), S[3][s]);} 


/** MD5 receives registers and padded words, calculates the outputs in registers. 
 * A,B,C,D : 4 registers
 * words : padded message
 * N : length of words, a multiple of 16 (16 words for a block)
 */ 
void MD5(word *A, word *B, word *C, word *D, word *words, int N)
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

        printf("Before round 1.0:\n"); // before start
        printf("   a : %lx\n   b : %lx\n   c : %lx\n   d : %lx\n",a,b,c,d);

        // Round 1 
        round1(&a, b, c, d, 0, 0, 0); // 1.1
        printf("After round 1.1:\n");
        printf("   a : %lx\n   b : %lx\n   c : %lx\n   d : %lx\n",a,b,c,d);

        round1(&d, a, b, c, 1, 1, 1); //

        printf("After round 1.2:\n");
        printf("   a : %lx\n   b : %lx\n   c : %lx\n   d : %lx\n",a,b,c,d);

        round1(&c, d, a, b, 2, 2, 2);
        round1(&b, c, d, a, 3, 3, 3);
        round1(&a, b, c, d, 4, 4, 4);
        round1(&d, a, b, c, 5, 5, 5);

        printf("After round 1.6:\n");
        printf("   a : %lx\n   b : %lx\n   c : %lx\n   d : %lx\n",a,b,c,d);


        round1(&c, d, a, b, 6, 6, 6);
        round1(&b, c, d, a, 7, 7, 7);
        round1(&a, b, c, d, 8, 8, 8);
        round1(&d, a, b, c, 9, 9, 9);
        round1(&c, d, a, b, 10, 10, 10);
        round1(&b, c, d, a, 11, 11, 11);
        round1(&a, b, c, d, 12, 12, 12);
        round1(&d, a, b, c, 13, 13, 13);
        round1(&c, d, a, b, 14, 14, 14);
        round1(&b, c, d, a, 15, 15, 15);

        printf("After round 1:\n");
        printf("   a : %lx\n   b : %lx\n   c : %lx\n   d : %lx\n",a,b,c,d);

        //Round 2
        round2(&a, b, c, d, 0, 0, 0);
        round2(&d, a, b, c, 1, 1, 1);
        round2(&c, d, a, b, 2, 2, 2);
        round2(&b, c, d, a, 3, 3, 3);
        round2(&a, b, c, d, 4, 4, 4);
        round2(&d, a, b, c, 5, 5, 5);
        round2(&c, d, a, b, 6, 6, 6);
        round2(&b, c, d, a, 7, 7, 7);
        round2(&a, b, c, d, 8, 8, 8);
        round2(&d, a, b, c, 9, 9, 9);
        round2(&c, d, a, b, 10, 10, 10);
        round2(&b, c, d, a, 11, 11, 11);
        round2(&a, b, c, d, 12, 12, 12);
        round2(&d, a, b, c, 13, 13, 13);
        round2(&c, d, a, b, 14, 14, 14);
        round2(&b, c, d, a, 15, 15, 15);

        printf("After round 2:\n");
        printf("   a : %lx\n   b : %lx\n   c : %lx\n   d : %lx\n",a,b,c,d);

        //Round 3
        round3(&a, b, c, d, 0, 0, 0);
        round3(&d, a, b, c, 1, 1, 1);
        round3(&c, d, a, b, 2, 2, 2);
        round3(&b, c, d, a, 3, 3, 3);
        round3(&a, b, c, d, 4, 4, 4);
        round3(&d, a, b, c, 5, 5, 5);
        round3(&c, d, a, b, 6, 6, 6);
        round3(&b, c, d, a, 7, 7, 7);
        round3(&a, b, c, d, 8, 8, 8);
        round3(&d, a, b, c, 9, 9, 9);
        round3(&c, d, a, b, 10, 10, 10);
        round3(&b, c, d, a, 11, 11, 11);
        round3(&a, b, c, d, 12, 12, 12);
        round3(&d, a, b, c, 13, 13, 13);
        round3(&c, d, a, b, 14, 14, 14);
        round3(&b, c, d, a, 15, 15, 15);

        printf("After round 3:\n");
        printf("   a : %lx\n   b : %lx\n   c : %lx\n   d : %lx\n",a,b,c,d);

        //Round 4
        round4(&a, b, c, d, 0, 0, 0);
        round4(&d, a, b, c, 1, 1, 1);
        round4(&c, d, a, b, 2, 2, 2);
        round4(&b, c, d, a, 3, 3, 3);
        round4(&a, b, c, d, 4, 4, 4);
        round4(&d, a, b, c, 5, 5, 5);
        round4(&c, d, a, b, 6, 6, 6);
        round4(&b, c, d, a, 7, 7, 7);
        round4(&a, b, c, d, 8, 8, 8);
        round4(&d, a, b, c, 9, 9, 9);
        round4(&c, d, a, b, 10, 10, 10);
        round4(&b, c, d, a, 11, 11, 11);
        round4(&a, b, c, d, 12, 12, 12);
        round4(&d, a, b, c, 13, 13, 13);
        round4(&c, d, a, b, 14, 14, 14);
        round4(&b, c, d, a, 15, 15, 15);
    
        printf("After round 4:\n");
        printf("   a : %lx\n   b : %lx\n   c : %lx\n   d : %lx\n",a,b,c,d);

        *A += a;
        *B += b;
        *C += c;
        *D += d;
    }
}

int main()
{
    word *wordBuffer;
    int wordLen;

    //byte *msg = "jdfgsdhfsdfsd 156445dsfsd7fg/*/+bfjsdgf%$^";
    byte *msg = "ABCDE";
    msgToWords(msg, strlen(msg), &wordBuffer, &wordLen);

    // for(int i = 0; i < wordLen; i++)
    //     printf("at %2d : %lx\n", i, wordBuffer[i]);

    word A,B,C,D;
    MD5(&A, &B, &C, &D, wordBuffer, wordLen);
    printf("%lx%lx%lx%lx\n",A,B,C,D);

    byte *a = &A, *b = &B, *c = &C, *d = &D;
    byte *registers[] = {a,b,c,d};
    for(int i = 0; i < 4; i++)
    {
        for(int j = 0; j < 4; j++) 
            printf("%02lx",registers[i][j]);  
    }
    printf("\n");
    //free(byteBuffer);
    free(wordBuffer);
}