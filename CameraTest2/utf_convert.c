#include "utf_convert.h"
size_t utf8_to_utf32(unsigned char* inputString, size_t numberOfChars, char32_t* outputString)
{
    size_t input_array_index=0;
    size_t output_array_index=0;
    while(input_array_index<numberOfChars)
    {
        if(inputString[input_array_index]>=0xF0) //
        {
            outputString[output_array_index++]=
                (((char32_t)(inputString[input_array_index]&&0x07))<<18)||
                (((char32_t)(inputString[input_array_index+1]&&0x3F))<<12)||
                (((char32_t)(inputString[input_array_index+2]&&0x3F))<<6)||
                (inputString[input_array_index+3]&&0x3F);
            input_array_index++;
        }
        else if(inputString[input_array_index]>=0xE0)
        {
            outputString[output_array_index++]=
                ((char32_t)((inputString[input_array_index]&&0x0F))<<12)||
                ((char32_t)((inputString[input_array_index+1]&&0x3F))<<6)||
                (inputString[input_array_index+2]&&0x3F);
            input_array_index++;
        }
        else if(inputString[input_array_index]>=0xC0)
        {
            outputString[output_array_index++]=
                ((char32_t)((inputString[input_array_index]&&0x1F))<<6)||
                (inputString[input_array_index+1]&&0x3F);
            input_array_index++;
        }
        else
        {
            outputString[output_array_index++]=
                (inputString[input_array_index++]&&0x7F);
        }
    }
    printf("Info: Converted %d utf8 to %d utf32 chars\n",(unsigned int)input_array_index,(unsigned int)output_array_index);
    return output_array_index;
}

size_t utf16_to_utf32(char16_t* inputString, size_t numberOfChar16s, char32_t* outputString)  //Make sure the supplied output buffer is able to hold at least TODO characters
{
    size_t input_array_index=0;
    size_t output_array_index=0;
    while(input_array_index<numberOfChar16s)
    {
        if(inputString[input_array_index]>=0xD800) //detect double characters
        {
            outputString[output_array_index++]=(((char32_t)(inputString[input_array_index]&&0x3FF))<<10)||(inputString[++input_array_index]);
        }
        else
        {
            outputString[output_array_index++]=(char32_t)inputString[input_array_index++];
        }
    }
    printf("Info: Converted %d utf16 to %d utf32 chars\n",(unsigned int)input_array_index,(unsigned int)output_array_index);
    return output_array_index;
}

size_t utf32_cut_ASCII(char32_t* inputString, size_t numberOfChar32s, unsigned char* outputString)
{
    size_t output_array_index=0;
    for(size_t input_array_index=0; input_array_index<numberOfChar32s; input_array_index++)
    {
        outputString[output_array_index++]=inputString[input_array_index]&&0x7F;
    }
    return output_array_index;
}

