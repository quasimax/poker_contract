#include <vector>
#include <set>
#include <algorithm>
#include <functional>
#include <iterator>
#include <string>
#include <eosiolib/time.hpp>
#include "pokercontract.hpp"
#include <math.h>
#include <eosiolib/print.hpp> // for DEBUG!

#define ACE_CARD            14
#define MAX_INDEX_IN_COMBO  4
#define COMBO_SIZE          5

globalstate pokercontract::get_default_parameters()
{
        globalstate gs;

        gs.version = 393;

        gs.small_blind_values.push_back(eosio::asset(100, EOS_SYMBOL));
        gs.small_blind_values.push_back(eosio::asset(200, EOS_SYMBOL));
        gs.small_blind_values.push_back(eosio::asset(500, EOS_SYMBOL));
        gs.small_blind_values.push_back(eosio::asset(1000, EOS_SYMBOL));
        gs.small_blind_values.push_back(eosio::asset(10000, EOS_SYMBOL));

        gs.max_players_count_values.push_back(6);

        gs.penalty_percent = 50;
        gs.max_penalty_value = eosio::asset(500000, EOS_SYMBOL);
        gs.player_pay_percent = 70;
        gs.master_pay_percent = 30;
        gs.rake_percent = 3;
        gs.max_rake_value = eosio::asset(50000, EOS_SYMBOL);
        gs.warning_timeout_sec = 25;
        gs.last_timeout_sec = 15;
        gs.delete_table_timeout_sec = 10;
        gs.delete_tables_count = 2;
        gs.freezing = 0;
        gs.min_sb_buyin = 40;
        gs.max_sb_buyin = 200;
        gs.r = eosio::asset(0, EOS_SYMBOL);

        return gs;
}

ACTION pokercontract::setparams(eosio::name owner, globalstate& gs)
{
    require_auth(owner);
    eosio_assert(owner == _self, "Only owner can run setparams");

    globalstate gstate_for_version = get_default_parameters();
    globalstate gstate = global.get();
    gstate.version = gstate_for_version.version;

    eosio_assert(gs.small_blind_values.size()>=1, "small_blind_values must contain at least one value");
    std::set<eosio::asset> check;
    for(eosio::asset a: gs.small_blind_values)
    {
        eosio_assert(a.amount > 0, "small_blind_values must be positive");
        eosio_assert(a.symbol == EOS_SYMBOL, "wrong small blind symbol");
        check.insert(a);        
    }
    eosio_assert(check.size()==gs.small_blind_values.size(),"small_blind_values contains same elements");
    gstate.small_blind_values = gs.small_blind_values;

    eosio_assert(gs.max_players_count_values.size()>=1, "max_players_count_values must contain at least one value");
    std::set<uint8_t> check_count_players;
    for(uint8_t count: gs.max_players_count_values)
    {
        eosio_assert(count > 0, "max_players_count_values must be positive");
        check_count_players.insert(count);
    }
    eosio_assert(check_count_players.size()==gs.max_players_count_values.size(),"max_players_count_values contains same elements");
    eosio_assert( (*(check_count_players.begin())) >= 2,"max_players_count_values minimum value must be greater or equal 2");
    eosio_assert( (*(check_count_players.rbegin())) <= 9,"max_players_count_values maximum value must be less or equal 9");
    gstate.max_players_count_values = gs.max_players_count_values;

    eosio_assert(gs.penalty_percent > 0, "penalty_percent must be greater than 0 and less or equal 50");
    eosio_assert(gs.penalty_percent <= 50, "penalty_percent must be greater than 0 and less or equal 50");
    gstate.penalty_percent = gs.penalty_percent;

    eosio_assert(gs.max_penalty_value.symbol == EOS_SYMBOL, "wrong max_penalty_value symbol");
    eosio_assert(gs.max_penalty_value.amount > 0, "max_penalty_value must be greater than 0");
    eosio_assert(gs.max_penalty_value.amount <= 500000, "max_penalty_value must be less or equal 50 EOS");
    gstate.max_penalty_value = gs.max_penalty_value;

    eosio_assert(gs.player_pay_percent >= 50, "player_pay_percent must be greater or equal 50 and less or equal 80");
    eosio_assert(gs.player_pay_percent <= 80, "player_pay_percent must be greater or equal 50 and less or equal 80");
    gstate.player_pay_percent = gs.player_pay_percent;

    eosio_assert(gs.master_pay_percent >= 20, "master_pay_percent must be greater or equal 20 and less or equal 50");
    eosio_assert(gs.master_pay_percent <= 50, "master_pay_percent must be greater or equal 20 and less or equal 50");
    gstate.master_pay_percent = gs.master_pay_percent;

    eosio_assert(gs.master_pay_percent + gs.player_pay_percent == 100, "sum of master_pay_percent and player_pay_percent must be 100");

    eosio_assert(gs.rake_percent >= 0.1, "rake_percent must be greater or equal 0.1 and less or equal 5.0");
    eosio_assert(gs.rake_percent <= 5.0, "rake_percent must be greater or equal 0.1 and less or equal 5.0");
    gstate.rake_percent = gs.rake_percent;

    double min_rake_percent = 0.1;
    int n = (gs.rake_percent*10)/(min_rake_percent*10);
    float check_rake_percent = n * min_rake_percent;

    eosio_assert(check_rake_percent == gs.rake_percent, "rake_percent must be a multiple of 0.1");

    eosio_assert(gs.max_rake_value.symbol == EOS_SYMBOL, "wrong max_rake_value symbol");
    eosio_assert(gs.max_rake_value.amount > 0, "max_rake_value must be greater than 0");
    eosio_assert(gs.max_rake_value.amount <= 500000, "max_rake_value must be less or equal 50 EOS");
    gstate.max_rake_value = gs.max_rake_value;

    eosio_assert(gs.warning_timeout_sec >= 10, "warning_timeout_sec must be greater or equal 10 and less or equal 30");
    eosio_assert(gs.warning_timeout_sec <= 30, "warning_timeout_sec must be greater or equal 10 and less or equal 30");
    gstate.warning_timeout_sec = gs.warning_timeout_sec;

    eosio_assert(gs.last_timeout_sec >= 10, "last_timeout_sec must be greater or equal 10 and less or equal 60");
    eosio_assert(gs.last_timeout_sec <= 60, "last_timeout_sec must be greater or equal 10 and less or equal 60");
    gstate.last_timeout_sec = gs.last_timeout_sec;

    eosio_assert(gs.delete_table_timeout_sec >= 1, "delete_table_timeout_sec must be greater or equal 1 and less or equal 600");
    eosio_assert(gs.delete_table_timeout_sec <= 600, "delete_table_timeout_sec must be greater or equal 1 and less or equal 600");
    gstate.delete_table_timeout_sec = gs.delete_table_timeout_sec;

    eosio_assert(gs.delete_tables_count >= 1, "delete_tables_count must be greater or equal 1 and less or equal 10");
    eosio_assert(gs.delete_tables_count <= 10, "delete_tables_count must be greater or equal 1 and less or equal 10");
    gstate.delete_tables_count = gs.delete_tables_count;

    eosio_assert( (gs.freezing == 0) || (gs.freezing == 1), "freezing must be 0 or 1");
    gstate.freezing = gs.freezing;

    eosio_assert( gs.client_version.length() > 0, "client_version length must be > 0");
    gstate.client_version = gs.client_version;

    global.set(gstate, owner);
}

ACTION pokercontract::setref(eosio::name owner, uint32_t percent)
{
    require_auth(owner);
    eosio_assert(owner == _self, "Only owner can run setref");

    eosio_assert(percent <= 100, "percent must be less or equal 100");

    globalref gref = global_ref.get();
    gref.percent = percent;
    global_ref.set(gref, owner);
}

void setSuitsByGraterValue( std::set<Card>& spades, 
                            std::set<Card>& hearts, 
                            std::set<Card>& diamonds, 
                            std::set<Card>& clubs, 
                            const std::multiset<Card>& cards)
{
    for( auto it = cards.begin(); it != cards.end(); it++)
    {
        if( (*it).suit == S_SPADES)
            spades.insert( *it );
        else if( (*it).suit == S_HEARTS)
            hearts.insert( *it );
        else if( (*it).suit == S_DIAMONDS)
            diamonds.insert( *it );
        else if( (*it).suit == S_CLUBS)
            clubs.insert( *it );
    }
}

bool getBestFlushInSuit(const std::set<Card>& cards, Combination& combo)
{
    if(cards.size() < COMBO_SIZE)
        return false;

    uint8_t index_in_combo = 0;

    for(auto it_cur = cards.begin(); it_cur!= cards.end(); it_cur++)
    {
        combo.cards[index_in_combo] = *it_cur;

        auto it_next = std::next(it_cur,1);
        if(it_next == cards.end())
            break;

        if( it_cur->value - 1 == it_next->value )
        {
            if( ++index_in_combo == MAX_INDEX_IN_COMBO)
            {
                combo.cards[index_in_combo] = *it_next;
                combo.type = (combo.cards[0].value == ACE_CARD) ? C_ROYAL_FLUSH : C_STRAIGHT_FLUSH;
                return true;
            }
        }
        else
            index_in_combo = 0;
    }

    // check a wheel = 5 4 3 2 + A
    if( (combo.cards[0].value == 5) && (index_in_combo == 3) )
    {
        if((*cards.begin()).value == ACE_CARD)
        {
            combo.cards[MAX_INDEX_IN_COMBO] = *cards.begin();
            combo.type = C_STRAIGHT_FLUSH;
            return true;
        }
    }

    // just simple flush
    std::copy_n( cards.begin(), COMBO_SIZE, combo.cards.begin() );
    combo.type = C_FLUSH;
    return true;
}

Combination getBestFlushCombo(const Combination& combo1, const Combination& combo2)
{
    if(combo1.type > combo2.type)
        return combo1;

    if(combo2.type > combo1.type)
        return combo2;

    if(std::lexicographical_compare(combo1.cards.begin(), combo1.cards.end(), 
                                    combo2.cards.begin(), combo2.cards.end()))
        return combo1;
    else return combo2;
}

int getBestFlush(const std::multiset<Card>& cards, Combination& combo)
{
    if(cards.size()<5)
        return 0;

    combo.type = C_NO_COMBINATION;

    std::vector< std::set<Card> >   suits(4); // spades hearts diamonds clubs
    std::vector< Combination >      tmp_combo(4);

    setSuitsByGraterValue(suits[0], suits[1], suits[2], suits[3], cards);

    for(int i = 0; i< 4; i++)
    {
        if(getBestFlushInSuit(suits[i], tmp_combo[i]) == false)
            continue;
        
        combo = tmp_combo[i];

        if(tmp_combo[i].type == C_ROYAL_FLUSH)
            return combo.type;

        if(i > 0) // compare with previous
            combo = getBestFlushCombo(tmp_combo[i-1], tmp_combo[i]);
    }

    return combo.type;
}

int getBestFourOfAKind(const std::multiset<Card>& cards, Combination& combo)
{
    if(cards.size() < 5)
        return 0;

    uint8_t index_in_combo = 0; // if this index == 3 then we have four of a kind!

    for(auto it_cur = cards.begin(); it_cur != cards.end(); it_cur++)
    {
        combo.cards[index_in_combo] = *it_cur;
        
        auto it_next = std::next(it_cur,1);
        if(it_next == cards.end())
            break;

        if( (*it_cur).value == (*it_next).value )
        {
            if(++index_in_combo == 3) 
            {
                combo.cards[index_in_combo] = *it_next;

                // we always have previous or next card in this case
                if( (*cards.begin()).value != (*it_cur).value )
                    combo.cards[MAX_INDEX_IN_COMBO] = *cards.begin();
                else
                    combo.cards[MAX_INDEX_IN_COMBO] = *(++it_next);

                combo.type = C_FOUR_OF_A_KIND;
                return combo.type;
            }
        }
        else
            index_in_combo = 0;
    }

    return 0;
}

Combination getBestFourOfAKindCombo(const Combination& combo1, const Combination& combo2)
{
    if(combo1.cards[0].value > combo2.cards[0].value)
        return combo1;
    else
        return combo2;
}

int getThreeOfAKindOnly(const std::multiset<Card>& cards, Combination& combo)
{
    if(cards.size()<3)
        return 0;

    uint8_t cards_in_line=1;

    for(auto it_cur = cards.begin(); it_cur != cards.end(); it_cur++)
    {
        auto it_next = std::next(it_cur,1);
        if(it_next == cards.end())
            break;

        if( (*it_cur).value == (*it_next).value )
            cards_in_line++;
        else
            cards_in_line = 1;

        if(cards_in_line == 3) // have three of a kind
        {
            it_cur--;

            for(int i=0; i<3; i++)
            {
                combo.cards[i] = *it_cur;
                it_cur++;
            }
            combo.type = C_THREE_OF_A_KIND;
            return combo.type;
        }
    }
    return 0;
}

int getBestFullHouse(const std::multiset<Card>& cards, Combination& combo)
{
    if(cards.size()<5)
        return 0;

    std::multiset<Card> remaining_cards(cards);

    // looking for three of a kind
    if(!getThreeOfAKindOnly(cards, combo))
        return 0;

    for(int i=0; i<3; i++)
    {
        auto it_for_remove = remaining_cards.find(combo.cards[i]);
        eosio_assert(it_for_remove != remaining_cards.end(), "find assertion");
        remaining_cards.erase(it_for_remove);
    }

    // now looking for a pair in remaining cards
    for(auto it_cur = remaining_cards.begin(); it_cur != remaining_cards.end(); it_cur++)
    {
        auto it_next = std::next(it_cur,1);
        if(it_next == remaining_cards.end())
            break;

        if( (*it_cur).value == (*it_next).value ) // got it!
        {
            combo.cards[3] = *it_cur;
            combo.cards[MAX_INDEX_IN_COMBO] = *it_next;
            combo.type = C_FULL_HOUSE;
            return combo.type;
        }
    }

    return 0;
}

int getBestStraight(const std::multiset<Card>& cards, Combination& combo)
{
    if(cards.size()<5)
        return 0;

    uint8_t cards_in_line = 1;

    for(auto it_cur = cards.begin(); it_cur != cards.end(); it_cur++)
    {
        combo.cards[cards_in_line-1] = *it_cur;

        auto it_next = std::next(it_cur, 1);
        if(it_next == cards.end())
            break;

        if( (*it_cur).value == (*it_next).value )
            continue;

        if( (*it_cur).value - 1 == (*it_next).value )
            cards_in_line++;
        else
            cards_in_line = 1;

        if(cards_in_line == 5) // got it!
        {
            combo.cards[MAX_INDEX_IN_COMBO] = *it_next;
            combo.type = C_STRAIGHT;
            return combo.type;
        }
    }

    // check a wheel = 5 4 3 2 + A
    if( (combo.cards[0].value == 5) && (cards_in_line == 4) )
    {
        if((*cards.begin()).value == ACE_CARD)
        {
            combo.cards[MAX_INDEX_IN_COMBO] = *cards.begin();
            combo.type = C_STRAIGHT;
            return combo.type;
        }
    }

    return 0;
}

int getBestThreeOfAKind(const std::multiset<Card>& cards, Combination& combo)
{
    if(cards.size()<5)
        return 0;

    std::multiset<Card> remaining_cards(cards);

    if(!getThreeOfAKindOnly(cards, combo))
        return 0;

    for(int i=0; i<3; i++)
    {
        auto it_for_remove = remaining_cards.find(combo.cards[i]);
        eosio_assert(it_for_remove != remaining_cards.end(), "find assertion");
        remaining_cards.erase(it_for_remove);
    }

    auto it = remaining_cards.begin();
    combo.cards[3] = *it;
    combo.cards[MAX_INDEX_IN_COMBO] = *(++it);
    combo.type = C_THREE_OF_A_KIND;
    return combo.type;
}

int getBestPairOnly(const std::multiset<Card>& cards, Combination& combo)
{
    for(auto it_cur = cards.begin(); it_cur != cards.end(); it_cur++)
    {
        auto it_next = std::next(it_cur, 1);
        if(it_next == cards.end())
            break;

        if( (*it_cur).value == (*it_next).value) // got first pair!
        {
            combo.cards[0] = *it_cur;
            combo.cards[1] = *it_next;
            return C_PAIR;
        }
    }
    
    return 0;
}

int getBestTwoPairs(const std::multiset<Card>& cards, Combination& combo)
{
    if(cards.size()<5)
        return 0;

    std::multiset<Card> remaining_cards(cards);
    Combination tmp_combo;
    uint8_t combo_index = 0;
    uint8_t pair_conts = 2;

    while(pair_conts--)
    {
        if(getBestPairOnly(remaining_cards, tmp_combo) == 0)
        {
            return 0;
        }

        for(int i=0; i<2; i++)
        {
            combo.cards[combo_index] = tmp_combo.cards[i];
            auto it_for_remove = remaining_cards.find(combo.cards[combo_index++]);
            eosio_assert(it_for_remove != remaining_cards.end(), "find assertion");
            remaining_cards.erase(it_for_remove);
        }
    }

    combo.cards[MAX_INDEX_IN_COMBO] = *remaining_cards.begin();
    combo.type = C_TWO_PAIRS;
    return C_TWO_PAIRS;
}

int getBestPair(const std::multiset<Card>& cards, Combination& combo)
{
    if(cards.size()<5)
        return 0;

    std::multiset<Card> remaining_cards(cards);
    uint8_t combo_index = 0;

    if(getBestPairOnly(remaining_cards, combo) == 0)
            return 0;

    for( int i=0; i<2; i++)
    {
        auto it_for_remove = remaining_cards.find(combo.cards[i]);
        eosio_assert(it_for_remove != remaining_cards.end(), "find assertion");
        remaining_cards.erase(it_for_remove);
    }
    
    auto it = remaining_cards.begin();
    for( int index=2; index<=MAX_INDEX_IN_COMBO; index++)
    {
        combo.cards[index] = *(it++);
    }

    combo.type = C_PAIR;
    return C_PAIR;
}

int getTheHighestCard(const std::multiset<Card>& cards, Combination& combo)
{
    if(cards.size() < 5)
        return 0;

    auto it = cards.begin();
    for(int i=0; i<=MAX_INDEX_IN_COMBO; i++)
        combo.cards[i] = *(it++);

    combo.type = C_HIGH_CARD;
    return combo.type;
}

// return type of combo
int getCombination(const std::multiset<Card>& cards, Combination& combo)
{
    if(cards.size()<5)
        return 0;

    bool have_flush = false;
    Combination flush_combo;

    int res = getBestFlush(cards, combo);
    if(res == C_ROYAL_FLUSH || res == C_STRAIGHT_FLUSH)
    {
        return combo.type;
    }

    if(res == C_FLUSH)
    {
        flush_combo = combo;
        have_flush = true;
    }

    res = getBestFourOfAKind(cards, combo);
    if(res == C_FOUR_OF_A_KIND)
    {
        return combo.type;
    }

    res = getBestFullHouse(cards, combo);
    if(res == C_FULL_HOUSE)
    {
        return combo.type;
    }

    if(have_flush)
    {
        combo = flush_combo;
        return combo.type;
    }

    res = getBestStraight(cards, combo);
    if(res == C_STRAIGHT)
    {
        return combo.type;
    }

    res = getBestThreeOfAKind(cards, combo);
    if(res == C_THREE_OF_A_KIND)
    {
        return combo.type;
    }

    res = getBestTwoPairs(cards, combo);
    if(res == C_TWO_PAIRS)
    {
        return combo.type;
    }

    res = getBestPair(cards, combo);
    if(res == C_PAIR)
    {
        return combo.type;
    }

    res = getTheHighestCard(cards, combo);
    if(res == C_HIGH_CARD)
    {
        return combo.type;
    }

    return 0;
}

const bool operator > (const Combination& combo1, const Combination& combo2)
{
    if(combo1.type > combo2.type)
        return true;

    if(combo1.type < combo2.type)
        return false;

    switch(combo1.type)
    {
        case C_HIGH_CARD:
        case C_PAIR:
        case C_TWO_PAIRS:
        case C_FOUR_OF_A_KIND:
        case C_THREE_OF_A_KIND:
        case C_FULL_HOUSE:
        case C_FLUSH:
        {
            for(int i=0; i<combo1.cards.size();i++)
            {
                if(combo1.cards[i] == combo2.cards[i])
                    continue;

                return combo1.cards[i].value > combo2.cards[i].value;
            }
            return false;
        }
        case C_STRAIGHT:
        case C_STRAIGHT_FLUSH:
        {
            return combo1.cards[0].value > combo2.cards[0].value;
        }

        case C_ROYAL_FLUSH:
            return false;

        default:
            return true; // C_NO_COMBO
    }
    eosio_assert(false,"wrong combination type");
    return false;
}

//-----------------------------------------------------------------------------
#define UINT_BIT ( sizeof( unsigned int ) * CHAR_BIT )
#define C1 0x1010101
#define C2 0x1010104

typedef unsigned long long int ULONG64;
typedef unsigned int UINT32;
typedef unsigned char UINT8;
typedef unsigned char BOOL;

struct CGost89Crypt
{
	UINT32 m_uiKey[8];
	UINT8 m_iTable[8][16];
};

static const UINT8 m_iTable[8][16] =
{
	0xF, 0xC, 0x2, 0xA, 0x6, 0x4, 0x5, 0x0, 0x7, 0x9, 0xE, 0xD, 0x1, 0xB, 0x8, 0x3,
	0xB, 0x6, 0x3, 0x4, 0xC, 0xF, 0xE, 0x2, 0x7, 0xD, 0x8, 0x0, 0x5, 0xA, 0x9, 0x1,
	0x1, 0xC, 0xB, 0x0, 0xF, 0xE, 0x6, 0x5, 0xA, 0xD, 0x4, 0x8, 0x9, 0x3, 0x7, 0x2,
	0x1, 0x5, 0xE, 0xC, 0xA, 0x7, 0x0, 0xD, 0x6, 0x2, 0xB, 0x4, 0x9, 0x3, 0xF, 0x8,

	0x0, 0xC, 0x8, 0x9, 0xD, 0x2, 0xA, 0xB, 0x7, 0x3, 0x6, 0x5, 0x4, 0xE, 0xF, 0x1,
	0x8, 0x0, 0xF, 0x3, 0x2, 0x5, 0xE, 0xB, 0x1, 0xA, 0x4, 0x7, 0xC, 0x9, 0xD, 0x6,
	0x3, 0x0, 0x6, 0xF, 0x1, 0xE, 0x9, 0x2, 0xD, 0x8, 0xC, 0x4, 0xB, 0xA, 0x5, 0x7,
	0x1, 0xA, 0x6, 0x8, 0xF, 0xB, 0x0, 0x4, 0xC, 0x3, 0x5, 0x9, 0x7, 0xD, 0x2, 0xE
};

struct GostData
{
	union
	{
		ULONG64 m_lData;
		UINT8 m_chData[8];
		UINT32 m_iData[2];
	};
};

static const int g_iKeyOffset[32] =
{
	0, 1, 2, 3, 4, 5, 6, 7,
	0, 1, 2, 3, 4, 5, 6, 7,
	0, 1, 2, 3, 4, 5, 6, 7,
	7, 6, 5, 4, 3, 2, 1, 0
};

inline unsigned int GetBit(void * src, unsigned int pos)
{
	unsigned char* ptr = (unsigned char*)src + pos / CHAR_BIT;
	return (*ptr >> (pos % CHAR_BIT)) & 1;
}

inline void SetBit(void* dst, unsigned int pos, unsigned int value)
{
	unsigned char* ptr = (unsigned char*)dst + pos / CHAR_BIT;
	unsigned bit_num = pos % CHAR_BIT;
	*ptr = (*ptr &~(1 << bit_num)) | ((value & 1) << bit_num);
}

ULONG64 GetRGPCH(ULONG64 inData)
{
	UINT32 N1;
	UINT32 N2;

	N1 = (UINT32)inData;
	N2 = inData >> 32;

	N1 = (N1 + C1) % 0x10000000;
	N2 = (N2 + C2) % 0xFFFFFFFF;

	inData = N2;
	inData = (inData << 32) | N1;
	return inData;
}

void DoMainStep(struct GostData* inData, BOOL bMac, BOOL bCrypt, const struct CGost89Crypt* inCryptEntity)
{
	unsigned int iLeft = inData->m_iData[0]; // Разбиваем на 2 блока.
	unsigned int iRight = inData->m_iData[1];
	unsigned int iSum = 0;

	unsigned int iMaxLoop;
	if (bMac) // Выработка имитовставки.
	{
		iMaxLoop = 16;
		bCrypt = true;
	}
	else
		iMaxLoop = 32;

	unsigned int i;
	unsigned int z;
	for (z = 0; z < iMaxLoop; z++)
	{
		if (bCrypt)    //  S = N1 + X (mod 2 в 32).
			iSum = iLeft + inCryptEntity->m_uiKey[g_iKeyOffset[z]] % INT_MAX;

		else
			iSum = iLeft + inCryptEntity->m_uiKey[g_iKeyOffset[31 - z]] % INT_MAX;

		int iBitArray[8];
		memset(iBitArray, 0, sizeof(int) * 8);

		// Число S разбивается на 8 частей: S0,S1,S2,S3, S4,S5,S6,S7 по 4 бита каждая, где S0 - младшая, а S7 - старшая части числа S.
		int iOffset = 31;
		for (i = 0; i < 8; i++)
		{
			SetBit(&iBitArray[i], 0, GetBit(&iSum, iOffset));
			iOffset--;
			SetBit(&iBitArray[i], 1, GetBit(&iSum, iOffset));
			iOffset--;
			SetBit(&iBitArray[i], 2, GetBit(&iSum, iOffset));
			iOffset--;
			SetBit(&iBitArray[i], 3, GetBit(&iSum, iOffset));
			iOffset--;
		};


		// Для всех i от 0 до 7: Si = T(i, Si), где T(a, b) означает ячейку таблицы замен с номером строки a и номером столбца b (счет с нуля).
		for (i = 0; i < 8; i++)
			iBitArray[i] = inCryptEntity->m_iTable[i][iBitArray[i]];

		iOffset = 0; // Укладываем обратно.
		for (i = 0; i < 8; i++)
		{
			SetBit(&iSum, iOffset, GetBit(&iBitArray[i], 0));
			iOffset++;

			SetBit(&iSum, iOffset, GetBit(&iBitArray[i], 1));
			iOffset++;

			SetBit(&iSum, iOffset, GetBit(&iBitArray[i], 2));
			iOffset++;

			SetBit(&iSum, iOffset, GetBit(&iBitArray[i], 3));
			iOffset++;
		};

		// Новое число S, полученное на предыдущем шаге циклически сдвигается в сторону старших разрядов на 11 бит.
		iSum = (iSum << 11) | (iSum >> 21);

		// S = S xor N2, где xor - операция исключающего или.
		iSum = iRight ^ iSum;

		iRight = iLeft; //# N2 = N1.
		iLeft = iSum;  //# N1 = S.
	}
	inData->m_iData[0] = iRight;
	inData->m_iData[1] = iLeft;
}

void GostGammaBlockEncode(struct GostData* inData, unsigned int iBlockCount, ULONG64 inKey, const struct CGost89Crypt* inCryptEntity)
{
	unsigned int i = 0;
	for (i = 0; i < iBlockCount; i++)
	{
		inKey = GetRGPCH(inKey); // Инициализируем РГПСЧ зашифрованной синхропосылкой.
		DoMainStep((struct GostData*)&inKey, false, true, inCryptEntity); // Шифруем синхропосылку главным шагом.
		inData->m_lData ^= inKey; //  Накладываем гамму.
		//inData++;
	}
}

unsigned char* decrypt_data1(unsigned char* data, unsigned int* key, unsigned int* synchro)
{
	CGost89Crypt g_ctx;
	memset(&g_ctx, 0, sizeof(CGost89Crypt));
	memcpy(&g_ctx.m_uiKey, &key[0], 32);
	memcpy(&g_ctx.m_iTable, &m_iTable, 128);

	ULONG64 sync_tmp;
	memcpy(&sync_tmp, &synchro[0], 8);

	unsigned char* tmp = new unsigned char[8];
	memcpy(tmp, data, 8);
	GostGammaBlockEncode((GostData*)tmp, 1, sync_tmp, &g_ctx);
	return tmp;
}

int freeMem(unsigned char* arrayPtr) {
	delete[] arrayPtr;
	return 0;
}

//-----------------------------------------------------------------------------

void Table::initTheDeckOfCards()
{
    the_deck_of_cards = the_const_deck;
}

uint8_t Table::getTableStatus() const
{
    return table_status;
}

void Table::setTableStatus(const uint8_t new_status)
{
    table_status = new_status;
}

void Table::updateOutPlayerCurRoundBets(Player& out_plr, uint8_t plr_index)
{
	eosio::asset out_player_bet = out_plr.cur_round_bets;
    
    for(Player& plr: players)
    {
        if(plr.name == out_plr.name)
            continue;

        if(plr.status == P_NO_PLAYER || plr.status == P_WAIT_NEW_GAME)
            continue;

        if(plr.all_in_flag == P_ALL_IN && plr.all_in_round == current_game_round)
        {
            if(out_player_bet < plr.acts.back().bet_)
                plr.all_in_bank += out_player_bet;
            else
                plr.all_in_bank += plr.acts.back().bet_;
        }
    }
            
    bank += out_player_bet;
    table_cur_round_bets -= out_player_bet;
    current_round_players_bet_acts[plr_index] = 0;
}

void Table::addNewAct(Player& player, uint8_t player_index, Act& act)
{
    switch(act.act_)
    {
        case ACT_BET:
        {
            eosio_assert(act.bet_.is_valid(), "Invalid bet value");
            eosio_assert(act.bet_.amount > 0, "Act: Quantity must be positive");

            // all_in
            if(act.bet_ == player.stack + player.cur_round_bets)
            {
                act.description = ACT_ALLIN;
                player.all_in_flag = P_ALL_IN;
                player.all_in_bank = bank;
                player.all_in_round = current_game_round;
                allin_players_count++;
                break;
            }

            // сумма ставок в этом раунде + stack <= bet
            eosio_assert(act.bet_ <= player.stack + player.cur_round_bets, "Insufficient stack");

            // NO BET`s YET
            if(current_bet.amount == 0)
            {
                int n = act.bet_/small_blind;
                eosio::asset check = n*small_blind;
                eosio_assert(check == act.bet_,"Invalid bet value");

                eosio_assert(act.bet_ >= 2*small_blind, "Invalid bet value");
                break;
            }

            // CALL
            if(act.bet_ == current_bet)
            {
                act.description = ACT_CALL;
                break;
            }

            // ONLY RAISE
            eosio_assert(act.bet_ >= current_bet*2, "Invalid bet value");
            act.description = ACT_RISE;
            break;
        }
        case ACT_CHECK:
        {
            if(current_game_round == 0)
            {
                    eosio_assert(players[bb_index].name == player.name || 
                                 player.extra_bb_status == 1, "Wrong act");
                    eosio_assert( current_bet == small_blind*2, "Wrong act");
            }
            else
            {
                eosio_assert(current_bet.amount == 0, "Wrong act");
            }
            break;
        }
        case ACT_FOLD:
        {
            if(player.cur_round_bets.amount != 0)
                updateOutPlayerCurRoundBets(player, player_index);

            player.status = P_FOLD;
            current_folds_count++;
            break;
        }
        default:
            eosio_assert(false,"Wrong Act type.");
    }

    if(act.act_ == ACT_BET)
    {
        if(act.bet_ > current_bet)
        {
            current_bet = act.bet_;
        }
        table_cur_round_bets += act.bet_ - player.cur_round_bets;
        current_bank += act.bet_ - player.cur_round_bets;  
        current_round_players_bet_acts[player_index]++;
    }
    players_acts.push_back(PlayerAct(player_index, player.name, act));
}

void Player::clearGameInfo()
{
    cards_indexes.clear();
    acts.clear();
    cur_round_bets = eosio::asset(0, EOS_SYMBOL);
    all_in_bank = eosio::asset(0, EOS_SYMBOL);
    all_in_round = 0;
    all_in_flag = 0;
    count_of_acts = 0;
    have_event = 0;

    sum_of_bets = eosio::asset(0, EOS_SYMBOL);
    rake = eosio::asset(0, EOS_SYMBOL);
    late_trxs.clear();
    applied_late_trxs.clear();
}

void Player::addNewAct(const Act& act)
{
    acts.push_back(act);

    if( (act.act_ == ACT_BET) || (act.act_ == ACT_SMALL_BLIND) || (act.act_ == ACT_BIG_BLIND))
    {
        eosio::asset odds = eosio::asset(0,EOS_SYMBOL);
        if(act.bet_ > cur_round_bets)
            odds = act.bet_ - cur_round_bets;
        stack -= odds;
        cur_round_bets += odds;

        if(stack.amount == 0)
            all_in_flag = P_ALL_IN;

        sum_of_bets += odds;
    }
    count_of_acts++;
}

void Table::setLastTime()
{
    last_act_time = eosio::time_point(eosio::microseconds(current_time()));
    timestamp = last_act_time.time_since_epoch().count();
    /*Debug dbg;
    dbg.timestamp = timestamp;
    dbg.table_status = table_status;
    debug.push_back(dbg);*/
}

void Table::clearGameInfo()
{
    current_players_received_count = 0;
    current_game_round = 0;
    allin_players_count = 0;
    current_folds_count = 0;
    current_round_players_bet_acts.clear();
    players_with_bets.clear();

    current_bet = eosio::asset(0, EOS_SYMBOL);
    bank = eosio::asset(0, EOS_SYMBOL);
    table_cur_round_bets = eosio::asset(0, EOS_SYMBOL);
    not_returned_bets = eosio::asset(0, EOS_SYMBOL);
    current_bank = eosio::asset(0, EOS_SYMBOL);

    possible_moves.clear();
    raise_variants.clear();

    table_cards_indexes.clear();
    table_cards.clear();
    players_acts.clear();
    waiting_keys_indexes.clear();
    all_keys.clear();
    players_rsa_keys.clear();

    open_key.e.clear();
    open_key.n.clear();

    initTheDeckOfCards();

    for(Player& plr: players)
        plr.clearGameInfo();

    if(history.size() != 0)
        if(history.back().log.size() != 0)
            history.back().log.clear();
}

bool Table::checkMaxRaiseValue(const eosio::asset& value)
{
    eosio::asset max = players[next_player_index].stack + players[next_player_index].cur_round_bets;

    if(value >= max)
    {
        raise_variants.push_back(max);
        return true;
    }
    return false;
}

void Table::setRaiseVariants()
{
    raise_variants.clear();
    
    eosio::asset variant = small_blind*2;
   
    if(current_bet.amount != 0)
        variant = current_bet*2;

    while(true)
    {
        if(checkMaxRaiseValue(variant))
            return;

        raise_variants.push_back(variant);

        variant += small_blind;
    }
}

void Table::newGameStatistic()
{
    name contractname(CONTRACTNAME);

    statistic_index gamesstats(contractname,contractname.value);
    auto itr_stat = gamesstats.emplace(contractname, [&] (auto& game) {
        game.id = gamesstats.available_primary_key();
        game.start_time = last_act_time;
        game.small_blind = small_blind;
        game.players_count = current_game_players_count;
        game.bank = eosio::asset(0,EOS_SYMBOL);
        game.rake = eosio::asset(0,EOS_SYMBOL);
        game.status = R_IN_GAME;

        for(Player plr: players)
            if(plr.status == P_IN_GAME)
                game.players.push_back(plr.name);
    });

    game_id = (*itr_stat).id;
}

void Table::endGameStatistic() const
{
    name contractname(CONTRACTNAME);
    statistic_index gamesstats(contractname,contractname.value);

    auto itr_stats = gamesstats.find(game_id);
    eosio_assert(itr_stats != gamesstats.end(), "find game assertion");

    gamesstats.modify(itr_stats, contractname, [&] (auto& game){

        game.end_time = eosio::time_point(eosio::microseconds(current_time()));
        game.bank = history.back().bank;
        game.rake = history.back().bank_rake_asset;
        game.status = R_NORMAL;
        game.table_cards = table_cards;
        game.result_table_status = T_WAIT_END_GAME;
        game.players.clear();
        game.players_info = history.back().players_info;
    });
}

void Table::resettableGameStatistic() const
{
    name contractname(CONTRACTNAME);
    statistic_index gamesstats(contractname,contractname.value);
    auto itr_stats = gamesstats.find(game_id);
    eosio_assert(itr_stats != gamesstats.end(), "find game assertion");
    gamesstats.modify(itr_stats, contractname, [&] (auto& game){

        game.end_time = eosio::time_point(eosio::microseconds(current_time()));
        game.status = R_TIMEOUT_RESET;
        game.result_table_status = table_status;

        for(Player plr: players)
            if(plr.status == P_TIMEOUT)
                game.timeout_players.push_back(plr.name);
    });
}

void Table::deletetableGameStatistic() const
{
    name contractname(CONTRACTNAME);
    statistic_index gamesstats(contractname,contractname.value);
    auto itr_stats = gamesstats.find(game_id);
    eosio_assert(itr_stats != gamesstats.end(), "find game assertion");
    gamesstats.modify(itr_stats, contractname, [&] (auto& game){

        game.end_time = eosio::time_point(eosio::microseconds(current_time()));
        game.status = R_DEAD_TABLE_RESET;
        game.result_table_status = table_status;
    });
}

void Table::setNewInGameIndex(uint8_t& index, uint8_t offset)
{
    uint8_t not_checked_players_count = players.size();

    while(not_checked_players_count--)
    {
        if(++index == players.size())
            index = 0;

        if( (players[index].status == P_IN_GAME) ||
            (players[index].status == P_FOLD) ||
            (players[index].status == P_OUT) ||
            (players[index].status == P_TIMEOUT) )
            offset--;

        if(offset == 0)
            return;
    }
}

void Table::moveBigBlindIndex(uint8_t& index, uint8_t offset)
{
    uint8_t not_checked_players_count = players.size();

    while(not_checked_players_count--)
    {
        if(++index == players.size())
            index = 0;

        if( players[index].status == P_IN_GAME)
            offset--;
        else
        {
            if(offset == 1 && players[index].status == P_WAIT_NEW_GAME)
                offset--;
            else if( (offset == 2) && (players[index].status == P_WAIT_NEW_GAME && players[index].wait_for_bb == 0))
                offset--;
        }

        if(offset == 0)
            return;
    }
}

void Table::setCurrentGamePlayersCount()
{
    current_game_players_count = 0;
    for(Player plr: players)
        if(plr.status == P_IN_GAME)
            current_game_players_count++;
}

void Table::setPlayersCount()
{
    players_count = 0;
    for(Player plr: players)
    {
        if(plr.status == P_NO_PLAYER)
            continue;

        players_count++;
    }
}

void Table::cutNoPlayers()
{
    uint8_t last_player_index = 0;
    for(int i = 0; i< players.size(); i++)
    {
        if(players[i].status == P_NO_PLAYER)
            continue;
        last_player_index = i;
    }

    if(last_player_index + 1 < players.size())
        players.resize(last_player_index + 1);
}

void Table::setNoPlayersAndRefillStack()
{
    name contractname(CONTRACTNAME);
    account_index   accounts(contractname,contractname.value);

    for(Player& plr: players)
    {
        if(plr.status == P_NO_PLAYER || plr.status == P_WAIT_NEW_GAME)
            continue;

        auto itr_accounts = accounts.find((plr.name).value);
        eosio_assert(itr_accounts != accounts.end(), "find assertion");

        if((plr.status == P_TIMEOUT) || (plr.status == P_OUT))
        {
            if(plr.status == P_TIMEOUT)
            {
                accounts.modify(itr_accounts, contractname, [&] (auto& acnt){
                    acnt.addBalance(plr.stack);
                    acnt.out_reason = "Timeout";
                    if(acnt.table_id_.size() != 0)
                        if(acnt.table_id_.back() == id)
                            acnt.table_id_.clear();
                });
            }
            plr.status = P_NO_PLAYER;
            plr.clearGameInfo();
            plr.name.value = 0;
            continue;
        }

        if(plr.stack < small_blind*2)
        {
            eosio::asset odt = small_blind*(*itr_accounts).buyin_sb - plr.stack;

            if( ((*itr_accounts).autorebuy == 1) && (*itr_accounts).quantity_ >= odt)
            {
                if(odt.amount > 0)
                {
                    accounts.modify(itr_accounts, contractname, [&] (auto& acnt){
                        acnt.quantity_ -= odt;
                    });
                    plr.stack += odt;
                }
            }
            else // delete from table
            {
                accounts.modify(itr_accounts, contractname, [&] (auto& acnt){
                    acnt.quantity_ += plr.stack;
                    acnt.out_reason = "Stack less than big blind";
                    if(acnt.table_id_.size() != 0)
                        if(acnt.table_id_.back() == id)
                            acnt.table_id_.clear();
                });
                plr.status = P_NO_PLAYER;
                plr.clearGameInfo();
                plr.name.value = 0;
                continue;
            }
        }

        if(plr.status == P_FOLD)
            plr.extra_bb_status = 0;

        plr.status = P_IN_GAME;
        
        if(history.size() != 0)
        {
            // new extra bb player already paid and no have timeout - save extra_bb_status to exclude repeated paid
            if(history.back().result == R_TIMEOUT_RESET && plr.extra_bb_status == 1)
                plr.status = P_WAIT_NEW_GAME;
            else
                plr.extra_bb_status = 0;
        }

        plr.start_stack = plr.stack;
    }
}

void Table::setExtraBBPlayers()
{
    uint8_t plr_index = 0;
    for(Player& plr: players)
    {
        if(plr.status == P_WAIT_NEW_GAME && plr.wait_for_bb == 0)
        {
            plr.extra_bb_status = 1;
            plr.status = P_IN_GAME;
            plr.start_stack = plr.stack;
        }
        plr_index++;
    }
}

bool Table::zeroPlayers()
{
    if(players_count == 0)
    {
        setTableStatus(T_DELETE);
        return true;
    }
    return false;
}

bool Table::onlyOnePlayer()
{
    if(players_count == 1)
    {
        Player the_one;
        for(Player& plr: players)
        {
            if(plr.status == P_IN_GAME || plr.status == P_WAIT_NEW_GAME)
            {
                the_one = plr;
                break;
            }
        }
        players[0] = the_one;
        players[0].status = P_WAIT_NEW_GAME;
        players.resize(1);
        dealer_index = sb_index = bb_index = next_player_index = 0;
        setTableStatus(T_WAIT_PLAYER);
        current_game_players_count = 0;
        game_id = std::numeric_limits<uint64_t>::max();
        return true;
    }
    return false;
}

void Table::moveDealerIndex()
{
   uint8_t not_checked_players_count = players.size();

    while(not_checked_players_count--)
    {
        if(++dealer_index == players.size())
            dealer_index = 0;

        if(players[dealer_index].status == P_IN_GAME) 
            break;
    }
}

void Table::setDealerIndex(bool move_dealer)
{
    if(current_game_players_count == 0)
    {
        dealer_index = 0;
        for(Player plr: players)
        {
            if(plr.status == P_WAIT_NEW_GAME)
                return;
            dealer_index++;
        }
    }

    if(players[dealer_index].status == P_NO_PLAYER || move_dealer)
    {
        moveDealerIndex();
    }
}

void Table::initNewGame(bool move_dealer)
{
    eosio::print(" initNewGame.");
    setLastTime();
    clearGameInfo();
    setNoPlayersAndRefillStack();
    setPlayersCount();

    if(zeroPlayers())
    {
        eosio::print(" end. Zero players.");
        return;
    }

    if(onlyOnePlayer())
    {
        eosio::print(" end. one player.");
        return;
    }

    current_round_players_bet_acts.resize(players.size());
    setExtraBBPlayers();
    setCurrentGamePlayersCount();
    setDealerIndex(move_dealer);
    
    uint8_t prev_bb = bb_index;
    sb_index = bb_index = dealer_index;
    moveBigBlindIndex(bb_index, 2);

    if(players[dealer_index].status == P_WAIT_NEW_GAME /* no players from prev game. new dealer */||
       current_game_players_count == 1 /* take one BB from wait_bb */ )
    {
        eosio::print(" heads up game with new player(s)");
        players[dealer_index].status = P_IN_GAME;
        players[dealer_index].start_stack = players[dealer_index].stack;
        bb_index = dealer_index;
        moveBigBlindIndex(bb_index, 1);
        players[bb_index].status = P_IN_GAME;
        players[bb_index].start_stack = players[bb_index].stack;
        current_game_players_count = 2;
    }
    else if(current_game_players_count == 2 && players[bb_index].status == P_IN_GAME) /* no wait_bb in BB position */
    {
        eosio::print(" heads up game with same players.");
        bb_index = dealer_index;
        setNewInGameIndex(bb_index, 1);
        if( (prev_bb == bb_index) && (move_dealer == true))
        {
            eosio::print(" same bb_index but not reseted game. jump dealer and set new.");
            bb_index = sb_index;
            sb_index = dealer_index = prev_bb;
        }
    }
    else
    {
        eosio::print(" >2 players game");
        setNewInGameIndex(sb_index, 1);

        if( (prev_bb == bb_index) && (move_dealer == true) )
        {
            eosio::print(" same bb_index but not reseted game. jump dealer and set new.");
            moveDealerIndex();
            sb_index = dealer_index;
            setNewInGameIndex(sb_index, 1);
            bb_index = dealer_index;
            moveBigBlindIndex(bb_index, 2);
        }

        if(players[bb_index].status == P_WAIT_NEW_GAME)
        {
            players[bb_index].status = P_IN_GAME;
            players[bb_index].start_stack = players[bb_index].stack;
            current_game_players_count++;
        }
    }
    next_player_index = dealer_index;

    cutNoPlayers();
    newGameStatistic();
    setCardsIndexesToPlayers();
    setTableStatus(T_WAIT_START_GAME);
    
eosio::print(" end init game. np =", players[next_player_index].name," sb=",(int)sb_index, " bb=", (int)bb_index);
}

void Table::addNewPlayer(const eosio::name& name, const eosio::asset& stack, uint8_t wait_for_bb)
{
    eosio_assert(++players_count <= max_players, "The table is full");

    Player new_player(name);
    new_player.stack = stack;
    new_player.wait_for_bb = wait_for_bb;

    for(Player& plr: players)
        if(plr.status == P_NO_PLAYER)
        {            
            plr = new_player;
            return;
        }

    players.push_back(new_player);
}

void Table::setNewDeck(const std::vector<Card>& cards)
{
    the_deck_of_cards = cards;
    setLastTime();
}

void Table::setCardsIndexesToPlayers()
{
    uint8_t count_players = current_game_players_count;
    uint8_t card_index = 0;
    uint8_t i = next_player_index;

    while(count_players--)
    {
        waiting_keys_indexes.push_back(card_index);
        players[i].cards_indexes.push_back(card_index++);
        waiting_keys_indexes.push_back(card_index);
        players[i].cards_indexes.push_back(card_index++);
        setNewInGameIndex(i, 1);
    }
}

bool Table::checkEndGame() const
{
    if(current_folds_count + 1 >= current_game_players_count)
            return true;

    return false;
}

bool Table::checkEndAllInGame() const
{
    if(players_acts.back().act_.act_ == ACT_NEW_ROUND)
    {
        if( current_folds_count + allin_players_count + 1 >= current_game_players_count)
            return true;
    }
    return false;
}

void Table::update_players_with_bets()
{
    uint8_t count_players_with_bet = 0;
    for(uint8_t count: current_round_players_bet_acts)
        if(count != 0)
            count_players_with_bet++;

    current_round_players_bet_acts.clear();
    current_round_players_bet_acts.resize(players.size());

    players_with_bets.push_back(count_players_with_bet);
}

void Table::setNewRoundAct()
{
    name contractname(CONTRACTNAME);
    Act act = Act(ACT_NEW_ROUND, eosio::asset(0, EOS_SYMBOL));
    players_acts.push_back(PlayerAct(13,contractname,act));
    bank += table_cur_round_bets;
    current_bank = bank;
    table_cur_round_bets.amount = 0;
    current_bet.amount = 0;

    for(Player& plr: players)
    {
        if(plr.status == P_WAIT_NEW_GAME || plr.status == P_NO_PLAYER)
            continue;

        plr.count_of_acts = 0;
        plr.cur_round_bets.amount = 0;
    }
    update_players_with_bets();
}

void Table::setPossibleMoves()
{
    possible_moves.clear();
    possible_moves.push_back(ACT_FOLD);
    possible_moves.push_back(ACT_BET);  
        
    if(players[next_player_index].cur_round_bets < current_bet)
        if(players[next_player_index].stack > current_bet - players[next_player_index].cur_round_bets)
            possible_moves.push_back(ACT_CALL);

    if(current_bet.amount == 0)
    {
        possible_moves.push_back(ACT_CHECK);
    }

    if(current_game_round == 0)
    {
        if(next_player_index == bb_index) 
        {
            if(players[next_player_index].count_of_acts == 1) //only BB
                if( current_bet == small_blind*2)
                {
                    possible_moves.push_back(ACT_CHECK);
                    return;
                }
        }
        else if(players[next_player_index].extra_bb_status == 1)
        {
            if(current_bet == small_blind*2 && 
               players[next_player_index].cur_round_bets == current_bet)
                possible_moves.push_back(ACT_CHECK);
        }
    }
}

uint8_t Table::setNextPlayerIndex()
{
    eosio::print(" setNextPlayerIndex() ");
    if(checkEndGame())
    {
        bank += table_cur_round_bets;
        current_bank = bank;
        update_players_with_bets();
        setTableStatus(T_END_GAME);
        eosio::print(" return end_game");
        return T_END_GAME;
    }

    uint8_t cur_players = current_game_players_count;

    if(checkEndAllInGame())
    {
        eosio::print(" return end_allin_game");
        return T_END_ALL_IN_GAME;
    }

    while(--cur_players)
    {
        setNewInGameIndex(next_player_index, 1);

        if( (players[next_player_index].all_in_flag == P_ALL_IN && players[next_player_index].status == P_IN_GAME) ||  
            (players[next_player_index].status == P_FOLD) ||
            (players[next_player_index].status == P_OUT) || 
            (players[next_player_index].status == P_TIMEOUT))
        {
            continue;
        }

        if(players[next_player_index].count_of_acts == 0)
        {
            break;
        }

        if((current_game_round == 0) && players[next_player_index].count_of_acts == 1 &&
           players[next_player_index].extra_bb_status == 1 &&
           next_player_index != sb_index &&
           next_player_index != bb_index)
        {
            break;
        }

        // если его ставку перебили
        if(players[next_player_index].cur_round_bets < current_bet)
        {
            break;
        }

        // если на префлопе никто не повысил, то ББ может походить
        if( (current_game_round == 0) && (next_player_index == bb_index) )
        {
            if(players[next_player_index].count_of_acts == 1) //only BB
                if( current_bet == small_blind*2)
                {
                    break;
                }
        }

        // если на префлопе extra BB в позиции SB.
        if( (current_game_round == 0) && (next_player_index == sb_index) && players[next_player_index].extra_bb_status == 1)
        {
            if(players[next_player_index].count_of_acts == 1) //only SB
                {
                    break;
                }
        }

    } // while( current_game_players_count )

    eosio::print(" np = ",players[next_player_index].name);

    if(cur_players) // found one!
    {
        setRaiseVariants();
        setPossibleMoves();
        eosio::print(" return wait_player_act");
        return T_WAIT_PLAYERS_ACT;
    }

    // новый раунд
    setNewRoundAct();
    eosio::print(" return act_new_round");
    return ACT_NEW_ROUND;
}

void Table::jobSetNextPlayerIndex()
{
    setLastTime();
    uint8_t res = setNextPlayerIndex();
    if(res == T_END_GAME)
    {
        endGame();
    }
    if(res == T_END_ALL_IN_GAME)
    {
        actAllInKeys();
    }
    if(res == ACT_NEW_ROUND)
    {
        actMasterShowDown();
    }
}

void Table::actMasterBlind()
{
    next_player_index = sb_index;

    if(players[next_player_index].extra_bb_status != 1 || current_game_players_count == 2)
    {
        Act sb = Act(ACT_SMALL_BLIND, small_blind);
        players[next_player_index].addNewAct(sb);    
        players_acts.push_back(PlayerAct(next_player_index, players[next_player_index].name, sb));
        
        current_round_players_bet_acts[next_player_index]++;
        current_bet = sb.bet_;
    
        table_cur_round_bets += sb.bet_;
        current_bank += sb.bet_;
        not_returned_bets += small_blind;
    }

    next_player_index = bb_index;

    uint8_t count = current_game_players_count;
    uint8_t i = next_player_index;

    while(count--)
    {
        if( (i == next_player_index) || 
            (players[i].extra_bb_status == 1) )
        {
            if(i == sb_index && current_game_players_count == 2)
            {
                // like continue
                // no extra_bb in HU game for SB
            }
            else
            {
                Act bb = Act(ACT_BIG_BLIND, small_blind*2);
                players[i].addNewAct(bb);
                if(players[i].all_in_flag == P_ALL_IN)
                {
                    players[i].all_in_bank = bank;
                    players[i].all_in_round = current_game_round;
                    allin_players_count++;
                }
                players_acts.push_back(PlayerAct(i, players[i].name, bb));
                current_round_players_bet_acts[i]++;        
    
                table_cur_round_bets += bb.bet_;
                current_bank += bb.bet_;
                not_returned_bets += small_blind*2; 
            }
        }
        setNewInGameIndex(i, 1);
    }

    current_bet = small_blind*2;

    setNextPlayerIndex();

    setLastTime();
    setTableStatus(T_WAIT_KEYS_FOR_PLAYERS);
}

void Table::actAllInKeys()
{
    waiting_keys_indexes.clear();
    uint8_t last_index = current_game_players_count*2 + table_cards.size();
    for(int i = last_index; i < 52; i++)
        waiting_keys_indexes.push_back(i);

    setTableStatus(T_WAIT_ALLIN_KEYS);
}

void Table::actMasterShowDown()
{
    switch(current_game_round)
    {
        case 0:
        {
            if(table_cards_indexes.empty())
            {
                uint8_t index = current_game_players_count * 2;
                waiting_keys_indexes.clear();
                for(int i = 0; i< 3; i++)
                {
                    table_cards_indexes.push_back(index + i);
                    waiting_keys_indexes.push_back(index + i);
                }

                setTableStatus(T_WAIT_KEYS_FOR_SHOWDOWN);
            }
            else
            {
                for(int i = 0; i < table_cards_indexes.size(); i++)
                    table_cards.push_back(the_deck_of_cards[table_cards_indexes[i]]);

                setTableStatus(T_WAIT_PLAYERS_ACT);
                next_player_index = dealer_index;
                uint8_t res = setNextPlayerIndex();
                if(res == T_END_GAME)
                    endGame();
                if(res == T_END_ALL_IN_GAME)
                    actAllInKeys();
                else
                    current_game_round++; 
            }
            break;
        }
        case 1:
        case 2:
        {
            if( table_cards_indexes.size() == table_cards.size() )
            {
                waiting_keys_indexes.clear();
                uint8_t index = current_game_players_count * 2 + table_cards_indexes.size();
                table_cards_indexes.push_back(index);
                waiting_keys_indexes.push_back(index);
                setTableStatus(T_WAIT_KEYS_FOR_SHOWDOWN);
            }
            else
            {
                table_cards.push_back(the_deck_of_cards[table_cards_indexes.back()]);
                
                setTableStatus(T_WAIT_PLAYERS_ACT);
                next_player_index = dealer_index;
                uint8_t res = setNextPlayerIndex();
                if(res == T_END_GAME)
                    endGame();
                else if(res == T_END_ALL_IN_GAME)
                    actAllInKeys();
                else
                    current_game_round++;
            }
            break;
        }
        case 3:
        {
            // this is end of the game
            uint8_t last_index = current_game_players_count*2 + table_cards.size();
            for(int i = last_index; i < 52; i++)
                waiting_keys_indexes.push_back(i);
            setTableStatus(T_WAIT_ALL_KEYS);
            break;
        }
    }
    setLastTime();
}

void Table::saveOneWinnerHistory(GameResult& res)
{
    uint8_t count = current_game_players_count;
    uint8_t i = next_player_index;

    while(count--)
    {
        PlayerHistoryInfo player_history;
        player_history.name = players[i].name;
        player_history.winnings = eosio::asset(0, EOS_SYMBOL);
        
        if(players[i].status == P_IN_GAME)
            player_history.winnings = res.bank;

        res.players_info.push_back(player_history);
        setNewInGameIndex(i, 1);
    }

    // sort by winners
    std::sort(res.players_info.begin(), res.players_info.end(), [](const PlayerHistoryInfo& a, const PlayerHistoryInfo& b) -> bool {
        return a.winnings > b.winnings;
    });
}

uint8_t Table::getCountOfWinners(const std::vector<PlayerHistoryInfo>&  bidders)
{
    uint8_t count_of_winners = 1;

    for(auto itr = bidders.begin(); itr != bidders.end(); itr++)
        {
            auto next_itr = std::next(itr, 1);
            if( next_itr == bidders.end() )
                break;

            if( (*itr) > (*next_itr)  )
                break;
            count_of_winners++;
    }

    return count_of_winners;
}

void Table::calculateWinners(eosio::asset bank_size, std::vector<PlayerHistoryInfo>&  bidders, bool all_in)
{
    int count_of_winners = getCountOfWinners(bidders);
    eosio::asset winner_prize = bank_size/count_of_winners;

    for(int i=0;i<bidders.size();i++)
    {
        if(count_of_winners == 0)
        {
            winner_prize.amount = 0;
        }
        bidders[i].winnings += winner_prize;

        if(all_in)
        {
            SidePot side_pot;
            side_pot.bank = bank_size;
            side_pot.win = winner_prize;
            bidders[i].side_pots.push_back(side_pot);
        }
        count_of_winners--;
    }
}

void Table::getAllInSortedPlayers(std::vector<Player>& sorted_players)
{
    // ALL_INN players first
    for(Player plr: players)
        if(plr.all_in_flag == P_ALL_IN && plr.status != P_OUT)
        {
            sorted_players.push_back(plr);
        }

    std::sort(sorted_players.begin(), sorted_players.end(), [](const Player& a, const Player& b) -> bool {
        eosio_assert(a.acts.size() != 0, "player a acts size=0");
        eosio_assert(b.acts.size() != 0, "player b acts size=0");
        if(a.all_in_bank.amount == b.all_in_bank.amount)
        {
            return a.acts.back().bet_.amount < b.acts.back().bet_.amount ;
        }
        return a.all_in_bank.amount < b.all_in_bank.amount;
    });

    // other players
    for(Player plr: players)
    {
        if( (plr.all_in_flag != P_ALL_IN) && 
            (plr.status == P_IN_GAME))                
                sorted_players.push_back(plr);
    }
}

void Table::getComboSortedPlayers(const std::vector<Player>& unsorted, std::vector<PlayerHistoryInfo>& comboSortedPlayers)
{
    uint8_t table_card_index = current_game_players_count*2;
    table_cards.clear();
    for(int i=0;i<5;i++)
    {
        table_cards.push_back(the_deck_of_cards[table_card_index]);
        table_card_index++;
    }

    for(const Player& plr: unsorted)
    {
        PlayerHistoryInfo info;
        info.name = plr.name;
        info.winnings = eosio::asset(0, EOS_SYMBOL);

        if((plr.status != P_FOLD) && (plr.status != P_OUT) && (plr.status != P_TIMEOUT))
        {
            info.hand.push_back(the_deck_of_cards[plr.cards_indexes[0]]);
            info.hand.push_back(the_deck_of_cards[plr.cards_indexes[1]]);

            std::multiset<Card> cards;
            for(uint8_t card_index: plr.cards_indexes)
                cards.insert(the_deck_of_cards[card_index]);
        
            for(Card card: table_cards)
                cards.insert(card);

            int get_combo_res = getCombination(cards, info.combo);
            eosio_assert(get_combo_res,"error get combination");
            if(plr.all_in_flag != 0)
            {
                if(plr.status == P_IN_GAME)
                    info.show = 1;
            }
        }

        comboSortedPlayers.push_back(info); 
    }

    std::sort(comboSortedPlayers.begin(), comboSortedPlayers.end(), [](const PlayerHistoryInfo& a, const PlayerHistoryInfo& b) -> bool{
        return a > b;
    });
}

bool Table::decryptCardByOneKey(Card& card, Key& key)
{
    unsigned char in_data[8];
    memset(in_data,0,8);
    in_data[0] = card.suit;
    in_data[1] = card.value;

    unsigned int sync[2];
    memcpy(&sync[0], &key.s[0], 8);

    unsigned int key_int[8];
    memcpy(&key_int[0],&key.data[0], 32);

    unsigned char* text = decrypt_data1(in_data, &key_int[0], &sync[0]);
        
    card.suit = text[0];
    card.value = text[1];

    freeMem(text);   

    return true;
}

bool Table::decryptCardByAllKeys(Card& card, int card_index)
{
    int decrypt_count = 0;
    for(int i = 0; i<all_keys.size(); i++)
    {
        if(card_index != all_keys[i].card_index)
            continue;

        decryptCardByOneKey(card, all_keys[i]);

        decrypt_count++;
        if(decrypt_count == current_game_players_count)
            break;
    }
    eosio_assert(decrypt_count == current_game_players_count, "wrong decrypt keys count");

    return true;
}

void Table::decryptPlayersCards()
{
    uint32_t count_cards = current_game_players_count * 2;
    std::vector<bool> not_decrypt(count_cards);
    not_decrypt.assign(count_cards, false);

    for(Player plr: players)
        if((plr.status == P_OUT) || (plr.status == P_FOLD) || (plr.status == P_TIMEOUT))
        {
            not_decrypt[plr.cards_indexes[0]] = true;
            not_decrypt[plr.cards_indexes[1]] = true;
        }

    for(int i = 0; i < count_cards; i++)
    {
        if(not_decrypt[i])
            continue;

        decryptCardByAllKeys(the_deck_of_cards[i], i);
    }
}

// ищем в players_with_bets последний раунд с ненулевыми ставками - только его надо проверить.
// собираем все ставки этого раунда в сортированый вектор и сравниваем две последние.
// если они не равны, тогда возвращаем разницу.
void Table::returnBetsOdds()
{
    int last_bet_round = -1;
    for(int i = 0; i < players_with_bets.size(); i++)
    {
        if(players_with_bets[i] == 0)
            continue;

        last_bet_round = i;
    }
    eosio_assert(last_bet_round != -1, " Error get last bet round ");

    eosio:asset max_ingame_player_bet = eosio::asset(0, EOS_SYMBOL);
    std::vector<PlayerAct> all_bets_in_round; // with timeout players

    int cur_round = 0;
    for(int i = 0; i< players_acts.size(); i++)
    {
        if(players_acts[i].act_.act_ == ACT_BET || players_acts[i].act_.act_ == ACT_SMALL_BLIND || players_acts[i].act_.act_ == ACT_BIG_BLIND)
            if(cur_round == last_bet_round)
            {
                if(players[players_acts[i].player_index].status == P_IN_GAME)
                {
                    if(players_acts[i].act_.bet_ > max_ingame_player_bet)
                        max_ingame_player_bet = players_acts[i].act_.bet_;
                }

                all_bets_in_round.push_back(players_acts[i]);
             }

        if(players_acts[i].act_.act_ == ACT_NEW_ROUND)
        {
            if(cur_round == last_bet_round)
                break;
            cur_round++;
        }
    }

    if(rsa_key_flag == 1)
    {
        eosio::print(" check timeout players ");
        std::vector<uint8_t> timeout_indexes;

        for(PlayerAct& act: all_bets_in_round)
            eosio::print(" act.bet=",act.act_.bet_);

        for(PlayerAct& act: all_bets_in_round)
        {
            if(players[act.player_index].status == P_TIMEOUT)
            {
                eosio::print(" got one! name=", players[act.player_index].name);
                bool already_checked = false;
                for(uint8_t tindex: timeout_indexes)
                    if(act.player_index == tindex)
                    {
                        already_checked = true;
                        break;
                    }

                if(already_checked)
                {
                    eosio::print(". Repeated-find next ");
                    continue;
                }

                timeout_indexes.push_back(act.player_index);

                eosio::asset timeout_plr_max_bet = act.act_.bet_;
                int cur_index=0, max_bet_index=0;
                for(PlayerAct try_max_bet: all_bets_in_round)
                {
                    if(try_max_bet.player_index == act.player_index)
                        if(try_max_bet.act_.bet_ >= timeout_plr_max_bet)
                        {
                            timeout_plr_max_bet = try_max_bet.act_.bet_;
                            max_bet_index = cur_index;
                        }
                    cur_index++;
                }

                if(timeout_plr_max_bet > max_ingame_player_bet)
                {
                    eosio::asset odd = timeout_plr_max_bet - max_ingame_player_bet;
                    eosio::print(" for return=", odd);
                    players[act.player_index].stack += odd;
                    players[act.player_index].acts.back().bet_-= odd;
                    players[act.player_index].sum_of_bets -= odd;
                    bank -= odd;
                    current_bank -= odd;

                    all_bets_in_round[max_bet_index].act_.bet_-= odd;
                }
            }
        }
        eosio::print(" Check complited ");
    }
    
    eosio_assert(all_bets_in_round.size() >= 1, " Error bets_in_round size ");

    std::sort(all_bets_in_round.begin(), all_bets_in_round.end(), [](const PlayerAct& a, const PlayerAct& b) -> bool {
        return a.act_.bet_.amount < b.act_.bet_.amount;
    });

    for(PlayerAct& act: all_bets_in_round)
            eosio::print(" act.bet=",act.act_.bet_);

    int max_player_index = all_bets_in_round[all_bets_in_round.size()-1].player_index;
    eosio::asset max_bet = all_bets_in_round[all_bets_in_round.size()-1].act_.bet_;

    if( (all_bets_in_round.size()) == 1 )
    {
        if(players[max_player_index].all_in_flag != P_ALL_IN)
        {
            players[max_player_index].stack += max_bet;
            players[max_player_index].acts.back().bet_-= max_bet;
            players[max_player_index].sum_of_bets -= max_bet;
            bank -= max_bet;
            current_bank -= max_bet;
        }
        return;
    }

    eosio::asset prev_bet = all_bets_in_round[all_bets_in_round.size()-2].act_.bet_;

    if(prev_bet.amount != max_bet.amount)
    {
        if(all_bets_in_round[all_bets_in_round.size()-2].player_index != all_bets_in_round[all_bets_in_round.size()-1].player_index)
        {
            eosio::asset odd = max_bet - prev_bet;
            players[max_player_index].stack += odd;
            players[max_player_index].acts.back().bet_-= odd;
            players[max_player_index].sum_of_bets -= odd;
            bank -= odd;
            current_bank -= odd;
        }
    }
}

void Table::saveAllInHistory(GameResult& res)
{
    decryptPlayersCards();

    std::vector<Player> allInSortedPlayers, allInSortedPlayers2;
    std::vector<PlayerHistoryInfo>  comboSortedPlayers, comboSortedPlayers2;
   
    getAllInSortedPlayers(allInSortedPlayers);
    for(auto inf: allInSortedPlayers)
        eosio::print(" allin name: ",inf.name);
    getComboSortedPlayers(allInSortedPlayers, comboSortedPlayers);
    for(auto inf: comboSortedPlayers)
        eosio::print(" combo name: ",inf.name);

    eosio::asset total_bank = res.bank;
    eosio::asset prev_rounds_bank = eosio::asset(0,EOS_SYMBOL);;
    eosio::asset already_used_bank = eosio::asset(0,EOS_SYMBOL);;
    eosio::asset prev_all_in_bet = eosio::asset(0,EOS_SYMBOL);; // in one round!
    eosio::asset all_in_bet = eosio::asset(0,EOS_SYMBOL);;
    int all_in_round = -1;
    int prev_all_in_round = -1;
    uint8_t prev_all_in_players = 0; // in one round!
    bool all_in_flag = false;

    uint8_t all_in_players = allin_players_count;

    while(all_in_players != 0)
    {
        all_in_flag = true;
        all_in_bet = allInSortedPlayers.begin()->acts.back().bet_;
        all_in_round = allInSortedPlayers.begin()->all_in_round;
        uint8_t round_bets_count = players_with_bets[all_in_round];

        if(prev_all_in_round == all_in_round) // have another all_in in the same round!
        {
            all_in_bet -= prev_all_in_bet;
            round_bets_count -= prev_all_in_players;
        }
        else
        {
            prev_all_in_bet = eosio::asset(0,EOS_SYMBOL);
            already_used_bank = eosio::asset(0,EOS_SYMBOL);
            prev_all_in_players = 0;
        }

        eosio::asset bank_size = allInSortedPlayers.begin()->all_in_bank + all_in_bet * round_bets_count;
        bank_size -= already_used_bank;

        if(prev_all_in_round != all_in_round)
            bank_size -= prev_rounds_bank;

        int bank_rake_size_amount = (float)(bank_size.amount)*res.rake_percent/100;

        bank_size = bank_size - eosio::asset(bank_rake_size_amount, EOS_SYMBOL);

        if(bank_size > total_bank)
            bank_size = total_bank;

        eosio::print(" bank_size=",bank_size);

        calculateWinners(bank_size, comboSortedPlayers, all_in_flag);

        if((prev_all_in_round != all_in_round) && (all_in_round != 0))
            already_used_bank += allInSortedPlayers.begin()->all_in_bank;

        total_bank -= bank_size;
        prev_all_in_round = all_in_round;
        prev_all_in_bet += all_in_bet;
        prev_rounds_bank += bank_size + eosio::asset(bank_rake_size_amount, EOS_SYMBOL);

        // move all_in player from comboSortedPlayers to res.players_info
        uint8_t deleted_count = 0;
        for(auto itr = allInSortedPlayers.begin(); itr != allInSortedPlayers.end();itr++)
        {
            // delete from comboSortedPlayers
            for( auto itr_for_delete = comboSortedPlayers.begin(); itr_for_delete != comboSortedPlayers.end(); itr_for_delete++)
            {
                if( (*itr).name == (*itr_for_delete).name )
                {
                    eosio::print(" res.push_back=",(*itr_for_delete).name);
                    res.players_info.push_back(*itr_for_delete);
                }
                else
                    comboSortedPlayers2.push_back(*itr_for_delete);
            }
            comboSortedPlayers = comboSortedPlayers2;
            comboSortedPlayers2.clear();

            all_in_players--;
            prev_all_in_players++;
            deleted_count++;

            auto itr_next = std::next(itr, 1);
            if(itr_next == allInSortedPlayers.end())
                break; // move all_in

            if((*itr_next).all_in_flag == P_ALL_IN )
                if((*itr).all_in_bank == (*itr_next).all_in_bank)
                    if((*itr).acts.back().bet_ == (*itr_next).acts.back().bet_)
                    {
                        eosio::print(" same_allin!");
                        continue; // same ALL_IN
                    }
            break;// move all_in
        }

        eosio::print(" deleted_count=",(int)deleted_count);
        for(auto itr = allInSortedPlayers.begin(); itr != allInSortedPlayers.end();itr++)
        {
            eosio::print(" (*itr).name=",(*itr).name);
            if(deleted_count != 0)
            {
                deleted_count--;
                continue;
            }
            allInSortedPlayers2.push_back(*itr);
        }
        allInSortedPlayers = allInSortedPlayers2;
        allInSortedPlayers2.clear();
    }

    eosio::print(" comboSortedPlayers.size()=",comboSortedPlayers.size());

    if( comboSortedPlayers.size() != 0 )
    {
        // если осталось что ещё разыгрывать
        if( (comboSortedPlayers.size() >= 1) && (total_bank.amount != 0) )
        {
            calculateWinners(total_bank, comboSortedPlayers, all_in_flag);
        }

        for(PlayerHistoryInfo info: comboSortedPlayers)
        {
           res.players_info.push_back(info); 
        }
    }
    
    //  add P_FOLD
    uint8_t count = current_game_players_count;
    uint8_t i = next_player_index;

    while(count--)
    {
        if((players[i].status == P_FOLD) || (players[i].status == P_OUT) || (players[i].status == P_TIMEOUT))
        {
            bool already_have=false;
            if(players[i].status == P_TIMEOUT)
            {
                for(auto inf: res.players_info)
                    if(players[i].name == inf.name)
                    {
                        already_have = true;
                        break;
                    }
            }

            if(already_have == false)
            {
                PlayerHistoryInfo info;
                info.name = players[i].name;
                info.winnings = eosio::asset(0, EOS_SYMBOL);
                eosio::print(" res.push_back=",info.name);
                res.players_info.push_back(info);
            }
        }
        setNewInGameIndex(i, 1);
    }

    // last sort by winners
    std::sort(res.players_info.begin(), res.players_info.end(), [](const PlayerHistoryInfo& a, const PlayerHistoryInfo& b) -> bool {
        return a.winnings > b.winnings;
    });
}

void Table::endResetGame(eosio::asset& plr_fine_part)
{
    GameResult res;
    res.result = R_TIMEOUT_RESET;

    for(Player& plr: players)
    {
        if(plr.status != P_IN_GAME)
            continue;

        PlayerHistoryInfo info;
        info.name = plr.name;
        info.winnings = plr_fine_part;
        res.players_info.push_back(info);
    }

    if(history.size()!=0)
        res.log = history.back().log;
    history.clear();
    history.push_back(res);

    // FOR SENDENDGAME WAIT
    setEventsFromOutPlayers();

    setLastTime();
    setTableStatus(T_WAIT_END_GAME);
}

void Table::setShowDown(GameResult& res)
{
    for(PlayerHistoryInfo& info: res.players_info)
    {
        if(info.winnings.amount != 0)
        {
            info.show = 1;
            continue;
        }
    }

    uint8_t start_index = dealer_index;
    eosio::print(" start_index=",(int)start_index);
    setNewInGameIndex(start_index, 1);
    eosio::asset max_bet = eosio::asset(0, EOS_SYMBOL);

    // если сейчас последний раунд
    //if(current_game_round == 3)
    {
        // ищем агрессора
        for(auto it = players_acts.rbegin()+1; it != players_acts.rend(); it++)
        {
            if(it->act_.act_ == ACT_NEW_ROUND)
                break;

            if(it->act_.act_ == ACT_BET)
            {
                if(it->act_.bet_ > max_bet)
                    max_bet = it->act_.bet_;
            }

            if(it->act_.description == ACT_RISE || it->act_.description == ACT_BET)
            {
                if(it->act_.bet_ == max_bet)
                {
                    start_index = it->player_index;
                    eosio::print(" have agressor_index=",(int)start_index);
                    break;
                }
            }

            if(it->act_.description == ACT_ALLIN)
            {
                if(it->act_.bet_ == max_bet)
                {
                    start_index = it->player_index;
                    eosio::print(" probably have agressor_index=",(int)start_index);
                }
            }
        }

        std::vector<PlayerHistoryInfo>  players_info;
        uint8_t allin_count = 0;
        uint8_t count = res.players_info.size();
        while(count)
        {
            for(PlayerHistoryInfo info: res.players_info)
                if(info.name == players[start_index].name)
                {
                    if(players[start_index].all_in_flag == P_ALL_IN)
                        allin_count++;
                    players_info.push_back(info);
                    setNewInGameIndex(start_index, 1);
                    count--;
                    break;
                }
        }


        uint8_t combos_count = 0;

        if(players_info[0].combo.type != C_NO_COMBINATION) 
        {
            players_info[0].show = 1;
            combos_count++;
        }

        Combination best_prev_combo = players_info[0].combo;

        for(int i=1; i< players_info.size(); i++)
        {
            if(players_info[i].combo.type != C_NO_COMBINATION)
                combos_count++;

            if( players_info[i].winnings.amount !=0 || 
                players_info[i].combo > best_prev_combo)
                {
                    players_info[i].show = 1;
                    best_prev_combo = players_info[i].combo;
                }
                else
                {
                    bool prev_combo_better = best_prev_combo > players_info[i].combo;
                    if(prev_combo_better == false)
                    {
                        players_info[i].show = 1;
                        best_prev_combo = players_info[i].combo;
                    }
                }
        }

        if(combos_count - allin_count == 1)
        {
            for(int i=1; i< players_info.size(); i++)
            {
                if(players_info[i].combo.type != C_NO_COMBINATION)
                    players_info[i].show = 1;
            }
        }

        res.players_info = players_info;
    }
}

void Table::endGame()
{
    eosio::print(" IN_END_GAME");
     
    // FOR SENDENDGAME WAIT
    setEventsFromOutPlayers();

    name contractname(CONTRACTNAME);
    global_state_singleton global(contractname,contractname.value);
    globalstate gstate = global.get();
    global_fine_singleton global_fine(contractname,contractname.value);
    globalfine gfine = global_fine.get();

    uint8_t have_rake = 1;
    float rake_percent = 0;
    eosio::asset bank_rake_asset = eosio::asset(0, EOS_SYMBOL);
    
    if(table_cards.size() == 0)
        have_rake = 0;

    if(current_game_round == 0 && current_bank == not_returned_bets)
        {}
    else
        returnBetsOdds();

    if(have_rake != 0)
    {
        rake_percent = gstate.rake_percent;
        int bank_rake_asset_amount = round((float)(current_bank.amount)*gstate.rake_percent/100);
        if(bank_rake_asset_amount > gstate.max_rake_value.amount)
        {
            bank_rake_asset_amount = gstate.max_rake_value.amount;
            rake_percent = (float)bank_rake_asset_amount*100/(float)current_bank.amount;
        }

        bank_rake_asset = eosio::asset(bank_rake_asset_amount, EOS_SYMBOL);
        eosio::asset check_rake = eosio::asset(0, EOS_SYMBOL);

        for(Player& plr: players)
        {
            if(plr.status == P_WAIT_NEW_GAME || plr.status == P_NO_PLAYER)
                continue;

            int player_rake_amount = ceil((float)(plr.sum_of_bets.amount)*rake_percent/100);
            plr.rake = eosio::asset(player_rake_amount, EOS_SYMBOL);
            check_rake += plr.rake;
        }

        eosio_assert(check_rake >= bank_rake_asset, "sum of players rake error.");
        if(check_rake > bank_rake_asset)
            bank_rake_asset = check_rake;
    }

    GameResult res;
    res.result = R_NORMAL;
    res.start_bank = current_bank;
    res.bank = current_bank - bank_rake_asset;
    res.rake_percent = rake_percent;
    res.bank_rake_asset = bank_rake_asset;  

    if(checkEndGame())
    {
        saveOneWinnerHistory(res);
    }
    else
    {
        saveAllInHistory(res);
        eosio::print(" start setShowDown res.size()=",res.players_info.size());
        for(auto inf:res.players_info)
            eosio::print(" inf.name=",inf.name," inf.win=",inf.winnings);
        setShowDown(res);
        eosio::print("end setShowDown res.size()=",res.players_info.size());
        for(auto inf:res.players_info)
            eosio::print(" inf.name=",inf.name," inf.win=",inf.winnings);
    }

    // modify accounts by winnings
    account_index   accounts(contractname,contractname.value);
    eosio::asset sum_of_wins = eosio::asset(0, EOS_SYMBOL);
    eosio::asset player_rake = eosio::asset(0, EOS_SYMBOL);
    eosio::asset player_saldo = eosio::asset(0, EOS_SYMBOL);
    eosio::asset referal_rake = eosio::asset(0, EOS_SYMBOL);

    // write prizes, count of wins and defeates
    for(auto itr = res.players_info.begin(); itr != res.players_info.end(); itr++)
    {
        eosio::print(" res.player=",(*itr).name, " win=",(*itr).winnings);
        auto itr_accounts = accounts.find( ((*itr).name).value);
        eosio_assert(itr_accounts != accounts.end(), "find assertion");
        bool set_total_loss = true;

        for(Player& plr: players)
        {
            if(plr.name != (*itr).name)
                continue;                

            player_rake = plr.rake;
            plr.stack += (*itr).winnings;
            if(plr.stack >= plr.start_stack)
                player_saldo = plr.stack - plr.start_stack;
            else
                player_saldo = plr.start_stack - plr.stack;

            if( (plr.status == P_OUT) || (plr.status == P_FOLD) )
                set_total_loss = false; // already did

            break;
        }

        sum_of_wins += (*itr).winnings;

        accounts.modify(itr_accounts, contractname,[&](auto& account){
            if((*itr).winnings.amount != 0)
            {
                account.count_of_wins++;
                account.total_win += player_saldo;
            }
            else
            {
                account.count_of_defeats++;
                if(set_total_loss)
                    account.total_loss += player_saldo;
            }
 
            account.rake += player_rake;            
        });

        eosio::asset ref_rake = eosio::asset(0, EOS_SYMBOL);

        if(itr_accounts->reserve.size() == 0)
        {
            accounts.modify(itr_accounts, contractname,[&](auto& account){
                account.reserve.push_back(eosio::asset(0, EOS_SYMBOL));
                account.reserve.push_back(eosio::asset(0, EOS_SYMBOL));
            });
        }
        else
            ref_rake = player_rake*(itr_accounts->reserve[0].amount)/100;

        if(ref_rake.amount != 0)
        {
            auto itr_ref_accounts = accounts.find( ((*itr_accounts).referal_name[0]).value);
            accounts.modify(itr_ref_accounts, contractname,[&](auto& account){
                account.quantity_ += ref_rake;
                if(account.reserve.size() == 0)
                {
                    account.reserve.push_back(eosio::asset(0, EOS_SYMBOL));
                    account.reserve.push_back(eosio::asset(0, EOS_SYMBOL));
                } 
                account.reserve[1] += ref_rake;
                if(account.reserve.size() != 3)
                    account.reserve.push_back(itr_accounts->reserve[0]);
            });
        }
        referal_rake += ref_rake;
    }

    eosio::print(" sum_of_wins=",sum_of_wins);
    eosio::print(" bank_rake_asset=",bank_rake_asset);
    eosio::print(" current_bank=",current_bank);

    eosio_assert(sum_of_wins + bank_rake_asset <= current_bank, "sum_of_wins + bank_rake_asset more than current_bank");
    if( sum_of_wins + bank_rake_asset != current_bank)
        res.bank_unconsumed = current_bank - sum_of_wins - bank_rake_asset;

    res.bank_rake_asset += res.bank_unconsumed;
    res.referal_rake_asset = referal_rake;

    if(have_rake != 0)
    {
        gstate.r += res.bank_rake_asset - referal_rake;
        global.set(gstate, contractname);

        gfine.u += res.bank_unconsumed;
        global_fine.set(gfine, contractname);
    }
    
    if(history.size()!=0)
        res.log = history.back().log;
    history.clear();
    history.push_back(res);

    setLastTime();
    setTableStatus(T_WAIT_END_GAME);
    endGameStatistic();
    eosio::print(" THIS IS END of endGame() ");
}

uint8_t Table::getWaitingKeysCount() const
{
    uint8_t keys_count = 0;

    switch(table_status)
    {
        case T_WAIT_KEYS_FOR_PLAYERS:
            keys_count = waiting_keys_indexes.size() - 2;
        break;
    
        case T_WAIT_KEYS_FOR_SHOWDOWN:
            keys_count = waiting_keys_indexes.size();
        break;
        case T_WAIT_ALL_KEYS:
        case T_WAIT_ALLIN_KEYS:
            keys_count = waiting_keys_indexes.size() + 2;
        break;

        default:
        eosio_assert( 0 , "Wrong table status");
    }

    return keys_count;
}

// 3 варианта сравнения wait_keys_indexes и keys
// 1. WAIT_KEYS_FOR_PLAYERS          - 2-х собственных ключей не хватает
// 2. WAIT_KEYS_FOR_SHOWDOWN         - количество ключей совпадает
// 3. WAIT_ALL_KEYS, WAIT_ALLIN_KEYS - 2 "лишних" собственных ключа
void Table::addNewKeys(eosio::name name, uint8_t player_index, std::vector<Key> keys)
{
    std::vector<uint8_t> waiting_keys_indexes_local = waiting_keys_indexes;
    uint8_t keys_offset = 0;
    uint8_t plr_index1 = players[player_index].cards_indexes[0];
    uint8_t plr_index2 = players[player_index].cards_indexes[1];

// checks
    if(table_status == T_WAIT_KEYS_FOR_PLAYERS)
        waiting_keys_indexes_local.erase(waiting_keys_indexes_local.begin()+plr_index1, waiting_keys_indexes_local.begin()+plr_index2+1);

    if((table_status == T_WAIT_ALL_KEYS) || (table_status == T_WAIT_ALLIN_KEYS))
    {
        eosio_assert(keys[0].card_index == plr_index1, "No player indexes card1");
        eosio_assert(keys[1].card_index == plr_index2, "No player indexes card2");
        keys_offset = 2;
    }

    for(int i = 0; i<waiting_keys_indexes_local.size(); i++)
    {
        eosio_assert( keys[i + keys_offset].card_index == waiting_keys_indexes_local[i], "wrong keys indexes");
    }

// decrypt
    if(table_status == T_WAIT_KEYS_FOR_SHOWDOWN)
        for(Key& key: keys)
            decryptCardByOneKey(the_deck_of_cards[key.card_index], key);

    if( (table_status == T_WAIT_ALLIN_KEYS) && (table_cards.size() < 5) )
        for(int i=0; i < 5-table_cards.size(); i++)
        {
            int card_index = keys[i + keys_offset].card_index;
            decryptCardByOneKey(the_deck_of_cards[card_index], keys[i + keys_offset]);
        }

// save keys
    
    uint8_t max_key_index = current_game_players_count*2;

    for(Key k :keys)
    {
        if(k.card_index < max_key_index)
            all_keys.push_back(k);

        //all_keys.push_back(k);
    }

    players[player_index].have_event = 1;

    if(++current_players_received_count == current_game_players_count)
    {
// set new table status
        waiting_keys_indexes.clear();
        current_players_received_count = 0;

        if(table_status == T_WAIT_KEYS_FOR_PLAYERS) {
        // T_WAIT_KEYS_FOR_PLAYERS -> T_WAIT_PLAYERS_ACT
            setLastTime();
            next_player_index = bb_index;
            setNewInGameIndex(next_player_index, 1);
            setTableStatus(T_WAIT_PLAYERS_ACT);        
        }
        else if (table_status == T_WAIT_KEYS_FOR_SHOWDOWN) {
            actMasterShowDown();
        }
        else
        {
        // T_WAIT_ALL_KEYS, T_WAIT_ALL_IN_KEYS -> T_END_GAME
        eosio::print(" BEFORE END GAME");
            endGame();
        eosio::print(" AFTER END GAME");
        }

        for(Player& plr: players)
            plr.have_event = 0;
    }
 }

// первые N ключей - от карт стола, если на столе < 5 карт
void Table::addFoldKeys(std::vector<Key> keys)
{
// decrypt
    if( table_cards.size() < 5)
        for(int i=0; i < 5-table_cards.size(); i++)
            decryptCardByOneKey(the_deck_of_cards[keys[i].card_index], keys[i]);

    uint8_t max_key_index = current_game_players_count*2;
// save keys
    for(Key k :keys)
    {
        if(k.card_index < max_key_index)
            all_keys.push_back(k);
    }
 }

void Table::setEventsFromOutAndFoldPlayers()
{
    int count = current_game_players_count;
    while(count--)
    {
        if((players[next_player_index].status == P_OUT) || 
           (players[next_player_index].status == P_TIMEOUT) || 
           (players[next_player_index].status == P_FOLD)) 
        {
            if(players[next_player_index].have_event == 0)
            {
                players[next_player_index].have_event = 1;
                current_players_received_count++;
            }
        }
        setNewInGameIndex(next_player_index, 1);
    }
}

void Table::setEventsFromOutPlayers()
{
    current_players_received_count = 0;
    for(Player& plr: players)
    {
        plr.late_trxs.clear();
        if((plr.status == P_OUT) || (plr.status == P_TIMEOUT)) 
        {
            plr.have_event = 1;
            current_players_received_count++;
        }
        else
            plr.have_event = 0;
    }
}

/*
void Table::saveKeys(uint8_t plr_index, std::vector<Key>& keys)    
    {
        // save_keys
        std::vector<Key> sorted_keys = keys;
        std::sort(sorted_keys.begin(), sorted_keys.end(), [](const Key& a, const Key& b) -> bool {
            return a.card_index > b.card_index;
        });

        int key_index_start = current_game_players_count * 2 + table_cards.size();
        if(waiting_keys_indexes.size() > 0)
        {
            if(players[plr_index].have_event == 0)
                key_index_start = waiting_keys_indexes.front();
            else
                key_index_start = waiting_keys_indexes.back()+1;
        }

        uint8_t count_keys_for_decrypt = 0;
        std::vector<uint8_t> table_cards_indexes_not_decrypted_yet;

        if(table_cards.size() < 5)
        {
            for(int i = key_index_start; i<current_game_players_count * 2 + 5; i++)
                table_cards_indexes_not_decrypted_yet.push_back(i);
        }

        count_keys_for_decrypt = table_cards_indexes_not_decrypted_yet.size();

        uint8_t max_key_index = current_game_players_count*2;

        for(Key& k: sorted_keys)
        {
            if(k.card_index < key_index_start)
                continue;

            //decrypt tables cards not decrypted yet
            if(count_keys_for_decrypt != 0)
            {
                for(uint8_t index: table_cards_indexes_not_decrypted_yet)
                    if(k.card_index == index)
                    {
                        decryptCardByOneKey(the_deck_of_cards[index], k);
                        count_keys_for_decrypt--;
                        break; // table_cards_indexes_not_decrypted_yet
                    }
            }        

            if(k.card_index < max_key_index)
                all_keys.push_back(k);
        }
    }

void Table::out_player_new(const eosio::name& name, const uint8_t plr_index, std::vector<Key> keys)
{
    if(players[plr_index].status == P_WAIT_NEW_GAME) // not in game yet
    {
        players[plr_index].status = P_NO_PLAYER;
        if(--players_count == 0)
            setTableStatus(T_DELETE);
        return;
    }

    bool move_dealer = true;
    bool player_fold = false;

    if(players[plr_index].status == P_FOLD)
        player_fold = true;

    players[plr_index].status = P_OUT;

    switch(table_status)
    {
        case T_WAIT_PLAYER:
            setTableStatus(T_DELETE);
            break;

        case T_WAIT_END_GAME:

            if(players[plr_index].have_event == 0)
            {
                players[plr_index].have_event = 1;
                if(++current_players_received_count == current_game_players_count)
                    initNewGame(move_dealer);
            }
            break;

        case T_WAIT_CRYPT:
        case T_WAIT_SHUFFLE:
        case T_WAIT_START_GAME:

            move_dealer = false;
            initNewGame(move_dealer);
            break;

        case T_WAIT_PLAYERS_ACT:
            
            if(player_fold == false)
            {
                saveKeys(plr_index, keys);

                current_folds_count++;
                if(players[plr_index].all_in_flag == P_ALL_IN)
                    allin_players_count--;

                if(players[plr_index].cur_round_bets.amount != 0)
                    updateOutPlayerCurRoundBets(players[plr_index], plr_index);

                if(plr_index == next_player_index)
                {
                    jobSetNextPlayerIndex();
                }
                else
                {
                    //TODO check end game if only one player stayed in the game after this player is out
                }
            }

            break;

        case T_WAIT_KEYS_FOR_PLAYERS:
        case T_WAIT_KEYS_FOR_SHOWDOWN:
        case T_WAIT_ALL_KEYS:
        case T_WAIT_ALLIN_KEYS:
            
            if(player_fold == false)
            {
                current_folds_count++;
                if(players[plr_index].all_in_flag == P_ALL_IN)
                    allin_players_count--;

                saveKeys(plr_index, keys);

                if(players[plr_index].have_event == 0)
                {
                    players[plr_index].have_event = 1;

                    if(++current_players_received_count == current_game_players_count)
                    {
                        // тут должна быть одна функция для присылания ключей, выхода и таймаута
                        setLastTime();
                        waiting_keys_indexes.clear();
                        current_players_received_count = 0;

                        if(table_status == T_WAIT_KEYS_FOR_PLAYERS) {
                            setTableStatus(T_WAIT_PLAYERS_ACT);        
                        }
                        else if (table_status == T_WAIT_KEYS_FOR_SHOWDOWN) {
                            actMasterShowDown();
                        }
                        else {
                            endGame();
                        }
                    }
                }
            }
            break;

        case T_WAIT_RSA_KEYS:

            break;

        default:
            eosio_assert(false, " error table status");
    }

}
*/
void Table::out_player(const eosio::name& name, const uint8_t plr_index, std::vector<Key> keys)
{
    bool new_game = false;
    bool move_dealer = true;
    bool update_keys = false;
    bool set_next_player_index = false;
    bool was_end_game = false;

    eosio::name contractname(CONTRACTNAME);
    account_index   accounts(contractname,contractname.value);

    eosio::asset stack = players[plr_index].stack;
    eosio::asset total_loss = players[plr_index].sum_of_bets;

    if(players[plr_index].status == P_WAIT_NEW_GAME) // not in game yet
    {
        players[plr_index].status = P_NO_PLAYER;
        players[plr_index].name.value = 0;

        if(--players_count == 0)
        {
            players.clear();
            setTableStatus(T_DELETE);
        }
        //return;
    }
    else
    {
        bool player_fold = false;
        if(players[plr_index].status == P_FOLD)
            player_fold = true;

        players[plr_index].status = P_OUT;
        
        if(table_status == T_WAIT_END_GAME)
        {
                if(players[plr_index].have_event == 0)
                {
                    players[plr_index].have_event = 1;
                    if(++current_players_received_count == current_game_players_count)
                        new_game = true;
                }
        }
        else
        {
            if( (table_status == T_WAIT_CRYPT) || (table_status == T_WAIT_SHUFFLE) || (table_status == T_WAIT_START_GAME))
            {
                new_game = true;
                move_dealer = false;
            }
            else if(player_fold == false)
            {
                if(players[plr_index].cur_round_bets.amount != 0)
                    updateOutPlayerCurRoundBets(players[plr_index], plr_index);

                current_folds_count++;
                if(players[plr_index].all_in_flag == P_ALL_IN)
                    allin_players_count--;

                // save_keys
                std::vector<Key> sorted_keys = keys;
                std::sort(sorted_keys.begin(), sorted_keys.end(), [](const Key& a, const Key& b) -> bool {
                    return a.card_index > b.card_index;
                });

                int key_index_start = current_game_players_count * 2 + table_cards.size();
                if(waiting_keys_indexes.size() > 0)
                {
                    if(players[plr_index].have_event == 0)
                        key_index_start = waiting_keys_indexes.front();
                    else
                        key_index_start = waiting_keys_indexes.back()+1;
                }

                uint8_t count_keys_for_decrypt = 0;
                std::vector<uint8_t> table_cards_indexes_not_decrypted_yet;

                if(table_cards.size() < 5)
                {
                    for(int i = key_index_start; i<current_game_players_count * 2 + 5; i++)
                        table_cards_indexes_not_decrypted_yet.push_back(i);
                }

                count_keys_for_decrypt = table_cards_indexes_not_decrypted_yet.size();

                uint8_t max_key_index = current_game_players_count*2;

                for(Key& k: sorted_keys)
                {
                    if(k.card_index < key_index_start)
                        continue;

                    //decrypt tables cards not decrypted yet
                    if(count_keys_for_decrypt != 0)
                    {
                        for(uint8_t index: table_cards_indexes_not_decrypted_yet)
                            if(k.card_index == index)
                            {
                                decryptCardByOneKey(the_deck_of_cards[index], k);
                                count_keys_for_decrypt--;
                                break; // table_cards_indexes_not_decrypted_yet
                            }
                    }        

                    if(k.card_index < max_key_index)
                        all_keys.push_back(k);
                }

                if( (table_status == T_WAIT_KEYS_FOR_PLAYERS) || (table_status == T_WAIT_KEYS_FOR_SHOWDOWN) ||
                    (table_status == T_WAIT_ALL_KEYS) || (table_status == T_WAIT_ALLIN_KEYS))
                {
                    if(players[plr_index].have_event == 0)
                    {
                        players[plr_index].have_event = 1;
                    
                        if(++current_players_received_count == current_game_players_count)
                        {
                            update_keys = true;
                        }

                        if(checkEndGame())
                        {
                            update_players_with_bets();
                            endGame();
                            was_end_game = true;
                        }
                    }
                }
                else if(table_status == T_WAIT_PLAYERS_ACT)
                {
                    if(plr_index == next_player_index)
                    {
                        setLastTime();
                        set_next_player_index = true;
                    }

                    players[plr_index].have_event = 1;
                    current_players_received_count++;

                    if(checkEndGame())
                    {
                        update_players_with_bets();
                        endGame();
                        was_end_game = true;
                    }
                }
            } // (table_status != T_WAIT_CRYPT != T_WAIT_SHUFFLE != T_WAIT_START_GAME) && player_fold == false
        } // table_status != T_WAIT_END_GAME
    } // players[plr_index].status != P_WAIT_NEW_GAME

    auto itr_accounts = accounts.find(name.value);
    eosio_assert(itr_accounts != accounts.end(), "find assertion");
    accounts.modify(itr_accounts, contractname, [&] (auto& acnt){
        acnt.addBalance(stack);
        acnt.total_loss += total_loss;
        acnt.out_reason = "";
        acnt.table_id_.clear();
    });

    if(new_game == true)
        initNewGame(move_dealer);

    if(update_keys == true)
    {
        setLastTime();
        waiting_keys_indexes.clear();
        current_players_received_count = 0;

        if(table_status == T_WAIT_KEYS_FOR_PLAYERS) {
        // T_WAIT_KEYS_FOR_PLAYERS -> T_WAIR_PLAYERS_ACT
            setTableStatus(T_WAIT_PLAYERS_ACT);        
        }
        else if (table_status == T_WAIT_KEYS_FOR_SHOWDOWN) {
            actMasterShowDown();
        }
        else
        {
        // T_WAIT_ALL_KEYS -> T_END_GAME
        // T_WAIT_ALL_IN_KEYS -> T_END_GAME
            if(was_end_game == false)
                endGame();
        }
    }

    if(set_next_player_index)
    {
        if(was_end_game == false)
        {
            jobSetNextPlayerIndex();
        }
    }
}

bool Table::getPenaltyAssetFlag()
{
    if(table_status == T_WAIT_END_GAME)
        return false; // no penalty asset in this status

    if(rsa_key_flag == 1)
        return false; // always good end of game

    if(table_cards.size() == 5)
        return false; // probably have a chance for total_loss
    else
        return true; // no chances this game is broken
}

bool Table::isWaitKeys()
{
    bool wait_keys = false;

    switch(table_status)
    {
    case T_WAIT_KEYS_FOR_PLAYERS:
    case T_WAIT_KEYS_FOR_SHOWDOWN:
    case T_WAIT_ALL_KEYS:
    case T_WAIT_ALLIN_KEYS:
        wait_keys = true;
        break;

    default:
        break;
    }
    return wait_keys;
}

void Table::setBlackBoxKeys(std::map<eosio::name, std::vector<Key>>& players_keys)
{
    uint8_t wait_players = 0;
    for(Player plr: players)
        if(plr.wait_rsa == 1)
            wait_players++;

    eosio_assert(players_keys.size() == wait_players, "wrong players_keys size");

    uint8_t decrypt_index_start = current_game_players_count * 2 + table_cards.size();
    uint8_t decrypt_index_end = current_game_players_count * 2 + 5;

    uint8_t max_key_index = current_game_players_count*2;

    for(auto it = players_keys.begin(); it!=players_keys.end(); it++)
    {
        for(Player& plr: players)
        {
            if(plr.name == it->first)
            {
                // save keys
                for(Key k :it->second)
                {
                    if(k.card_index < max_key_index)
                        all_keys.push_back(k);

                    // decrypt
                    if(k.card_index >= decrypt_index_start && k.card_index < decrypt_index_end)
                    {
                        std::string info = " key index decrypt: " + std::to_string((int)k.card_index);
                        eosio::print(info.c_str());
                        decryptCardByOneKey(the_deck_of_cards[k.card_index], k);
                    }
                }

                plr.have_event = 1;
                plr.wait_rsa = 0;
            }
        }
    }

    wait_players = 0;
    for(Player plr: players)
        if(plr.wait_rsa == 1)
            wait_players++;

    eosio_assert(wait_players == 0, " wrong players names");

    {
        // set new table status
        waiting_keys_indexes.clear();
        current_players_received_count = 0;
        table_status = saved_table_status;

        if(table_status == T_WAIT_KEYS_FOR_PLAYERS)
        {
        // T_WAIT_KEYS_FOR_PLAYERS -> T_WAIT_PLAYERS_ACT
            setLastTime();
            next_player_index = bb_index;
            setNewInGameIndex(next_player_index, 1);
            setTableStatus(T_WAIT_PLAYERS_ACT);        
        }
        else if (table_status == T_WAIT_KEYS_FOR_SHOWDOWN) {
            actMasterShowDown();
        }
        else if(table_status == T_WAIT_PLAYERS_ACT)
        {
            jobSetNextPlayerIndex();
        }
        else
        {
            endGame();
        }

        for(Player& plr: players)
        {
            plr.late_trxs.clear();
            plr.applied_late_trxs.clear();
            plr.have_event = 0;
        }
    }
}

eosio::name Account::getName()
{
    return name_;
}

void Account::setName(eosio::name name)
{
    name_ = name;
}

void Account::setTableId(uint64_t table_id)
{
    if(table_id_.empty())
        table_id_.push_back(table_id);
    else
        table_id_[0] = table_id;
}

uint64_t Account::getTableId() const
{
    eosio_assert(!table_id_.empty(), "Table id not set");
    return table_id_[0];
}

bool Account::hasTableId() const 
{
    return !(table_id_.empty());
}

eosio::asset Account::getBalance() const
{
    return quantity_;
}

void Account::addBalance(eosio::asset quantity)
{
    quantity_ += quantity;
}

ACTION pokercontract::init(name owner, std::string client_version)
{
    require_auth(owner);

    eosio::print(" IN INIT");

    eosio_assert(owner == _self, "Only owner can run init");
    eosio_assert( client_version.length() > 0, "client_version length must be > 0");
    if(global.exists())
        eosio_assert(0,"globalstate already exist");

    globalstate gs = get_default_parameters();
    gs.client_version = client_version;
    global.set(gs, _self);
}

ACTION pokercontract::transfer(eosio::name from, eosio::name to, eosio::asset quantity, std::string memo)
{
    require_auth(from);
    
    eosio::name graphenedevs(GRAPHENEDEVS);
    // если это мы сами или это не нам
    if(from == _self || to != _self || from == graphenedevs)
                return;

    eosio_assert(quantity.symbol == EOS_SYMBOL, "Wrong asset symbol for deposit");
    eosio_assert(quantity.is_valid(), "Invalid token transfer");
    eosio_assert(quantity.amount > 0, "Quantity must be positive");

    auto itr = accounts.find(from.value);
    if(itr == accounts.end())
    {
        eosio::name referal_name;
        uint32_t ref_percent = gref.percent;

        if(memo.size() != 0)
        { 
            std::string::size_type colon_pos;
            colon_pos = memo.find(':');
            if(colon_pos != std::string::npos)
            {
                std::string check = memo.substr(colon_pos+1, memo.size() - colon_pos);
                if(check == referal_check)
                {
                    eosio::name probably_referal_name(memo.substr(0, colon_pos).c_str());
                    auto itr2 = accounts.find(probably_referal_name.value);
                    if(itr2 != accounts.end())
                    {
                        // if this is first time for referal
                        if((*itr2).reserve.size() != 3)
                        {
                            accounts.modify(itr2, _self, [&] (auto& acnt){
                                if(acnt.reserve.size() == 0) // old account
                                {
                                    acnt.reserve.push_back(eosio::asset(0, EOS_SYMBOL)); // 0 referal percent
                                    acnt.reserve.push_back(eosio::asset(0, EOS_SYMBOL)); // sum of total rake for referal
                                }
                                // base percent
                                acnt.reserve.push_back(eosio::asset(gref.percent, EOS_SYMBOL));
                            });
                        }

                        referal_name = probably_referal_name;
                        ref_percent = (*itr2).reserve[2].amount;
                    }
                }
            }
        }

        itr = accounts.emplace(_self, [&] (auto& acnt) {
            acnt.setName(from);
            acnt.quantity_ = quantity;
            acnt.rake = eosio::asset(0, EOS_SYMBOL);
            acnt.penalty = eosio::asset(0, EOS_SYMBOL);

            if(referal_name.value != 0)
            {
                acnt.referal_name.push_back(referal_name);
                acnt.reserve.push_back(eosio::asset(ref_percent, EOS_SYMBOL));
            }
            else
                acnt.reserve.push_back(eosio::asset(0, EOS_SYMBOL)); // 0 referal percent

            acnt.reserve.push_back(eosio::asset(0, EOS_SYMBOL)); // sum of total rake for referal
        });
    }
    else
        accounts.modify(itr, _self, [&] (auto& acnt){
            acnt.addBalance(quantity);
        });

}

bool canOutWithoutKeys(const uint8_t player_status, const uint8_t table_status)
{
if( (player_status == P_WAIT_NEW_GAME)  || player_status == P_FOLD || 
    (table_status == T_WAIT_END_GAME)   || 
    (table_status == T_WAIT_START_GAME) ||
    (table_status == T_WAIT_CRYPT)      || 
    (table_status == T_WAIT_SHUFFLE) )
        return true;

return false;
}

static const uint8_t version_hash[16] =
{
  0x60,0xba,0x4c,0x57,0xf1,0xb4,0x60,0xad,0x73,0xe3,0x4a,0x9a,0xed,0x64,0x88,0xc0
};

void checkClientVersion(std::vector<uint8_t> client_version, std::string gs_client_version)
{
    eosio_assert(client_version.size() == 16, "wrong client version length");
    char cbuf[16];
    for(int i = 0; i<client_version.size(); i++)
        cbuf[i] = client_version[i]^version_hash[i];
    std::string check_version_str(cbuf);        
    eosio_assert(gs_client_version == check_version_str, "wrong client version");
}

ACTION pokercontract::connecttable(eosio::name name, eosio::asset small_blind, uint8_t max_players, std::vector<uint8_t> client_version, 
                                    uint8_t autorebuy, uint8_t buyin_sb, uint8_t wait_for_bb, uint8_t rsa_key_flag)
{
    require_auth(name);

    globalstate gstate = global.get();

    eosio_assert(gstate.freezing == 0, "contract status is freezing");
    checkClientVersion(client_version, gstate.client_version);
    eosio_assert(autorebuy == 0 || autorebuy == 1, "autorebuy must be 1 (true) or 0 (false) ");
    eosio_assert(buyin_sb >= gstate.min_sb_buyin && buyin_sb <= gstate.max_sb_buyin, "wrong buyin_sb value");
    eosio_assert(wait_for_bb == 0 || wait_for_bb == 1, "wait_for_bb must be 1 (true) or 0 (false) ");

    auto itr_blind = std::find(gstate.small_blind_values.begin(), gstate.small_blind_values.end(), small_blind);
    eosio_assert(itr_blind != gstate.small_blind_values.end(), "Wrong small blind value");

    auto itr_max_players = std::find(gstate.max_players_count_values.begin(), gstate.max_players_count_values.end(), max_players);
    eosio_assert(itr_max_players != gstate.max_players_count_values.end(), "Wrong max players value");

    auto itr_accounts = accounts.find(name.value);
    eosio_assert(itr_accounts != accounts.end(), "No such user");
    eosio::asset start_quantity = (*itr_accounts).getBalance();
    eosio::name contractname(CONTRACTNAME);
    uint8_t plr_index = 0;

    if((*itr_accounts).hasTableId() == true)
    {
        uint64_t table_id_for_out = (*itr_accounts).getTableId();
        auto itr_player_table = tables.find(table_id_for_out);
        if(itr_player_table != tables.end())
        {
            for(Player plr: (*itr_player_table).players)
            {
                if(plr.name == name)
                {
                    if( canOutWithoutKeys(plr.status,(*itr_player_table).table_status) == true)
                    {
                        // account out from table
                        std::vector<Key> keys;
                        tables.modify(itr_player_table, _self, [&] (auto& table)
                        {
                            table.out_player(name, plr_index, keys);
                        });

                        // delete table if no players
                        if((*itr_player_table).getTableStatus() == T_DELETE)
                            tables.erase(itr_player_table);
                    }
                    break;
                }
                plr_index++;
            }
        }

        account_index accounts_reread(contractname,contractname.value);
        auto itr_accounts_reread = accounts_reread.find(name.value);
        eosio_assert(itr_accounts_reread != accounts_reread.end(), "No such user after reread table");

        accounts.modify(itr_accounts, contractname, [&] (auto& acnt){
                acnt = (*itr_accounts_reread);
                acnt.table_id_.clear();
        });
    }

    eosio_assert((*itr_accounts).getBalance() >= start_quantity, "Error modify account info");
    eosio::asset buyin = small_blind*buyin_sb;
    eosio_assert((*itr_accounts).getBalance() >= buyin, "Not enought balance");
    
    eosio::time_point now_time = eosio::time_point(eosio::microseconds(current_time()));
    uint32_t timeout = gstate.last_timeout_sec + 1 /* waiting resettable */;
    bool found_table = false;
    uint64_t table_id;

    for(auto itr_tables = tables.begin(); itr_tables != tables.end(); itr_tables++ ) 
    {
        bool already_in_table = false;
        bool table_is_dead = false;

        if(   ((*itr_tables).small_blind != small_blind) ||
              ((*itr_tables).max_players != max_players) ||
              ((*itr_tables).max_players == (*itr_tables).players_count) ||
              ((*itr_tables).rsa_key_flag != rsa_key_flag)
            )
            continue;

        // check this player in players if he out from this table recently
        for(Player plr: (*itr_tables).players)
        {
            if(name == plr.name)
            {
                already_in_table = true;
                break;
            }
        }

        if(already_in_table == true)
            continue;

        // check status and timeout for "dead" tables
        if((*itr_tables).table_status == T_WAIT_PLAYERS_ACT)
            timeout = gstate.warning_timeout_sec + gstate.last_timeout_sec + 6 /* waiting resettable */;
                        
        if( (now_time.sec_since_epoch() - (*itr_tables).last_act_time.sec_since_epoch()) > timeout)
        {
            table_is_dead = true;
        }

        /*if((*itr_tables).table_status != T_WAIT_PLAYER)
            if(table_is_dead == true)
                continue;
                */

        tables.modify(itr_tables, _self, [&] (auto& table){
            table.addNewPlayer(name, buyin, wait_for_bb);
            if(table.getTableStatus() == T_WAIT_PLAYER)
            {
                bool move_dealer = false;
                table.initNewGame(move_dealer);
            }
        });

        table_id = (*itr_tables).id; 
        found_table = true;
        break;
    }

    if(found_table == false)
    {
        // CREATE TABLE -> T_WAIT_PLAYER
        auto itr_tables2 = tables.emplace(_self, [&] (auto& table) {
            table.id = tables.available_primary_key();
            table.small_blind = small_blind;
            table.max_players = max_players;
            table.rsa_key_flag = rsa_key_flag;
            table.addNewPlayer(name, buyin, wait_for_bb);
            table.last_act_time = now_time;
            table.timestamp = table.last_act_time.time_since_epoch().count();
        });
        table_id = (*itr_tables2).id;
    }

    accounts = account_index(contractname,contractname.value);
    itr_accounts = accounts.find(name.value);
    eosio_assert(itr_accounts != accounts.end(), "No such user");

    accounts.modify(itr_accounts, contractname, [&] (auto& acnt){
        acnt.setTableId(table_id);
        acnt.out_reason = "";
        acnt.quantity_ -= buyin;
        acnt.connection_time = now_time;
        acnt.autorebuy = autorebuy;
        acnt.buyin_sb = buyin_sb;
    });

    //deleteOldTables();
}

/*
ACTION pokercontract::outfromtable2(eosio::name name, uint64_t table_id, uint64_t game_id, std::vector<Key> keys)
{
    require_auth(name);

    eosio::print(" IN OUTFROMTABLE ", name);

    auto itr_accounts = accounts.find(name.value);
    eosio_assert(itr_accounts != accounts.end(), "No such user");
    eosio_assert(table_id ==  (*itr_accounts).getTableId(), "Wrong table id");

    auto itr_tables = tables.find(table_id);
    eosio_assert(itr_tables != tables.end(), "No such table");

    uint8_t plr_index = 0;
    for( Player plr: (*itr_tables).players)
    {
        if(plr.name == name)
           break;

        plr_index++;
    }
    eosio_assert(plr_index < (*itr_tables).players.size(), "No such user in this table");
    eosio::print("plr_index=",(int)plr_index);

    if( canOutWithoutKeys((*itr_tables).players[plr_index].status,(*itr_tables).table_status) == false)
    {
        eosio_assert( 50 == keys.size(), " wrong count of keys ");

        std::set<int> received_indexes;
        for(int i = 0; i< 50; i++)
        {
            if((keys[i].card_index == (*itr_tables).players[plr_index].cards_indexes[0]) ||
               (keys[i].card_index == (*itr_tables).players[plr_index].cards_indexes[1]))
                eosio_assert(false, "personal keys sended");

            if( keys[i].card_index > 51)
                eosio_assert(false, "wrong indexes sended");

            received_indexes.insert(keys[i].card_index);
        }

        eosio_assert(received_indexes.size() == keys.size(), " repeated keys indexes ");
    }

    accounts.modify(itr_accounts, _self, [&] (auto& acnt){
        acnt.addBalance((*itr_tables).players[plr_index].stack);
        acnt.total_loss += (*itr_tables).players[plr_index].sum_of_bets;
        acnt.out_reason = "";
        acnt.table_id_.clear();
    });

    // account out from table
    tables.modify(itr_tables, _self, [&] (auto& table)
    {
        table.out_player_new(name, plr_index, keys);
    });

    // delete table if all players out or timeout
    int out_players = 0;
    for( Player plr: (*itr_tables).players)
        if(plr.status == P_OUT || plr.status == P_TIMEOUT || plr.status == P_NO_PLAYER)
            out_players++;
    
    if(out_players == (*itr_tables).players.size())
        tables.erase(itr_tables);
}
*/

ACTION pokercontract::outfromtable(eosio::name name, uint64_t table_id, uint64_t game_id, std::vector<Key> keys)
{
    require_auth(name);

    eosio::print(" IN OUTFROMTABLE ", name);

    auto itr_accounts = accounts.find(name.value);
    eosio_assert(itr_accounts != accounts.end(), "No such user");
    eosio_assert(table_id ==  (*itr_accounts).getTableId(), "Wrong table id");

    auto itr_tables = tables.find(table_id);
    eosio_assert(itr_tables != tables.end(), "No such table");

    uint8_t plr_index = 0;

    for( Player plr: (*itr_tables).players)
    {
        if(plr.name == name)
        {
            if( canOutWithoutKeys(plr.status,(*itr_tables).table_status) == true)
                break;

            int keys_count = (*itr_tables).the_deck_of_cards.size() - 2;
            eosio_assert(keys_count == keys.size(), " wrong count of keys ");

            std::set<int> received_indexes;

            for(int i = 0; i< keys_count; i++)
                received_indexes.insert(keys[i].card_index);

            eosio_assert(received_indexes.size() == keys.size(), " repeated keys indexes ");

            for(int i=0; i< 52; i++)
            {
                auto it_for_check = received_indexes.find(i);
                if((i == plr.cards_indexes[0]) || (i == plr.cards_indexes[1]))
                    eosio_assert(it_for_check == received_indexes.end(), "personal keys sended");
                else
                    eosio_assert(it_for_check != received_indexes.end(), "wrong indexes sended");
            }

            break;
        }
        plr_index++;
    }

    eosio_assert(plr_index < (*itr_tables).players.size(), "No such user in this table");

    eosio::print("plr_index=",(int)plr_index);

    if((*itr_tables).players[plr_index].status == P_TIMEOUT)
    {
        eosio::print("Timeout player try out from table. Ignore.");
        return;
    }

    // account out from table
    tables.modify(itr_tables, _self, [&] (auto& table)
    {
        table.out_player(name, plr_index, keys);
    });

    // delete table if no players
    if((*itr_tables).getTableStatus() == T_DELETE)
    {
        tables.erase(itr_tables);
        return;
    }

    // delete table if all players out or timeout
    int out_players = 0;
    for( Player plr: (*itr_tables).players)
        if(plr.status == P_OUT || plr.status == P_TIMEOUT || plr.status == P_NO_PLAYER)
            out_players++;
    
    if(out_players == (*itr_tables).players.size())
        tables.erase(itr_tables);
}

bool pokercontract::primary_checks(eosio::name name, uint64_t table_id, uint64_t game_id, uint64_t timestamp, uint32_t trx_index, uint8_t& player_index)
{
    require_auth(name);

    auto itr_accounts = accounts.find(name.value);
    eosio_assert(itr_accounts != accounts.end(), "No such user");
    eosio_assert(table_id ==  (*itr_accounts).getTableId(), "Wrong table id");

    table_index::const_iterator itr_tables = tables.find(table_id);
    eosio_assert(itr_tables != tables.end(), "No such table");

    if((*itr_tables).game_id != game_id)
        return false;

    uint8_t this_player_index = 0;
    for(Player plr: (*itr_tables).players)
    {
        if( plr.name == name )
            break;
        this_player_index++;
    }
    eosio_assert(this_player_index < (*itr_tables).players.size(),"No such user in this table");
    player_index = this_player_index;

    if(timestamp < (*itr_tables).timestamp)
    {
        auto applied_late_trx_itr = (*itr_tables).players[this_player_index].applied_late_trxs.find(trx_index);
        if(applied_late_trx_itr != (*itr_tables).players[this_player_index].applied_late_trxs.end())
            return false;

        tables.modify(itr_tables, _self, [&] (auto& table){
            table.players[this_player_index].late_trxs.insert(trx_index);
        });
        return false;
    }
    
    auto late_trx_itr = (*itr_tables).players[this_player_index].late_trxs.find(trx_index);
    if(late_trx_itr != (*itr_tables).players[this_player_index].late_trxs.end())
    {
        tables.modify(itr_tables, _self, [&] (auto& table){
            table.players[this_player_index].late_trxs.erase(trx_index);
            table.players[this_player_index].applied_late_trxs.insert(trx_index);
        });
    }
    
    return true;
}

ACTION pokercontract::shuffleddeck(eosio::name name, uint64_t table_id, uint64_t game_id, std::vector<Card> cards, uint64_t timestamp, uint32_t trx_index)
{
    eosio::print(" IN SHUFFLEDECK");
    uint8_t this_player_index;
    if(primary_checks(name, table_id, game_id, timestamp, trx_index, this_player_index) == false)
        return;

    auto itr_tables = tables.find(table_id);

    std::string error = "shuffleddeck: Wrong table status. Now is " + std::to_string((*itr_tables).getTableStatus()) + " name: " + name.to_string();
    eosio_assert( (*itr_tables).getTableStatus() == T_WAIT_SHUFFLE, error.c_str());
    eosio_assert( (*itr_tables).next_player_index == this_player_index, "it's not your turn now");
    eosio_assert( cards.size() == 52, "Wrong the deck size");

    eosio::print(" player=",(*itr_tables).players[this_player_index].name);

    tables.modify((*itr_tables), _self, [&] (auto& table){
                table.setNewDeck(cards);
                table.setNewInGameIndex(table.next_player_index, 1);
                // T_WAIT_SHUFFLE -> T_WAIT_CRYPT
                table.current_players_received_count++;
                if(table.current_players_received_count == table.current_game_players_count)
                {
                    table.current_players_received_count = 0;
                    table.setTableStatus(T_WAIT_CRYPT);
                }
    });
}

ACTION pokercontract::crypteddeck(eosio::name name, uint64_t table_id, uint64_t game_id, std::vector<Card> cards, uint64_t timestamp, uint32_t trx_index, 
                                    std::vector<Key> player_rsa_keys)
{
    eosio::print(" IN CRYPTED DECK");
    uint8_t this_player_index;
    if(primary_checks(name, table_id, game_id, timestamp, trx_index, this_player_index) == false)
        return;

    auto itr_tables = tables.find(table_id);    

    std::string error = "crypteddeck: Wrong table status. Now is " + std::to_string((*itr_tables).getTableStatus()) + " name: " + name.to_string();
    eosio_assert( (*itr_tables).getTableStatus() == T_WAIT_CRYPT, error.c_str());
    eosio_assert( (*itr_tables).next_player_index == this_player_index, "it's not your turn now");
    eosio_assert( cards.size() == 52, "Wrong the deck size");

    if((*itr_tables).rsa_key_flag == 1)
    {
        eosio_assert((*itr_tables).open_key.e.size() != 0, "No RSA open key");
        eosio_assert(player_rsa_keys.size() == 50, "Wrong keys count");
    
        for(Key key: player_rsa_keys)
            if( key.card_index == (*itr_tables).players[this_player_index].cards_indexes[0] ||
                key.card_index == (*itr_tables).players[this_player_index].cards_indexes[1] )
                    eosio_assert(false, "Wrong rsa keys indexes");
    }

    eosio::print(" player=",(*itr_tables).players[this_player_index].name);

    tables.modify(itr_tables, _self, [&] (auto& table){
        table.setNewDeck(cards);
        table.setNewInGameIndex(table.next_player_index, 1);

        if(table.rsa_key_flag == 1)
        {
            std::vector<Key> short_rsa_keys;
            uint8_t keys_count = table.current_game_players_count*2-2+5;
            for(int i=0; i<keys_count;i++)
                short_rsa_keys.push_back(player_rsa_keys[i]);
            table.players_rsa_keys[name] = short_rsa_keys;
        }

        table.current_players_received_count++;
        if(table.current_players_received_count == table.current_game_players_count)
        {
            table.current_players_received_count = 0;
            table.actMasterBlind();
        }
    });
}

ACTION pokercontract::act(eosio::name name, uint64_t table_id, uint64_t game_id, Act player_act, uint64_t timestamp, uint32_t trx_index)
{
    eosio::print(" IN ACT");
    uint8_t this_player_index;
    if(primary_checks(name, table_id, game_id, timestamp, trx_index, this_player_index) == false)
        return;

    auto itr_tables = tables.find(table_id);

    eosio::print(" player=",(*itr_tables).players[this_player_index].name);

    std::string error = "act: Wrong table status. Now is " + std::to_string((*itr_tables).getTableStatus()) + " name: " + name.to_string();
    eosio_assert( (*itr_tables).getTableStatus() == T_WAIT_PLAYERS_ACT, error.c_str());
    eosio_assert( (*itr_tables).next_player_index == this_player_index, "it's not your turn now");
    eosio_assert(player_act.act_ != ACT_FOLD, " wrong act type FOLD for act action");
    eosio_assert(player_act.bet_.symbol == eosio::symbol("EOS", 4), " wrong symbol_type");
    if(player_act.act_ != ACT_BET)
        eosio_assert( player_act.bet_.amount == 0, "Wrong bet value");

    player_act.description = player_act.act_;

    tables.modify(itr_tables, _self, [&] (auto& table){
        table.addNewAct(table.players[this_player_index], this_player_index, player_act);
        table.players[this_player_index].addNewAct(player_act);
        table.setLastTime();
        uint8_t res = table.setNextPlayerIndex();
        if(res == T_END_GAME)
            table.endGame();
        else if(res == T_END_ALL_IN_GAME)
            table.actAllInKeys();
        else if(res == ACT_NEW_ROUND)
            table.actMasterShowDown();
    });
}

ACTION pokercontract::actfold(eosio::name name, uint64_t table_id, uint64_t game_id, std::vector<Key> keys, uint64_t timestamp, uint32_t trx_index)
{
    eosio::print(" IN ACT FOLD");
    uint8_t this_player_index;
    if(primary_checks(name, table_id, game_id, timestamp, trx_index, this_player_index) == false)
        return;

    auto itr_tables = tables.find(table_id);

    std::string error = "act: Wrong table status. Now is " + std::to_string((*itr_tables).getTableStatus()) + " name: " + name.to_string();
    eosio_assert( (*itr_tables).getTableStatus() == T_WAIT_PLAYERS_ACT, error.c_str());
    eosio_assert( (*itr_tables).next_player_index == this_player_index, "it's not your turn now");
    
    uint8_t waiting_start_key_index = (*itr_tables).current_game_players_count * 2;
    if((*itr_tables).table_cards_indexes.empty() == false)
        waiting_start_key_index = (*itr_tables).table_cards_indexes.back() + 1;

    uint32_t waiting_keys_count = the_const_deck.size() - waiting_start_key_index;
    
    std::sort(keys.begin(), keys.end(), [](const Key a, const Key b) -> bool{
        return a.card_index < b.card_index;
    });

    eosio_assert(waiting_keys_count == keys.size(), "Wrong count of keys");
    auto itr_keys = keys.begin();
    for(int i = waiting_start_key_index; i < the_const_deck.size(); i++)
    {
        eosio_assert((*itr_keys).card_index == i, "Required key index not found");
        itr_keys++;
    }

    Act act_fold = Act(ACT_FOLD, eosio::asset(0,EOS_SYMBOL));

    tables.modify(itr_tables, _self, [&] (auto& table){
        table.addFoldKeys(keys);
        table.addNewAct(table.players[this_player_index], this_player_index, act_fold);
        table.players[this_player_index].addNewAct(act_fold);
        table.jobSetNextPlayerIndex();
    });

    auto itr_accounts = accounts.find(name.value);
    eosio_assert(itr_accounts != accounts.end(), "No such user");
    accounts.modify(itr_accounts, _self, [&] (auto& acnt){
        acnt.total_loss += (*itr_tables).players[this_player_index].sum_of_bets;
    });
}

ACTION pokercontract::setcardskeys(eosio::name name, uint64_t table_id, uint64_t game_id, std::vector<Key> keys, uint64_t timestamp, uint32_t trx_index)
{
    uint8_t this_player_index;
    if(primary_checks(name, table_id, game_id, timestamp, trx_index, this_player_index) == false)
        return;

    auto itr_tables = tables.find(table_id);

    uint8_t table_status = (*itr_tables).getTableStatus();
    eosio::print(" IN SETCARDSKEYS. Table status = ", (int)table_status, " name=",name);

    std::string error = "setcardskeys: Wrong table status. Now is " + std::to_string(table_status) + " name: " + name.to_string();
    eosio_assert( table_status == T_WAIT_KEYS_FOR_PLAYERS || table_status == T_WAIT_KEYS_FOR_SHOWDOWN ||
                  table_status == T_WAIT_ALL_KEYS || table_status == T_WAIT_ALLIN_KEYS , error.c_str());

    uint8_t keys_count = (*itr_tables).getWaitingKeysCount();
    eosio_assert( keys.size() == keys_count, "Wrong number of keys");

    std::sort(keys.begin(), keys.end(), [](const Key a, const Key b) -> bool{
        return a.card_index < b.card_index;
    });

    tables.modify(itr_tables, _self, [&] (auto& table){
        table.setEventsFromOutAndFoldPlayers();
        if(table.players[this_player_index].have_event == 0)
            table.addNewKeys(name, this_player_index, keys);
    });
    eosio::print(" SETCARDSKEYS END. Table status = ", (int)table_status);
}

ACTION pokercontract::resettable(eosio::name name, uint64_t table_id, uint64_t game_id, uint8_t table_status, uint64_t timestamp, uint32_t trx_index)
{
    eosio::print("resettable ", name);
    auto itr_tables = tables.find(table_id);
    if((*itr_tables).getTableStatus() != table_status)
        return;

    uint8_t this_player_index;
    if(primary_checks(name, table_id, game_id, timestamp, trx_index, this_player_index) == false)
        return;

    eosio::time_point now_time = eosio::time_point(eosio::microseconds(current_time()));
    
    uint32_t timeout = gstate.last_timeout_sec;
    if((*itr_tables).table_status == T_WAIT_PLAYERS_ACT)
        timeout = gstate.warning_timeout_sec + gstate.last_timeout_sec + 5;

    eosio::print("timeout=",timeout);

    uint64_t time_elapsed = now_time.time_since_epoch().to_seconds() - (*itr_tables).last_act_time.time_since_epoch().to_seconds();
    eosio::print("now_time=",now_time.time_since_epoch().to_seconds());
    eosio::print("table_time=",(*itr_tables).last_act_time.time_since_epoch().to_seconds());
    eosio::print("elapsed=",time_elapsed);

    if(time_elapsed < timeout)
        return;

    if ((*itr_tables).table_status == T_WAIT_PLAYER)
        return;

    tables.modify(itr_tables, _self, [&] (auto& table){

        if(time_elapsed >= timeout + gstate.delete_table_timeout_sec)
        {
            if(table.table_status != T_WAIT_END_GAME)
                if(table.players_count >= 3) // one new + 2 in dead game
                    table.deletetableGameStatistic();

            // return money
            for(Player& plr: table.players)
            {
                if(plr.name == name || plr.status == P_NO_PLAYER || plr.status == P_OUT)
                    continue;

                auto itr_accounts = accounts.find(plr.name.value);
                eosio_assert(itr_accounts != accounts.end(), "find assertion");

                //check recently connected players
                if( (*itr_accounts).connection_time > (*itr_tables).last_act_time)
                    continue;

                accounts.modify(itr_accounts, _self, [&] (auto& acnt){
                acnt.out_reason = "Dead table";
                if(acnt.table_id_.size() != 0)
                {
                    if(acnt.table_id_.back() == table.id)
                        acnt.table_id_.clear();
                }
            
                if(plr.status == P_WAIT_NEW_GAME)
                    acnt.quantity_ += plr.stack;
                else
                    acnt.quantity_ += plr.start_stack;
                });

                plr.status = P_NO_PLAYER;
                plr.clearGameInfo();
                plr.name.value = 0;
            }

            bool move_dealer = true;
            table.initNewGame(move_dealer);
            eosio::print(" resettable_res = deadtable");
            return;
        }

        std::vector<uint8_t> new_timeout, in_game;
        bool set_penalty_asset = table.getPenaltyAssetFlag();

        table.setPlayersTimeoutsAndPenalty(set_penalty_asset, new_timeout, in_game);

        if(table.table_status == T_WAIT_END_GAME)
        {
            bool move_dealer = true;
            table.initNewGame(move_dealer);
            eosio::print(" resettable_res=1");
            return;
        }

        if( (table.table_status >= T_WAIT_PLAYERS_ACT) && (table.table_status != T_WAIT_RSA_KEYS) && (table.rsa_key_flag == 1) && (in_game.size() > 0) )
        { 
            if(in_game.size() == 1)
            {
                eosio::print(" resettable_res= one player stayed in tournament mode");
                table.update_players_with_bets();
                table.endGame();
                return;
            }
            
            eosio::print(" resettable_res=2");
            for(uint8_t i: new_timeout)
            {
                if(table.players[i].cur_round_bets.amount != 0)
                    table.updateOutPlayerCurRoundBets(table.players[i], i);
            }

            table.saved_table_status = table.table_status;
            table.setTableStatus(T_WAIT_RSA_KEYS);
            table.setLastTime();
            
            for(Player& plr: table.players)
            {
                plr.late_trxs.clear();
                plr.applied_late_trxs.clear();
            }
            return;
        }

        if( (table.table_cards.size() == 5) && (in_game.size() > 1) )
        {
            for(uint8_t i: new_timeout)
            {
                if(table.players[i].cur_round_bets.amount != 0)
                    table.updateOutPlayerCurRoundBets(table.players[i], i);
            }

            if( (table.table_status == T_WAIT_ALLIN_KEYS) || 
                (table.table_status == T_WAIT_ALL_KEYS) )
            {
                table.update_players_with_bets();
                table.endGame();
                eosio::print(" resettable_res=4");
            }            
            else
            {
                table.jobSetNextPlayerIndex();
                eosio::print(" resettable_res=5");
            }
            return;
        }
        else // if table.cards.size() < 5 || in_game.size() == 0
        {
            for(uint8_t i:in_game)
            {
                table.players[i].stack = table.players[i].start_stack;
                table.current_bank -= table.players[i].sum_of_bets;                
            }

            eosio::asset plr_fine_part = eosio::asset(0, EOS_SYMBOL);

            if(table.current_bank.amount != 0)
            {
                eosio::asset master_pay_total = table.current_bank;

                if(in_game.size() != 0)
                {
                    eosio::asset fine_bank = table.current_bank * gstate.player_pay_percent/100;
                    plr_fine_part = fine_bank/in_game.size();
                    eosio_assert(plr_fine_part*in_game.size() <= fine_bank, "error too much plr_fine_part");
                    master_pay_total = table.current_bank - plr_fine_part*in_game.size();

                    for(uint8_t i: in_game)
                    {
                        auto itr_accounts = accounts.find(table.players[i].name.value);
                        eosio_assert(itr_accounts != accounts.end(), "No such user");
                        accounts.modify(itr_accounts, _self, [&] (auto& acnt){
                            acnt.quantity_ += plr_fine_part;
                        });
                    }    
                }

                gfine.fine_value += master_pay_total;
                global_fine.set(gfine, _self);
            }

            table.resettableGameStatistic();
            table.endResetGame(plr_fine_part);
            eosio::print(" resettable_res=6");

            return;
        }
    });

    // delete table if no players
    if((*itr_tables).getTableStatus() == T_DELETE)
        tables.erase(itr_tables);
}

ACTION pokercontract::sendendgame(eosio::name name, uint64_t table_id, uint64_t game_id, uint64_t timestamp, uint32_t trx_index)
{
    eosio::print(" IN SENDENDGAME ", name);
    globalstate gstate = global.get();
    eosio_assert(gstate.freezing == 0, "contract status is freezing");

    uint8_t this_player_index;
    if(primary_checks(name, table_id, game_id, timestamp, trx_index, this_player_index) == false)
        return;

    auto itr_tables = tables.find(table_id);

    std::string error = "sendendgame: Wrong table status. Now is " + std::to_string((*itr_tables).getTableStatus()) + " name: " + name.to_string();
    eosio_assert( (*itr_tables).getTableStatus() == T_WAIT_END_GAME, error.c_str());
    uint8_t status = (*itr_tables).players[this_player_index].status;
    eosio_assert( status == P_IN_GAME || status == P_FOLD, "Wrong player status");

    if((*itr_tables).players[this_player_index].have_event == 1)
        return;

    tables.modify(itr_tables, _self, [&] (auto& table){

        table.players[this_player_index].have_event = 1;

        if(++table.current_players_received_count == table.current_game_players_count)
        {
            bool move_dealer = true;
            if(table.history.back().result == R_TIMEOUT_RESET)
                move_dealer = false;
            table.initNewGame(move_dealer);
        }
    });
    
    if((*itr_tables).getTableStatus() == T_DELETE)
        tables.erase(itr_tables);
}

ACTION pokercontract::sendnewgame(eosio::name name, uint64_t table_id, uint64_t game_id, uint64_t timestamp, uint32_t trx_index)
{
    eosio::print(" IN SENDNEWGAME ", name);
    globalstate gstate = global.get();
    eosio_assert(gstate.freezing == 0, "contract status is freezing");

    uint8_t this_player_index;
    if(primary_checks(name, table_id, game_id, timestamp, trx_index, this_player_index) == false)
        return;

    auto itr_tables = tables.find(table_id);

    std::string error = "sendnewgame: Wrong table status. Now is " + std::to_string((*itr_tables).getTableStatus()) + " name: " + name.to_string();

    if((*itr_tables).getTableStatus() != T_WAIT_START_GAME)
        eosio::print(error.c_str());

    eosio_assert( (*itr_tables).getTableStatus() == T_WAIT_START_GAME, error.c_str());
    eosio_assert( (*itr_tables).players[this_player_index].status == P_IN_GAME, "Wrong player status");

    if((*itr_tables).players[this_player_index].have_event == 1)
        return;

    tables.modify(itr_tables, _self, [&] (auto& table){

        table.players[this_player_index].have_event = 1;

        if(++table.current_players_received_count == table.current_game_players_count)
        {
            table.current_players_received_count = 0;
            // T_WAIT_NEW_GAME -> T_WAIT_SHUFFLE
            table.setTableStatus(T_WAIT_SHUFFLE);
            
            for(Player& plr: table.players)
                plr.have_event = 0;
            table.setLastTime();
        }
    });
}

ACTION pokercontract::clear(name owner, uint64_t count)
{
    require_auth(owner);

    eosio_assert(owner == _self, "Only owner can clear tables");
    name contractname(CONTRACTNAME);

    auto tables_it = tables.begin();
    while( tables_it != tables.end())
    {
        tables_it = tables.erase(tables_it);
        if(--count == 0)
            return;
    }

    auto accounts_it = accounts.begin();
    while( accounts_it != accounts.end())
    {
        accounts_it = accounts.erase(accounts_it);
        if(--count == 0)
            return;
    }

    combos_index   combostbl(contractname,contractname.value);
    auto combos_it = combostbl.begin();
    while( combos_it != combostbl.end())
    {
        combos_it = combostbl.erase(combos_it);
        if(--count == 0)
            return;
    }

    global.remove();
}

ACTION pokercontract::clearstats(name owner, uint64_t count)
{
    require_auth(owner);

    eosio_assert(owner == _self, "Only owner can clear gamesstats");
    name contractname(CONTRACTNAME);

    statistic_index gamesstats(contractname,contractname.value);
    auto stats_it = gamesstats.begin();
    while( stats_it != gamesstats.end())
    {
        stats_it = gamesstats.erase(stats_it);
        if(--count == 0)
            return;
    }
}

ACTION pokercontract::cleargamesid(eosio::name owner, uint64_t count)
{
    require_auth(owner);

    eosio_assert(owner == _self, "Only owner can clear games id`s");  

    name contractname(CONTRACTNAME);

    account_index   accounts(contractname,contractname.value);

    for(auto account_it = accounts.begin(); account_it != accounts.end(); account_it++ ) 
    {
        if( (*account_it).games.size() == 0)
            continue;

        accounts.modify(account_it, _self, [&] (auto& acnt){
            acnt.games.clear();
        });

        if(--count == 0)
        {
            eosio::print("Have more accounts with game id`s");
            return;
        }
    }
    eosio::print("All accounts with game id`s cleared");
}

bool Table::getTimeoutType()
{
    bool many_players_timeout = false;

    switch(table_status)
    {
        case T_WAIT_END_GAME:
        case T_WAIT_START_GAME:
        case T_WAIT_KEYS_FOR_PLAYERS:
        case T_WAIT_KEYS_FOR_SHOWDOWN:
        case T_WAIT_ALL_KEYS:
        case T_WAIT_ALLIN_KEYS:
        {
            many_players_timeout = true;
            break;
        }
        case T_WAIT_SHUFFLE:
        case T_WAIT_CRYPT:
        case T_WAIT_PLAYERS_ACT:
        case T_WAIT_RSA_KEYS:
        {
            many_players_timeout = false;
            break;
        }
        default:
            eosio_assert(0,"Bad table status for timeout");
    }
    return many_players_timeout;
}

void Table::setPlayersTimeoutsAndPenalty(bool set_penalty, std::vector<uint8_t>& new_timeout, std::vector<uint8_t>& in_game)
{
    eosio::name contractname(CONTRACTNAME);
    account_index   accounts(contractname,contractname.value);
    bool many_players_timeout = getTimeoutType();
    uint8_t count = current_game_players_count;
    uint8_t i = next_player_index;

    while(count--)
    {
        if(players[i].status == P_OUT || players[i].status == P_TIMEOUT)
        {
            setNewInGameIndex(i, 1);
            continue;
        }

        if( ((many_players_timeout == true) && (players[i].have_event == 0)) ||
            ((many_players_timeout == false) && (next_player_index == i)) )
        {
            players[i].status = P_TIMEOUT;
            if(rsa_key_flag == 1)
                players[i].wait_rsa = 1;

            current_folds_count++;
            /*if(players[i].all_in_flag == P_ALL_IN)
                    allin_players_count--;*/

            new_timeout.push_back(i);

            auto itr_accounts = accounts.find(players[i].name.value);
            eosio_assert(itr_accounts != accounts.end(), "No such user");
            accounts.modify(itr_accounts, contractname, [&] (auto& acnt){
                if(set_penalty == true)
                    acnt.penalty += players[i].sum_of_bets;

                acnt.penalty_count++;
            });
        }
        else
            in_game.push_back(i);

        setNewInGameIndex(i, 1);
    }

    // no chance for normal end of game
    if( (set_penalty == false) && (in_game.size() == 0) )
        for(uint8_t i: in_game)
        {
            auto itr_accounts = accounts.find(players[i].name.value);
            eosio_assert(itr_accounts != accounts.end(), "No such user");
            accounts.modify(itr_accounts, contractname, [&] (auto& acnt){
                    acnt.penalty += players[i].sum_of_bets;
            });            
        }
}

ACTION pokercontract::withdraw(eosio::name name)
{
    require_auth(name);
    eosio::name contractname(CONTRACTNAME);
    auto itr_accounts = accounts.find(name.value);
    eosio_assert(itr_accounts != accounts.end(), "No such user");

    eosio::asset quantity = (*itr_accounts).getBalance();

    if(quantity.amount == 0)
    {
        eosio::print(" Withdraw amount == 0. Transfer not completed.");
        return;
    }

    action{
            permission_level{_self, "active"_n},
            "eosio.token"_n,
            "transfer"_n,                
            std::make_tuple( _self, name, quantity, std::string("Thank you for playing with us!"))
            }.send();

    accounts.modify(itr_accounts, contractname, [&] (auto& acnt){
        acnt.quantity_.amount = 0;
    });
}

ACTION pokercontract::testrake(eosio::name name)
{
    require_auth(name);
    eosio_assert(name == _self, "Only owner can use testrake");

/****   delete graphenedevs account ************/
    eosio::name graphenedevs(GRAPHENEDEVS);
    auto itr_accounts = accounts.find(graphenedevs.value);
    if(itr_accounts != accounts.end())
        accounts.erase(itr_accounts);

/****   write new r value = users rake sum ************/
    eosio::asset users_rakes = eosio::asset(0, EOS_SYMBOL);

    for(auto itr_acc = accounts.begin(); itr_acc != accounts.end(); itr_acc++ ) 
        users_rakes += (*itr_acc).rake;

    global_state_singleton global(_self, _self.value);
    globalstate gstate = global.get();
    gstate.r = users_rakes;
    global.set(gstate, _self);
/*********************************************************************/
}

void insertCardsInVector(const std::vector<Card>& cards1, 
                      const std::vector<Card>& cards2,
                      const std::vector<Card>& cards3,
                      const std::vector<Card>& cards4,
                      const std::vector<Card>& cards5,
                      const std::vector<Card>& cards6,
                      const std::vector<Card>& cards7,
                      const std::vector<Card>& cards8,
                      const std::vector<Card>& cards9, 
                      std::vector<std::multiset<Card>>&    all_cards)
{
std::multiset<Card> cards;

for(Card card: cards1)
    cards.insert(card);
all_cards.push_back(cards);
cards.clear();

if(cards2.size() < 5)
    return;
for(Card card: cards2)
    cards.insert(card);
all_cards.push_back(cards);
cards.clear();

if(cards3.size() < 5)
    return;
for(Card card: cards3)
    cards.insert(card);
all_cards.push_back(cards);
cards.clear();

if(cards4.size() < 5)
    return;
for(Card card: cards4)
    cards.insert(card);
all_cards.push_back(cards);
cards.clear();

if(cards5.size() < 5)
    return;
for(Card card: cards5)
    cards.insert(card);
all_cards.push_back(cards);
cards.clear();

if(cards6.size() < 5)
    return;
for(Card card: cards6)
    cards.insert(card);
all_cards.push_back(cards);
cards.clear();

if(cards7.size() < 5)
    return;
for(Card card: cards7)
    cards.insert(card);
all_cards.push_back(cards);
cards.clear();

if(cards8.size() < 5)
    return;
for(Card card: cards8)
    cards.insert(card);
all_cards.push_back(cards);
cards.clear();

if(cards9.size() < 5)
    return;
cards.clear();
for(Card card: cards9)
    cards.insert(card);
all_cards.push_back(cards);
cards.clear();
}

ACTION pokercontract::testcombos( eosio::name name,
                      std::vector<Card> cards1, 
                      std::vector<Card> cards2,
                      std::vector<Card> cards3,
                      std::vector<Card> cards4,
                      std::vector<Card> cards5,
                      std::vector<Card> cards6,
                      std::vector<Card> cards7,
                      std::vector<Card> cards8,
                      std::vector<Card> cards9                                      
                      )
{
std::vector<std::multiset<Card>>    all_cards;
std::vector<ComboWin>               combos;
int comboNum = 0;

require_auth(name);
eosio::name contractname(CONTRACTNAME);
eosio_assert(cards1.size() >= 5, "combo1 size < 5");
insertCardsInVector(cards1, cards2, cards3, cards4, cards5, cards6, cards7, cards8, cards9, all_cards);

    for(auto itr = all_cards.begin(); itr != all_cards.end(); itr++)
    {
        ComboWin combo_i;
        int get_combo_res = getCombination((*itr), combo_i.combo);
        eosio_assert(get_combo_res,"error get combination");
        comboNum++;
        combo_i.comboNumber = comboNum;
        combos.push_back(combo_i);
    }

    std::sort(combos.begin(), combos.end(), [](const ComboWin& a, const ComboWin& b) -> bool {
        return a.combo > b.combo;
    });

    for(auto itr = combos.begin(); itr != combos.end(); itr++)
    {
        (*itr).win = 1;
        auto itr_next = std::next(itr,1);
        if(itr_next == combos.end())
            break;
        if( (*itr).combo > (*itr_next).combo )
            break;
    }

    combos_index   combostbl(contractname,contractname.value);
    auto itr = combostbl.find(name.value);

    if(itr == combostbl.end())
    {
        itr = combostbl.emplace(name, [&] (auto& combo) {
            combo.name = name;
            combo.combos = combos;
        });
    }
    else
        combostbl.modify(itr, name, [&] (auto& combo){
            combo.combos = combos;
        });
}

ACTION pokercontract::sendmsg(eosio::name name, uint64_t table_id, uint64_t game_id, std::string msg)
{
    require_auth(name);

    eosio_assert(msg.length() <= 100, "Message is too long");

    auto itr_accounts = accounts.find(name.value);
    eosio_assert(itr_accounts != accounts.end(), "No such user");
    eosio_assert(table_id ==  (*itr_accounts).getTableId(), "Wrong table id");

    auto itr_tables = tables.find(table_id);
    eosio_assert(itr_tables != tables.end(), "No such table");

    std::string auth_msg = name.to_string() + ": " + msg;

    tables.modify(itr_tables, _self, [&] (auto& table){
        if(table.history.size() == 0)
        {
            GameResult res;
            table.history.push_back(res);
        }
        table.history.back().log.push_back(auth_msg);
    });
}

ACTION pokercontract::setrsakeys(eosio::name name, uint64_t table_id, uint64_t game_id, std::map<eosio::name, std::vector<Key>> players_keys)
{
    require_auth(name);

    eosio::name blackbox(BLACKBOXACNT);
    eosio_assert(name == blackbox, " Error admin! ");

    auto itr_tables = tables.find(table_id);
    eosio_assert(itr_tables != tables.end(), "No such table");
    eosio_assert( (*itr_tables).game_id == game_id, "Wrong game id");
    if((*itr_tables).table_status != T_WAIT_RSA_KEYS)
        return;

    tables.modify(itr_tables, _self, [&] (auto& table){
        table.setBlackBoxKeys(players_keys);
    });

}

ACTION pokercontract::setopenkey(eosio::name name, uint64_t table_id, RsaOpenKey open_key)
{
    require_auth(name);

    eosio::name blackbox(BLACKBOXACNT);
    eosio_assert(name == blackbox, " Error admin! ");

    auto itr_tables = tables.find(table_id);
    eosio_assert(itr_tables != tables.end(), "No such table");

    if( (*itr_tables).open_key.e.size() != 0 )
        return;

    tables.modify(itr_tables, _self, [&] (auto& table){
        table.open_key = open_key;
    });
}

ACTION pokercontract::setnewref(eosio::name owner, std::vector<eosio::name> referals, uint32_t new_percent)
{
    require_auth(owner);
    eosio_assert(owner == _self, "Only owner can run setnewref");

    eosio_assert(new_percent <= 100, "new percent must be less or equal 100");

    // for referals
    for(eosio::name name:referals)
    {
        auto itr_ref_accounts = accounts.find( name.value);
        if(itr_ref_accounts != accounts.end())
        {
            accounts.modify(itr_ref_accounts, _self,[&](auto& account){
                if(account.reserve.size() == 0)
                {
                    account.reserve.push_back(eosio::asset(0, EOS_SYMBOL));
                    account.reserve.push_back(eosio::asset(0, EOS_SYMBOL));
                } 
                if(account.reserve.size() != 3)
                    account.reserve.push_back(eosio::asset(new_percent, EOS_SYMBOL));
                else
                    account.reserve[2] = eosio::asset(new_percent, EOS_SYMBOL);
            });
        }
    }

    // for users
    for(auto account_it = accounts.begin(); account_it != accounts.end(); account_it++ ) 
    {
        for(eosio::name name:referals)
            if(name == account_it->referal_name[0])
            {
                accounts.modify(account_it, _self, [&] (auto& acnt){
                    if(acnt.reserve.size() == 0)
                    {
                        acnt.reserve.push_back(eosio::asset(0, EOS_SYMBOL));
                        acnt.reserve.push_back(eosio::asset(0, EOS_SYMBOL));
                    }
                    acnt.reserve[0].amount = new_percent;
                });
            }
    }
}

#undef EOSIO_DISPATCH

#define EOSIO_DISPATCH( TYPE, MEMBERS ) \
extern "C" { \
    void apply( uint64_t receiver, uint64_t code, uint64_t action ) { \
        auto self = receiver; \
        if( action == "onerror"_n.value ) { \
            eosio_assert(code == ("eosio"_n).value, "onerror action's are only valid from the \"eosio\" system account"); \
        } \
        if( code == self ) { \
            if (action != ("transfer"_n).value) {\
                switch( action ) { \
                    EOSIO_DISPATCH_HELPER( TYPE, MEMBERS ) \
                } \
            }\
        } \
        else if (code == ("eosio.token"_n).value && action == ("transfer"_n).value ) {\
            execute_action(eosio::name(receiver), eosio::name(code), &pokercontract::transfer);\
        }\
    } \
}

EOSIO_DISPATCH(pokercontract,   (init) 
                                (clear) 
                                (clearstats)
                                (setparams)
                                (transfer) 
                                (connecttable) 
                                (outfromtable) 
                                (shuffleddeck) 
                                (crypteddeck) 
                                (act)
                                (actfold) 
                                (setcardskeys) 
                                (resettable) 
                                (sendendgame)
                                (sendnewgame)
                                (withdraw)
                                (testcombos)
                                (testrake)
                                (cleargamesid)
                                (sendmsg)
                                (setopenkey)
                                (setrsakeys)
                                (setref)
                                (setnewref)
                                )