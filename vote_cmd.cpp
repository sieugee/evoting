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

    /// Check if user has voted before
    string voted_user = string(argv[1]);
    ifstream voter_infile;
    voter_infile.open(VOTERS, ios::in|ios::binary);
    if ( voter_infile.is_open() )
    {
        while ( voter_infile )
        {
            char* tmp_voter_name = new char[128];
            voter_infile.getline(tmp_voter_name, 128);
            if ( voted_user == string(tmp_voter_name) )
            {
                voter_infile.close();
                cout << "You have voted before.\n";
                return 1;
            }
        }
        voter_infile.close();
    }

    /// Update voted users DB
    auto* console_buf = cout.rdbuf();
    ofstream voter_outfile;
    voter_outfile.open(VOTERS, ios::app|ios::binary);
    cout.rdbuf(voter_outfile.rdbuf());
    cout << voted_user << "\n";
    voter_outfile.close();
    cout.rdbuf(console_buf);


    uint32_t plain_votes[number_of_candidates] = {0};
    ECn* list_of_candidates = new ECn[number_of_candidates];
    ECCElGamalCipherText vote_result;

    this_vote.getCandidateList(list_of_candidates);

    uint32_t chosen_candidate = stoull(string(argv[2]));

    uint32_t number_of_invalid_votes = 0;
    VoteConfirmation confirmation_values[number_of_candidates];
    ECCElGamalCipherText tmp_vote = this_vote.voteForACandidate(chosen_candidate, list_of_candidates, confirmation_values);

    ifstream plain_vote_infile;
    plain_vote_infile.open(PLAIN_VOTES_TO_CHECK, ios::in|ios::binary);
    if (plain_vote_infile.is_open())
    {
        char* tmp_vote = new char[128];
        plain_vote_infile.getline(tmp_vote, 128);
        uint32_t i = 0;
        while ( plain_vote_infile && ( i < number_of_candidates ) )
        {
            plain_vote_infile.getline(tmp_vote, 128);
            plain_votes[i] = stoull(string(tmp_vote));
            i++;
        }
        plain_vote_infile.close();
    }

    /// Just for demo: Break the third_vote user to make it invalid
    if ( voted_user == "voter_3" )
    {
        tmp_vote.cipher_text_r = 3 * generator_point;
    }
    if ( this_vote.confirmVoteValidity(tmp_vote, list_of_candidates, confirmation_values) )
    {
        plain_votes[chosen_candidate]++;
        this_vote.updateVotedBallot(tmp_vote);
    }
    else
    {
        cout << "Invalid votes detected\n";
        return 1;
    }

    ofstream plain_vote_outfile;

    plain_vote_outfile.open(PLAIN_VOTES_TO_CHECK, ios::trunc|ios::binary);
    cout.rdbuf(plain_vote_outfile.rdbuf());
    for ( uint32_t i = 0; i < number_of_candidates; i++ )
    {
        cout << "\n" << plain_votes[i];
    }
    plain_vote_outfile.close();
    cout.rdbuf(console_buf);
    
    return 0;

}