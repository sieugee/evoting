#ifndef FILE_UTILS
#define FILE_UTILS

#include <iostream>
#include <fstream>
#include <limits>
#include <unordered_map>

#include "miracllib/ecn.h"
#include "file_name.hpp"
#include "utils.hpp"

using namespace std;

int readCurveConfig(Big* a, Big* b, Big* p)
{
    ifstream config_curve;
    char* tmp_str = new char[128];

    config_curve.open(CONFIG_CURVE, ios::in|ios::binary);
    if ( !config_curve )
    {
        cout << "Cannot read config curve\n";
        return -1;
    }
    config_curve.getline(tmp_str, 128);
    *a = Big(tmp_str); /// first line
    config_curve.getline(tmp_str, 128);
    *b = Big(tmp_str); /// second line
    config_curve.getline(tmp_str, 128);
    *p = Big(tmp_str); /// third line
    delete[] tmp_str;
    config_curve.close();
    return 0;
}

int readGeneratorPointConfig(Big* x_value, Big* y_value)
{
    ifstream config_curve;
    char* tmp_str = new char[128];
    int skip_lines = 4;

    config_curve.open(CONFIG_CURVE, ios::in|ios::binary);
    if ( !config_curve )
    {
        cout << "Cannot read config curve\n";
        return -1;
    }


    /// Skip lines
    for ( int i = 0; i < skip_lines; i++ )
    {
        config_curve.ignore(numeric_limits<streamsize>::max(),'\n');
    }

    config_curve.getline(tmp_str, 128);
    *x_value = Big(tmp_str);
    config_curve.getline(tmp_str, 128);
    *y_value = Big(tmp_str);
    config_curve.getline(tmp_str, 128);

    delete[] tmp_str;
    config_curve.close();

    return 0;
}

 unordered_map<string, string> readVoteCombinationHashTable(string filename)
 {
    unordered_map<string, string> combi_hash_table;
    ifstream infile;
    infile.open(filename, ios::in|ios::binary);
    if (!infile)
    {
        cout << "No prepared hash file\n";
        return combi_hash_table;
    }
    infile.ignore(numeric_limits<streamsize>::max(), '\n'); /// Skip the first line
    while ( infile )
    {
        char* tmp_line_x = new char[128];
        char* tmp_line_y = new char[128];
        infile.getline(tmp_line_x, 128);
        infile.getline(tmp_line_y, 128);
        string vote_result_id = getECPointIdString(tmp_line_x, tmp_line_y);
        infile.getline(tmp_line_x, 128);
        combi_hash_table.insert(make_pair(vote_result_id, tmp_line_x));
    }
    infile.close();

    return combi_hash_table;
 }

 #endif /// FILE_UTILS
