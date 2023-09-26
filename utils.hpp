/// Util functions
#ifndef UTILS 
#define UTILS

#include <iostream>
#include <string>

size_t byteLengthToUInt64Length(size_t byte_len)
{
    size_t uint64_len = static_cast<size_t>(byte_len/8);
    if ( (uint64_len * 8) < byte_len )
    {
        uint64_len++;
    }
    return uint64_len;
}

/// @brief Get a id string for a EC point
/// @param str_x x value in char* type
/// @param str_y y value in char* type
/// @return Indentify string for the point
string getECPointIdString(char* str_x, char* str_y)
{
    string vote_result_id;
    char delimiter[2] = "|";
    vote_result_id.append(string(str_x));
    vote_result_id.append(string(delimiter));
    vote_result_id.append(string(str_y));
    return vote_result_id;
}

/// @brief Convert a string in format [a,b,c] to an array of uint32_t
/// @param input_str Input string
/// @param size Array size
/// @param list_ptr Pointer to output array
void stringToUInt32Array(string input_str, uint32_t size, uint32_t* list_ptr)
{
    char* number;
    char* str = strdup(input_str.c_str());
    int i = 0;

    number = strtok(str+1, " ,[]");
    while ( ( number != nullptr ) && (i< size) )
    {
        *(list_ptr+i) = stoull(number);
        number = strtok(NULL, " ,[]");
        i++;
    }
    free(str);
}

#endif // UTILS
