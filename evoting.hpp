#ifndef TIENNV_EVOTING
#define TIENNV_EVOTING

#include <iostream>
#include <limits>
#include <random>

#include "eccElGamal.hpp"

#define MAX_RANDOM 100 /// Need this because type check for random calculations are messy

class VoteConfirmation
{
    public:
        int32_t first_factor;
        int32_t second_factor;
        ECn first_point;
        ECn second_point;

        /// Default constructor
        VoteConfirmation()
        {
            first_factor = 0;
            second_factor = 0;
            first_point = ECn(0,0);
            second_point = ECn(0,0);
        }

        /// Constructor
        VoteConfirmation(ECn public_key_alpha, ECn public_key_beta, ECn candidate_pnt, ECCElGamalCipherText encrypted_vote)
        {
            /// We can use this because the output is 64-bit, lower than 160-bit
            std::mt19937 gen (std::random_device{}());
            this->first_factor = (gen() % MAX_RANDOM);
            this->second_factor = (gen() % MAX_RANDOM);

            first_point = first_factor * public_key_alpha;
            first_point += second_factor * encrypted_vote.cipher_text_r;

            second_point = encrypted_vote.cipher_text_t;
            second_point -= candidate_pnt;
            second_point = second_factor * second_point;
            second_point += first_factor * public_key_beta;
        }

};

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

        /// @brief Initialize an empty vote process
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

        /// @brief Read vote config to construct the class
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
        /// @param candidate_ptr (Input) List of candidate points
        /// @param list_of_values (Output) List of confirmation values to check for encrypted vote validity
        /// @return Encrypted vote 
        ECCElGamalCipherText voteForACandidate(uint32_t candidate_number, ECn* const candidate_ptr, VoteConfirmation* list_of_confirmation_values)
        {
            std::mt19937 gen (std::random_device{}());
            uint32_t random_number_for_encryption = (gen() % MAX_RANDOM);

            ECn candidate_point = *(candidate_ptr + candidate_number);
            ECCElGamalCipherText vote = crypto_curve_in_use.encrypt(candidate_point, Big(random_number_for_encryption));

            /// Prepare the list of confirmation values;
            for ( uint32_t i = 0; i < number_of_candidates; i++ )
            {
                *(list_of_confirmation_values+i) = VoteConfirmation(
                                                    crypto_curve_in_use.public_key_alpha,
                                                    crypto_curve_in_use.public_key_beta,
                                                    *(candidate_ptr + i), vote);
            }

            uint32_t tmp_factor = static_cast<int32_t>(gen() % MAX_RANDOM);
            (list_of_confirmation_values+candidate_number)->first_point = tmp_factor * crypto_curve_in_use.public_key_alpha;
            (list_of_confirmation_values+candidate_number)->second_point = tmp_factor * crypto_curve_in_use.public_key_beta;
            

            ECn tmp_pnt = 0 * crypto_curve_in_use.public_key_alpha;
            int32_t tmp_sum = 0;
            for ( uint32_t i = 0; i < number_of_candidates; i++)
            {
                tmp_pnt += (list_of_confirmation_values+i)->first_point;
                tmp_pnt += (list_of_confirmation_values+i)->second_point;
                tmp_sum += (list_of_confirmation_values+i)->second_factor;
            }
            tmp_sum -= (list_of_confirmation_values+candidate_number)->second_factor;
            Big tmp_x, tmp_y;
            char str_x[100], str_y[100];
            tmp_pnt.get(tmp_x, tmp_y);
            str_x << tmp_x;
            str_y << tmp_y;
            string all_point_str = getECPointIdString(str_x, str_y);
            int32_t all_point_hash = static_cast<int32_t>(hash<string>()(all_point_str) % MAX_RANDOM);
            
            (list_of_confirmation_values+candidate_number)->second_factor = all_point_hash - tmp_sum;
            (list_of_confirmation_values+candidate_number)->first_factor = tmp_factor - ((list_of_confirmation_values+candidate_number)->second_factor * static_cast<int32_t>(random_number_for_encryption));

            // cout << "Factors: " << (list_of_confirmation_values+candidate_number)->first_factor << " - " << (list_of_confirmation_values+candidate_number)->second_factor << "\n";
            // cout << "Tmp factors: " << tmp_factor << "\n";
            // cout << "r_cp: " << random_number_for_encryption << "\n";
            // cout << "u_cp*r_cp = " << ((list_of_confirmation_values+candidate_number)->second_factor * static_cast<int32_t>(random_number_for_encryption)) << "\n";

            return vote;
        }

        /// @brief Confirm whether the encrypted vote is valid
        /// @param vote The encrypted vote to check
        /// @param candidate_ptr (Input) List of candidate points
        /// @param list_of_values (Input) List of vote confirmation values
        /// @return Is the encrypted vote valid?
        bool confirmVoteValidity(ECCElGamalCipherText vote, ECn* const candidate_ptr, VoteConfirmation* const list_of_values)
        {
            ECn tmp_pnt = 0 * crypto_curve_in_use.public_key_alpha;
            size_t second_factor_sum = 0;
            for ( uint32_t i = 0; i < number_of_candidates; i++)
            {
                ECn checked_first_point, checked_second_point;
                checked_first_point = (list_of_values+i)->first_factor * crypto_curve_in_use.public_key_alpha;
                checked_first_point += (list_of_values+i)->second_factor * vote.cipher_text_r;

                if ( checked_first_point != (list_of_values+i)->first_point )
                {
                    return false;
                }
                tmp_pnt += checked_first_point;

                checked_second_point = vote.cipher_text_t;
                checked_second_point -= *(candidate_ptr+i);
                checked_second_point = (list_of_values+i)->second_factor * checked_second_point;
                checked_second_point += (list_of_values+i)->first_factor * crypto_curve_in_use.public_key_beta;
                if ( checked_second_point != (list_of_values+i)->second_point )
                {
                    return false;
                }
                tmp_pnt += checked_second_point;

                second_factor_sum += (list_of_values+i)->second_factor;
            }

            Big tmp_x, tmp_y;
            char str_x[100], str_y[100];
            tmp_pnt.get(tmp_x, tmp_y);
            str_x << tmp_x;
            str_y << tmp_y;
            string all_point_str = getECPointIdString(str_x, str_y);
            int32_t all_point_hash = static_cast<int32_t>(hash<string>()(all_point_str) % MAX_RANDOM);
            if ( all_point_hash != second_factor_sum )
            {
                return false;
            }

            return true;
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
            if ( !votes_file.is_open() )
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
