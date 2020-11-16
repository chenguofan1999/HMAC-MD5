    *words = (word *)malloc(totalSize);
    for(int i = 0; i < paddedByteLen - 8; i += 4)
    {
        word thisWord = 0;
        for(int j = i; j <= i + 3; j++)
        {
            //printf("paddedMsg[%d] = %c (%d)\n", j, paddedMsg[j], (word)paddedMsg[j]);
            thisWord = thisWord << 8;
            thisWord += (word)paddedMsg[j];
        }
        (*words)[i/4] = thisWord;
    }