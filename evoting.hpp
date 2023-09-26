#ifndef TIENNV_EVOTING
#define TIENNV_EVOTING

#include <iostream>
#include <limits>

#include "eccElGamal.hpp"

class EVoting
{
    public:
        ECCElGamal crypto_curve_in_use;
        uint32_t number_of_candidates; /// s
        uint32_t number_of_votes; /// n

    private:

    public:

        /// Constructor
        EVoting(ECCElGamal crypto_curve_in_use, uint32_t s, uint32_t n)
            : crypto_curve_in_use(crypto_curve_in_use)
            , number_of_candidates(s)
            , number_of_votes(n)
        {
            Big tmp = Big(this->number_of_votes + 1);
            tmp = pow(tmp, this->number_of_candidates - 1);
            if ( tmp >= this->crypto_curve_in_use.curve.p )
            {
                cout << "Input for votes too large\n";
                exit(-1);
            }

        }

        /// @brief Initialize an empty vote
        void init()
        {
            ofstream votes_file;
            Big tmp_x, tmp_y;
            ECCElGamalCipherText total_vote;
            total_vote.cipher_text_r = 0 * crypto_curve_in_use.public_key_alpha;
            total_vote.cipher_text_t = 0 * crypto_curve_in_use.public_key_alpha;
            
            votes_file.open(ENCRYPTED_VOTES, ios::trunc|ios::binary);
            auto* console_buf = cout.rdbuf();
            cout.rdbuf(votes_file.rdbuf());
            total_vote.cipher_text_r.get(tmp_x, tmp_y);
            cout << "\n" << tmp_x;
            cout << "\n" << tmp_y;
            total_vote.cipher_text_t.get(tmp_x, tmp_y);
            cout << "\n" << tmp_x;
            cout << "\n" << tmp_y;
            cout.rdbuf(console_buf);
            votes_file.close();
        }

        /// @brief Read vote config to construc the class
        static void readVoteConfig(uint32_t* number_of_candidates, uint32_t* number_of_votes)
        {
            ifstream config_vote;
            char* tmp_str = new char[128];

            config_vote.open(CONFIG_VOTE, ios::in|ios::binary);
            if ( !config_vote )
            {
                cout << "Cannot read config vote\n";
                exit(-1);
            }
            config_vote.getline(tmp_str, 128);
            *number_of_candidates = static_cast<uint32_t>(stoull(tmp_str)); /// first line
            config_vote.getline(tmp_str, 128);
            *number_of_votes = static_cast<uint32_t>(stoull(tmp_str)); /// second line
            delete[] tmp_str;
            config_vote.close();
            if ( *number_of_candidates <= 1 )
            {
                cout << "Error: Too few candidates\n";
                exit(-1);
            }
        }

        /// @brief Prepare a candidate list and update it to file
        void createCandidateList()
        {

            ofstream candidate_file;
            candidate_file.open(CANDIDATES, ios::trunc|ios::binary);
            auto* console_buf = cout.rdbuf();
            cout.rdbuf(candidate_file.rdbuf());

            for ( uint32_t i = 0 ; i < number_of_candidates; i++ )
            {
                Big tmp_factor, tmp_x, tmp_y;
                tmp_factor = Big(this->number_of_votes + 1);
                tmp_factor = pow(tmp_factor, i);
                ECn tmp_candidate = tmp_factor * crypto_curve_in_use.public_key_alpha;
                tmp_candidate.get(tmp_x, tmp_y);
                if ( ( tmp_x == 0 ) && ( tmp_y == 0 ) )
                {
                    cout.rdbuf(console_buf); /// Return cout to console before halting this program
                    cout << "A candidate point is point at infinity\n";
                    exit(-1);
                }
                cout << "\n" << tmp_x;
                cout << "\n" << tmp_y;
            }
            cout.rdbuf(console_buf);
            candidate_file.close();
        }
        
        /// @brief Read the candidate file and confirm they are in the curve
        /// @param candidate_ptr Output array for the candidates
        void getCandidateList(ECn* const candidate_ptr)
        {
            uint32_t number_of_gotten_candidates = 0;
            ifstream candidate_file;
            char* tmp_line = new char[128];
            ECn* tmp_candidate = candidate_ptr;
            candidate_file.open(CANDIDATES, ios::in|ios::binary);
            if ( !candidate_file )
            {
                cout << "Cannot get candidate list\n";
                exit(-1);
            }

            candidate_file.ignore(numeric_limits<streamsize>::max(),'\n');
            while ( candidate_file ) 
            {
                Big tmp_x, tmp_y;
                candidate_file.getline(tmp_line, 128);
                tmp_x = Big(tmp_line);
                if (!candidate_file)
                {
                    break;
                }
                candidate_file.getline(tmp_line, 128);
                tmp_y = Big(tmp_line);
                *tmp_candidate = ECn(tmp_x, tmp_y);
                tmp_candidate++;
                number_of_gotten_candidates++;
            }
            candidate_file.close();

            if ( number_of_gotten_candidates != this->number_of_candidates )
            {
                string err_msg = (number_of_gotten_candidates > this->number_of_candidates)?"More":"Less";
                cout << "Error: " << err_msg << "candidates than it should be\n";
                exit(-1);
            }
        }

        /// @brief Vote for a candidate
        /// @param candidate_number Number representing that candidate
        /// @return Encrypted vote 
        ECCElGamalCipherText voteForACandidate(uint32_t candidate_number, ECn* const candidate_ptr)
        {
            ECn candidate_point = *(candidate_ptr + candidate_number);
            ECCElGamalCipherText vote = crypto_curve_in_use.encrypt(candidate_point);
            return vote;
        }

        /// @brief Get total vote from db file
        /// @return Total votes in encrypted format
        ECCElGamalCipherText getEncryptedTotalVote()
        {
            ifstream votes_file;
            Big tmp_x, tmp_y;
            ECCElGamalCipherText total_vote;
            char* tmp_line = new char[128];

            /// Get current total vote
            votes_file.open(ENCRYPTED_VOTES, ios::in|ios::binary);
            if ( !votes_file )
            {
                cout << "No existing vote results\n";
                exit(-1);
            }
            votes_file.ignore(numeric_limits<streamsize>::max(),'\n');

            votes_file.getline(tmp_line, 128);
            tmp_x = Big(tmp_line);
            votes_file.getline(tmp_line, 128);
            tmp_y = Big(tmp_line);
            total_vote.cipher_text_r = ECn(tmp_x, tmp_y);

            votes_file.getline(tmp_line, 128);
            tmp_x = Big(tmp_line);
            votes_file.getline(tmp_line, 128);
            tmp_y = Big(tmp_line);
            total_vote.cipher_text_t = ECn(tmp_x, tmp_y);

            delete[] tmp_line;
            votes_file.close();

            return total_vote;
        }

        /// @brief Update the encrypted vote ballot and rewrite the file
        /// @param encrypted_vote new vote
        void updateVotedBallot(ECCElGamalCipherText encrypted_vote)
        {
            ofstream votes_file;
            Big tmp_x, tmp_y;
            ECCElGamalCipherText total_vote = this->getEncryptedTotalVote();

            total_vote.cipher_text_r += encrypted_vote.cipher_text_r;
            total_vote.cipher_text_t += encrypted_vote.cipher_text_t;
            
            /// Update it to the file
            votes_file.open(ENCRYPTED_VOTES, ios::trunc|ios::binary);
            auto* console_buf = cout.rdbuf();
            cout.rdbuf(votes_file.rdbuf());
            total_vote.cipher_text_r.get(tmp_x, tmp_y);
            cout << "\n" << tmp_x;
            cout << "\n" << tmp_y;
            total_vote.cipher_text_t.get(tmp_x, tmp_y);
            cout << "\n" << tmp_x;
            cout << "\n" << tmp_y;
            cout.rdbuf(console_buf);
            votes_file.close();
        }

        /// @brief Decrypt the total vote from file and output the result
        ECn getVoteResult()
        {
            ECCElGamalCipherText total_vote = this->getEncryptedTotalVote();
            return this->crypto_curve_in_use.decrypt(total_vote);
        }

        void countVotes()
        {
            ECn vote_result_pnt;
            Big tmp_x, tmp_y;
            ECCElGamalCipherText total_vote = this->getEncryptedTotalVote();
            vote_result_pnt = this->crypto_curve_in_use.decrypt(total_vote);
            vote_result_pnt.get(tmp_x, tmp_y);
            cout << "(" << tmp_x << ", " << tmp_y << ")\n";

        }

};

#endif /// EVOTING
