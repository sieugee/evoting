#include "file_utils.hpp"
#include "evoting.hpp"

int main(int argc,char **argv)
{
    int st;
    Big a, b, p;
    Big private_key = Big(9876543210123);
    uint32_t number_of_candidates, number_of_votes;

    st = readCurveConfig(&a, &b, &p);
    if ( 0!= st )
    {
        return st;
    }

    Big tmp_x, tmp_y;
    st = readGeneratorPointConfig(&tmp_x, &tmp_y);
    if ( 0!= st )
    {
        return st;
    }

    ECCCurve e(a, b, p);
    ECn generator_point(tmp_x, tmp_y);
    ECCElGamal scheme(e, generator_point, private_key);

    EVoting::readVoteConfig(&number_of_candidates, &number_of_votes);
    EVoting this_vote(scheme, number_of_candidates, number_of_votes);

    cout << "----\nDecrypted vote result in EC point format:\n";
    this_vote.countVotes();

    /// Read the combination hash and put it in the hash table
    unordered_map<string, string> combi_hash_table = readVoteCombinationHashTable(COMBINATION_HASH);

    /// Read the combination array file and check for the vote result
    uint32_t list_of_vote_for_candidate[number_of_candidates] = {0};
    ifstream infile;
    char* tmp_line = new char[128];
    infile.open(COMBINATION_ARRAY, ios::in|ios::binary);
    if (!infile)
    {
        cout << "No prepared hash file\n";
        return -1;
    }

    infile.ignore(numeric_limits<streamsize>::max(), '\n'); /// Skip the first line
    while ( infile )
    {
        infile.getline(tmp_line, 128);
        tmp_x = Big(tmp_line);
        infile.getline(tmp_line, 128);
        tmp_y = Big(tmp_line);
        ECn arr_vote_result = ECn(tmp_x, tmp_y);

        ECn checked_hash_vote_result = this_vote.getVoteResult();
        checked_hash_vote_result -= arr_vote_result;
        checked_hash_vote_result.get(tmp_x, tmp_y);
        char str_x[100], str_y[100];
        str_x << tmp_x;
        str_y << tmp_y;
        string vote_result_id = getECPointIdString(str_x, str_y);

        infile.getline(tmp_line, 128);

        auto search = combi_hash_table.find(vote_result_id);
        if ( search != combi_hash_table.end() )
        {
            string result;
            result.append(search->second);
            result.append(tmp_line);
            stringToUInt32Array(result, number_of_candidates, list_of_vote_for_candidate);
            break;
        }
        
    }
    infile.close();
    
    cout << "-----------------\n";
    cout << "List of extracted votes from EC point:\n";
    for ( int i = 0; i < number_of_candidates; i++ )
    {
        cout << "Candidate #" << i+1 << ": " << list_of_vote_for_candidate[i] << " votes\n";
    }
 
    return 0;

}
