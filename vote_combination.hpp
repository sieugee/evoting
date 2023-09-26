#ifndef VOTE_COMBINATION
#define VOTE_COMBINATION

#include <unordered_map>

#include "eccElGamal.hpp"

class VoteCombination
{
    public:
        ECn vote_result;
        ECn generator_point;
        uint32_t first_number;
        uint32_t number_of_votes;
        uint32_t max_vote;
        uint32_t* list_of_votes; /// pointer to vote array

    private:

    public:

        VoteCombination()
        {
            /// do nothing
        }

        /// Constructor
        VoteCombination(ECn generator_point, uint32_t first_number, uint32_t number_of_votes, uint32_t max_vote)
            : generator_point(generator_point)
            , first_number(first_number)
            , number_of_votes(number_of_votes)
            , max_vote(max_vote)
        {
            list_of_votes = new uint32_t[number_of_votes];
            for ( uint32_t i = 0 ; i < number_of_votes; i++ )
            {
                *(list_of_votes+i) = 0;
            }
            vote_result = 0 * generator_point;
        }

        /// Destructor
//        ~VoteCombination()
//        {
//            delete[] list_of_votes;
//        }

        /// @brief Calculate vote_result based on list_of_votes
        void calculateVoteCombinationResult()
        {
            Big factor = Big(0);
            for ( uint32_t i = 0 ; i < number_of_votes; i++ )
            {
                if ( *(list_of_votes+i) != 0 )
                {
                    Big this_vote = Big(max_vote + 1);
                    this_vote = pow(this_vote, i+first_number);
                    this_vote = *(list_of_votes+i) * this_vote;
                    factor = factor + this_vote;
                }
            }
            vote_result = factor * generator_point;
        }

        /// @brief Copy vote combination result from other sources
        /// @param calculated_vote_result Calculated vote result from new_list_of_votes
        /// @param new_list_of_votes New list of votes
        void copyVoteCombination(ECn calculated_vote_result, uint32_t* new_list_of_votes)
        {
            for ( uint32_t i = 0 ; i < number_of_votes; i++ )
            {
                *(list_of_votes+i) = *(new_list_of_votes+i);
            }
            vote_result = calculated_vote_result;
        }

        /// @brief Compare function
        /// @param combi The combination to compare this object to
        /// @return Are two combination the same vote combination?
        bool operator==(const VoteCombination& combi) const
        {
            Big this_x, this_y, combi_x, combi_y;
            this->vote_result.get(this_x, this_y);
            combi.vote_result.get(combi_x, combi_y);
            return ( ( this_x == combi_x ) && ( this_y == combi_y ) );
        }

        /// @brief Print out this object to console or to file
        void printCombination()
        {
            Big this_x, this_y;
            vote_result.get(this_x, this_y);
            cout << "\n" << this_x;
            cout << "\n" << this_y;
            cout << "\n[";
            for ( int i = 0 ; i < number_of_votes ; i ++ )
            {
                cout << *(list_of_votes+i); 
                if ( i != number_of_votes - 1 )
                {
                    cout << ",";
                }
            }
            cout << "]";
        }

};

class VoteCombinationIterator
{
    public:
        VoteCombination current_combination;

    public:
        
        /// Constructor  
        VoteCombinationIterator(VoteCombination first_combination)
            : current_combination(first_combination)
        {
        }

        /// @brief Move to the next vote combination
        /// @return True if this is not the final combination
        bool advance()
        {
            for ( int64_t i = current_combination.number_of_votes - 1 ; i >= 0 ; i-- )
            {
                if ( *(current_combination.list_of_votes+i) < current_combination.max_vote )
                {
                    *(current_combination.list_of_votes+i) = *(current_combination.list_of_votes+i) + 1;
                    for ( uint32_t j = i + 1 ; j < current_combination.number_of_votes ; j ++ )
                    {
                        *(current_combination.list_of_votes+j) = 0;
                    }
                    current_combination.calculateVoteCombinationResult();
                    return true;
                }
            }
            return false;
        }

        /// @brief print out current combination
        void getCurrent()
        {
            current_combination.printCombination();
        }

};

#endif /// VOTE_COMBINDATION