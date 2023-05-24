#ifndef POKER_CONTRACT_CARD_H
#define POKER_CONTRACT_CARD_H

#include <eosiolib/types.h>
#include <eosiolib/eosio.hpp>

enum Suits
{
S_SPADES,
S_HEARTS,
S_DIAMONDS,
S_CLUBS
};

struct Card
{
    Card():suit(0),value(0)
    {
    }

    Card(uint8_t suit, uint8_t value)
    :suit(suit), 
    value(value)
    {
    }

    uint8_t suit;
    uint8_t value;

    EOSLIB_SERIALIZE(Card, (suit) (value))
};

const bool operator < (const Card& card1, const Card& card2) { return card1.value > card2.value; }
const bool operator == (const Card& card1, const Card& card2) { return card1.value == card2.value; }

#endif
