#include "file_utils.hpp"
#include "evoting.hpp"
#include "vote_combination.hpp"

int main(int argc,char **argv)
{
    int st;
    Big a, b, p;
    Big private_key = Big(9876543210123);
    Big tmp_x, tmp_y;
    uint32_t number_of_candidates, number_of_votes;

    st = readCurveConfig(&a, &b, &p);
    if ( 0!= st )
    {
        return st;
    }

    st = readGeneratorPointConfig(&tmp_x, &tmp_y);
    if ( 0!= st )
    {
        return st;
    }

    ECCCurve e(a, b, p);
    ECn generator_point(tmp_x, tmp_y);
    ECCElGamal scheme(e, generator_point, private_key);

    EVoting::readVoteConfig(&number_of_candidates, &number_of_votes);
    EVoting next_vote(scheme, number_of_candidates, number_of_votes);
    next_vote.init();

    /// Prepare the candidates file
    next_vote.createCandidateList();

    /// Prepare the combination files
    uint32_t middle_number = number_of_candidates / 2;
    ofstream outfile;
    auto* console_buf = cout.rdbuf();

    /// Prepare the combination hash file
    VoteCombination hash_combi(generator_point, 0, middle_number, number_of_votes);
    VoteCombinationIterator hash_combi_it(hash_combi);

    outfile.open(COMBINATION_HASH, ios::trunc|ios::binary);
    cout.rdbuf(outfile.rdbuf());
    bool keep_loop = false;
    do
    {
        hash_combi_it.getCurrent();
    } while ( hash_combi_it.advance() );
    cout.rdbuf(console_buf);
    outfile.close();

    /// Prepare the combination_array file
    VoteCombination arr_combi(generator_point, middle_number, (number_of_candidates - middle_number), number_of_votes);
    VoteCombinationIterator arr_combi_it(arr_combi);

    outfile.open(COMBINATION_ARRAY, ios::trunc|ios::binary);
    cout.rdbuf(outfile.rdbuf());
    do
    {
        arr_combi_it.getCurrent();
    } while ( arr_combi_it.advance() );
    cout.rdbuf(console_buf);
    outfile.close();

    return 0;
}
