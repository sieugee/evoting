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
    this_vote.init();

    uint32_t plain_votes[number_of_candidates] = {0};
    ECn* list_of_candidates = new ECn[number_of_candidates];
    ECCElGamalCipherText vote_result;

    this_vote.getCandidateList(list_of_candidates);

    uint32_t number_of_invalid_votes = 0;

    for ( uint32_t i = 0; i < number_of_votes; i++ )
    {
        /// User vote for a candidate
        VoteConfirmation confirmation_values[number_of_candidates];
        uint32_t chosen_candidate = rand() % number_of_candidates;
        ECCElGamalCipherText tmp_vote = this_vote.voteForACandidate(chosen_candidate, list_of_candidates, confirmation_values);

        /// Just for demo: Break the 3rd vote to make it invalid
        if ( i == 2 )
        {
            tmp_vote.cipher_text_r = 3 * generator_point;
        }

        /// Server confirm vote's validity and add the vote
        if ( this_vote.confirmVoteValidity(tmp_vote, list_of_candidates, confirmation_values) )
        {
            plain_votes[chosen_candidate]++;
            this_vote.updateVotedBallot(tmp_vote);
        }
        else
        {
            number_of_invalid_votes++;
        }
    }

    ofstream plain_vote_file;
    auto* console_buf = cout.rdbuf();

    plain_vote_file.open(PLAIN_VOTES_TO_CHECK, ios::trunc|ios::binary);
    cout.rdbuf(plain_vote_file.rdbuf());
    for ( uint32_t i = 0; i < number_of_candidates; i++ )
    {
        cout << "\n" << plain_votes[i];
    }
    cout.rdbuf(console_buf);
    plain_vote_file.close();

    cout << "Finish " << number_of_votes << " votes.\n";
    cout << number_of_invalid_votes << " invalid votes detected.\n";
    cout << "The encrypted result is in " << ENCRYPTED_VOTES <<"\n";
    cout << "Plain vote result to check is in " << PLAIN_VOTES_TO_CHECK << "\n";
    cout << "---------------\n";
    cout << "List of votes to check later:\n";
    for ( uint32_t i = 0; i < number_of_candidates; i++ )
    {
        cout << "Candidate #" << i+1 << ": " << plain_votes[i] << " votes\n";
    }
    cout << "---------------\n";

    return 0;

}