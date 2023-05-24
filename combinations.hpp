#ifndef POKER_CONTRACT_COMBINATIONS_H
#define POKER_CONTRACT_COMBINATIONS_H

#include <eosiolib/types.h>
#include "card.hpp"

enum CombinationTypes
{
    C_NO_COMBINATION,
    C_HIGH_CARD,
    C_PAIR,
    C_TWO_PAIRS,
    C_THREE_OF_A_KIND,
    C_STRAIGHT,
    C_FLUSH,
    C_FULL_HOUSE,
    C_FOUR_OF_A_KIND,
    C_STRAIGHT_FLUSH,
    C_ROYAL_FLUSH
};

struct Combination
{
    Combination()
    : cards(5) // like a Card cards[5];
    {

    }

    uint8_t type = C_NO_COMBINATION;
    std::vector<Card> cards;

    EOSLIB_SERIALIZE(Combination, (type) (cards))
};

int getCombination(const std::multiset<Card>& cards, Combination& combo);

const bool operator > (const Combination& combo1, const Combination& combo2);

#endif //POKER_CONTRACT_COMBINATIONS_H