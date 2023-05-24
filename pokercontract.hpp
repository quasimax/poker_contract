#ifndef POKER_CONTRACT_H
#define POKER_CONTRACT_H

#include <eosiolib/name.hpp>
#include <eosiolib/contract.hpp>
#include <eosiolib/eosio.hpp>
#include <eosiolib/singleton.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/time.hpp>
#include <eosiolib/symbol.hpp>
#include "card.hpp"
#include "combinations.hpp"

using namespace eosio;

#define EOS_SYMBOL  eosio::symbol("EOS", 4)
#define CONTRACTNAME "dcdpcontract"
#define GRAPHENEDEVS "graphenedevs"
#define BLACKBOXACNT "dcdpblackbox"
#define referal_check "referal"

struct [[eosio::table, eosio::contract("pokercontract")]]
Account
{
    eosio::name             name_;
    eosio::asset            quantity_;
    uint8_t                 autorebuy = 0;
    uint8_t                 buyin_sb;
    std::vector<uint64_t>   table_id_;
    uint32_t                count_of_wins = 0;
    uint32_t                count_of_defeats = 0;
    std::string             out_reason;
    eosio::time_point       connection_time;
    eosio::asset            rake  = eosio::asset(0, EOS_SYMBOL);
    std::vector<uint64_t>   games;
    uint32_t                penalty_count = 0;
    eosio::asset            total_win = eosio::asset(0, EOS_SYMBOL);
    eosio::asset            total_loss = eosio::asset(0, EOS_SYMBOL);
    eosio::asset            penalty = eosio::asset(0, EOS_SYMBOL);
    std::vector<eosio::name>    referal_name;
    std::vector<eosio::asset>   reserve;

    uint64_t primary_key() const { return name_.value;}

    eosio::name getName();
    void setName(eosio::name name);

    bool hasTableId() const;

    uint64_t getTableId() const;
    void setTableId(uint64_t table_id);

    eosio::asset getBalance() const;
    void addBalance(eosio::asset quantity);
};

enum ActTypes
{
    ACT_SMALL_BLIND,
    ACT_BIG_BLIND,
    ACT_BET, // includes CALL and RAISE // 2
    ACT_FOLD,
    ACT_CHECK,
    ACT_NEW_ROUND,
    // descriptions
    ACT_CALL,
    ACT_RISE, // 7
    ACT_ALLIN
};

struct Act
{
    Act():act_(0),bet_(0, EOS_SYMBOL),description(0)
    {
    }

    Act(uint8_t act, eosio::asset bet)
    {
        act_ = act;
        bet_ = bet;
        description = act_;
    }

    uint8_t         act_;
    eosio::asset    bet_; // only for a ACT_BET, SMALL and BIG BLINDs
    uint8_t         description;

    EOSLIB_SERIALIZE(Act, (act_) (bet_) (description)) 
};

struct PlayerAct
{
    PlayerAct()
    {        
    }

    PlayerAct(uint8_t index, eosio::name name, Act act)
    {
        player_index = index;
        name_ = name;
        act_ = act;
    }

    uint8_t         player_index;
    eosio::name    name_;
    Act             act_;

    EOSLIB_SERIALIZE(PlayerAct, (player_index)(name_) (act_))
};

struct Key
{
    std::vector<uint8_t>    data;
    uint8_t                 card_index;

    std::vector<uint8_t>    s;
    std::vector<uint8_t>    m;

    EOSLIB_SERIALIZE(Key, (data) (card_index) (s) (m)) 
};

struct RsaOpenKey
{
    std::vector<uint8_t>    e;
    std::vector<uint8_t>    n;

    EOSLIB_SERIALIZE(RsaOpenKey, (e) (n) ) 
};

enum PlayerStatus
{
    P_WAIT_NEW_GAME,
    P_IN_GAME,
    P_ALL_IN, // only for all_in_flag not for status!
    P_FOLD,
    P_TIMEOUT,
    P_DECRYPT_ERROR,
    P_OUT,
    P_NO_PLAYER
};

struct Player
{
    Player(eosio::name o = eosio::name()) : name(o){}
    
    eosio::name             name;
    uint8_t                 status = P_WAIT_NEW_GAME;
    uint8_t                 extra_bb_status = 0;
    uint8_t                 wait_for_bb = 0;
    uint8_t                 have_event = 0;
    uint8_t                 wait_rsa = 0;
    eosio::asset            start_stack = eosio::asset(0, EOS_SYMBOL);
    eosio::asset            stack = eosio::asset(0, EOS_SYMBOL);
    eosio::asset            sum_of_bets = eosio::asset(0, EOS_SYMBOL);
    eosio::asset            rake = eosio::asset(0, EOS_SYMBOL);
    eosio::asset            cur_round_bets = eosio::asset(0, EOS_SYMBOL);
    eosio::asset            all_in_bank = eosio::asset(0, EOS_SYMBOL);
    uint8_t                 all_in_round = 0;
    uint8_t                 count_of_acts = 0;
    std::vector<uint8_t>    cards_indexes;
    std::vector<Act>        acts;
    uint8_t                 all_in_flag = 0;
    std::set<uint32_t>      late_trxs;
    std::set<uint32_t>      applied_late_trxs;

    void clearGameInfo();
    void addNewAct(const Act& act);

    EOSLIB_SERIALIZE(Player, (name) 
                            (status)
                            (extra_bb_status)
                            (wait_for_bb)
                            (have_event)
                            (wait_rsa)
                            (start_stack)
                            (stack)
                            (sum_of_bets)
                            (rake)
                            (cur_round_bets)
                            (all_in_bank)
                            (all_in_round)
                            (count_of_acts)
                            (cards_indexes)
                            (acts)
                            (all_in_flag)
                            (late_trxs)
                            (applied_late_trxs))
};

enum TableStatus
{
    T_WAIT_PLAYER,
    //T_NO_STATUS,
    T_WAIT_START_GAME = 2,
    T_WAIT_SHUFFLE,
    T_WAIT_CRYPT,
    //T_NO_STATUS,
    T_WAIT_KEYS_FOR_PLAYERS = 6,
    T_WAIT_PLAYERS_ACT,
    T_WAIT_KEYS_FOR_SHOWDOWN, // 8
    //T_NO_STATUS,
    T_WAIT_ALL_KEYS = 10,
    T_WAIT_ALLIN_KEYS, // 11
    T_END_GAME,
    T_WAIT_END_GAME,
    T_END_ALL_IN_GAME,
    T_DELETE,
    T_WAIT_RSA_KEYS
};

struct SidePot
{
    eosio::asset    bank;
    eosio::asset    win;
};

struct PlayerHistoryInfo
{
    eosio::name             name;
    uint8_t                 show = 0;
    eosio::asset            winnings; // =0 if no winnings
    std::vector<SidePot>    side_pots;
    std::vector<Card>       hand;
    Combination             combo;
    
    EOSLIB_SERIALIZE(PlayerHistoryInfo, (name) (show) (winnings) (side_pots) (hand) (combo)) 
};

const bool operator > (const PlayerHistoryInfo& a, const PlayerHistoryInfo& b) { return a.combo > b.combo; }

enum ResultGame
{
    R_IN_GAME,
    R_NORMAL,
    R_TIMEOUT_RESET,
    R_DEAD_TABLE_RESET,
    R_DECRYPT_ERROR
};

struct GameResult
{
    uint8_t                         result;
    eosio::asset                    start_bank = eosio::asset(0, EOS_SYMBOL);
    float                           rake_percent = 0;
    eosio::asset                    bank_rake_asset = eosio::asset(0, EOS_SYMBOL);
    eosio::asset                    referal_rake_asset = eosio::asset(0, EOS_SYMBOL);
    eosio::asset                    bank_unconsumed = eosio::asset(0, EOS_SYMBOL);
    eosio::asset                    bank = eosio::asset(0, EOS_SYMBOL);
    std::vector<PlayerHistoryInfo>  players_info;
    std::vector<std::string>        log;

    EOSLIB_SERIALIZE(GameResult,    (result)
                                    (start_bank)
                                    (rake_percent)
                                    (bank_rake_asset)
                                    (referal_rake_asset)
                                    (bank_unconsumed)
                                    (bank)
                                    (players_info)
                                    (log)) 
};

const std::vector<Card> the_const_deck = 
{Card(0,2), Card(0,3), Card(0,4), Card(0,5), Card(0,6), Card(0,7), Card(0,8), Card(0,9), Card(0,10), Card(0,11), Card(0,12), Card(0,13), Card(0,14),
 Card(1,2), Card(1,3), Card(1,4), Card(1,5), Card(1,6), Card(1,7), Card(1,8), Card(1,9), Card(1,10), Card(1,11), Card(1,12), Card(1,13), Card(1,14),
 Card(2,2), Card(2,3), Card(2,4), Card(2,5), Card(2,6), Card(2,7), Card(2,8), Card(2,9), Card(2,10), Card(2,11), Card(2,12), Card(2,13), Card(2,14),
 Card(3,2), Card(3,3), Card(3,4), Card(3,5), Card(3,6), Card(3,7), Card(3,8), Card(3,9), Card(3,10), Card(3,11), Card(3,12), Card(3,13), Card(3,14)};

struct [[eosio::table, eosio::contract("pokercontract")]]
GamesStatistic
{
    uint64_t                        id;
    eosio::time_point               start_time;
    eosio::time_point               end_time;
    eosio::asset                    small_blind;
    uint8_t                         players_count;
    eosio::asset                    bank;
    eosio::asset                    rake;
    std::vector<Card>               table_cards;
    std::vector<eosio::name>        players;
    std::vector<PlayerHistoryInfo>  players_info;
    uint8_t                         result_table_status;
    uint8_t                         status = 0;
    std::vector<eosio::name>        timeout_players;
    
    uint64_t primary_key() const { return id;}
};

struct Debug
{
uint64_t    timestamp;
uint8_t     table_status;
};

struct [[eosio::table, eosio::contract("pokercontract")]]
Table
{
    uint64_t                id = 0;
    uint64_t                game_id = std::numeric_limits<uint64_t>::max();
    eosio::asset            small_blind;
    uint8_t                 max_players = 9; 
    uint8_t                 players_count = 0;
    uint8_t                 rsa_key_flag = 0;

    //std::vector<Debug>      debug;

    RsaOpenKey              open_key;

    uint8_t                 table_status = T_WAIT_PLAYER;
    uint8_t                 saved_table_status = 0;

    uint8_t                 current_game_players_count = 0;

    uint8_t                 allin_players_count = 0;
    uint8_t                 current_folds_count = 0; // P_OUT, P_TIMEOUT, P_FOLD
    uint8_t                 current_players_received_count = 0;
    
    uint8_t                 current_game_round = 0;
    std::vector<uint8_t>    current_round_players_bet_acts; // in one round for side pots [][][][][][][]
    std::vector<uint8_t>    players_with_bets; // for side pot by rounds

    eosio::asset            current_bet;
    eosio::asset            current_bank;
    eosio::asset            bank;
    eosio::asset            table_cur_round_bets;
    eosio::asset            not_returned_bets;

    uint8_t                 dealer_index;
    uint8_t                 sb_index;
    uint8_t                 bb_index;
    uint8_t                 next_player_index;

    eosio::time_point       last_act_time;
    uint64_t                timestamp;

    std::vector<uint8_t>    possible_moves;
    std::vector<eosio::asset> raise_variants;

    std::vector<uint8_t>    waiting_keys_indexes;
    std::vector<uint8_t>    table_cards_indexes; // 3, 4 or 5 max
    std::vector<Card>       table_cards; // 3, 4 or 5 max
    std::vector<Card>       the_deck_of_cards; // 52 cards

    std::vector<Player>     players;
    std::vector<PlayerAct>  players_acts;

    std::vector<GameResult> history;

    std::vector<Key>        all_keys; // current_game_players_count*2 keys by card indexes

    std::map<eosio::name, std::vector<Key>> players_rsa_keys;

    uint64_t primary_key() const { return id;}
    uint64_t by_last_act_time() const { return (uint64_t)(last_act_time.elapsed.count());}

    uint8_t getTableStatus() const;
    void setTableStatus(const uint8_t new_status);

    uint8_t getWaitingKeysCount() const;
    void addNewKeys(eosio::name name, uint8_t player_index, std::vector<Key> keys);
    void addFoldKeys(std::vector<Key> keys);

    void addNewPlayer(const eosio::name& name, const eosio::asset& stack, uint8_t wait_for_bb);
    
    void setEventsFromOutPlayers();
    void setEventsFromOutAndFoldPlayers();
    
    void updateOutPlayerCurRoundBets(Player& out_plr, uint8_t plr_index);
    void out_player(const eosio::name& name, const uint8_t plr_index, std::vector<Key> keys);

    void saveKeys(uint8_t plr_index, std::vector<Key>& keys);
    void out_player_new(const eosio::name& name, const uint8_t plr_index, std::vector<Key> keys);

    void setLastTime();

    void newGameStatistic();
    void endGameStatistic() const;
    void resettableGameStatistic() const;
    void deletetableGameStatistic() const;

    bool checkMaxRaiseValue(const eosio::asset& value);
    void setRaiseVariants();
    void setPossibleMoves();
    
    void initTheDeckOfCards();
    void setNewDeck(const std::vector<Card>& cards);

    void setNewInGameIndex(uint8_t& index, uint8_t offset);
    void moveDealerIndex();
    void setDealerIndex(bool move_dealer);
    void moveBigBlindIndex(uint8_t& index, uint8_t offset);
    uint8_t setNextPlayerIndex();
    void jobSetNextPlayerIndex();

    void clearGameInfo();
    void setNoPlayersAndRefillStack();
    void setExtraBBPlayers();
    void setCurrentGamePlayersCount();
    void setPlayersCount();
    bool zeroPlayers();
    bool onlyOnePlayer();
    void cutNoPlayers();
    void initNewGame(bool move_dealer);

    bool checkEndGame() const;
    bool checkEndAllInGame() const;
    void returnBetsOdds();

    uint8_t getCountOfWinners(const std::vector<PlayerHistoryInfo>&  players_info);

    void getAllInSortedPlayers(std::vector<Player>& sorted_players);
    void getComboSortedPlayers(const std::vector<Player>& unsorted, std::vector<PlayerHistoryInfo>& comboSortedPlayers);

    void calculateWinners(eosio::asset bank, std::vector<PlayerHistoryInfo>&  players, bool all_in);

    bool decryptCardByOneKey(Card& card, Key& key);
    bool decryptCardByAllKeys(Card& card, int card_index);
    void decryptPlayersCards();

    void saveOneWinnerHistory(GameResult& res);
    void saveAllInHistory(GameResult& res);
    void setShowDown(GameResult& res);

    void update_players_with_bets();
    void setNewRoundAct();

    void actMasterBlind();
    void setCardsIndexesToPlayers();
    void actAllInKeys();
    void actMasterShowDown();
    void addNewAct(Player& player, uint8_t player_index, Act& act);
    void endGame();
    void endResetGame(eosio::asset& plr_fine_part);
    bool getTimeoutType();
    bool getPenaltyAssetFlag();
    bool isWaitKeys();
    void setPlayersTimeoutsAndPenalty(bool set_penalty, std::vector<uint8_t>& new_timeout, std::vector<uint8_t>& in_game);

    void setBlackBoxKeys(std::map<eosio::name, std::vector<Key>>& players_keys);
};

struct [[eosio::table, eosio::contract("pokercontract")]]
globalstate{
        std::vector<eosio::asset>  small_blind_values;
        std::vector<uint8_t>    max_players_count_values;
        uint8_t                 penalty_percent;
        eosio::asset            max_penalty_value;
        uint8_t                 player_pay_percent;
        uint8_t                 master_pay_percent;
        float                   rake_percent;
        eosio::asset            max_rake_value;
        uint32_t                warning_timeout_sec;
        uint32_t                last_timeout_sec;
        uint32_t                delete_table_timeout_sec;
        uint32_t                delete_tables_count;
        uint8_t                 freezing;
        std::string             client_version = "";

        uint32_t                min_sb_buyin;
        uint32_t                max_sb_buyin;
        uint32_t                version;
        eosio::asset            r;
};

struct [[eosio::table, eosio::contract("pokercontract")]]
globalfine{
    eosio::asset  fine_value = eosio::asset(0,EOS_SYMBOL);
    eosio::asset           u = eosio::asset(0,EOS_SYMBOL);
};

struct [[eosio::table, eosio::contract("pokercontract")]]
globalref{
    uint32_t percent = 3;
};

struct ComboWin
{
    uint8_t comboNumber;
    uint8_t win = 0;
    Combination combo;
};

struct [[eosio::table, eosio::contract("pokercontract")]]
ComboSet 
{
    eosio::name              name;
    std::vector<ComboWin>    combos;
    uint64_t primary_key() const { return name.value;}
};

using account_index = multi_index<"accounts"_n, Account>;
using  table_index =  multi_index<"tables"_n, Table, 
              indexed_by<"bylasttime"_n, const_mem_fun< Table, uint64_t, &Table::by_last_act_time>>>;
using  global_state_singleton = singleton<"globalstate"_n, globalstate>;
using  global_fine_singleton = singleton<"globalfine"_n, globalfine>;
using  global_ref_singleton = singleton<"globalref"_n, globalref>;

using combos_index = multi_index<"combostbl"_n, ComboSet>;

using statistic_index = multi_index<"gamesstats"_n, GamesStatistic>;

CONTRACT pokercontract : public contract 
{
    public:
        using contract::contract;
        pokercontract(name self, name code, datastream<const char*> ds) : contract(self, code, ds), 
                        accounts(_self, _self.value),
                        tables(_self, _self.value),
                        global(_self, _self.value),
                        global_fine(_self, _self.value),
                        global_ref(_self, _self.value)
      {
        gstate = global.exists() ? global.get() : get_default_parameters();

        if(global_fine.exists())
            gfine = global_fine.get();
        else
        {
            globalfine gf;
            global_fine.set(gf, _self);
            gfine = gf;
        }

        if(global_ref.exists())
            gref = global_ref.get();
        else
        {
            globalref gr;
            global_ref.set(gr, _self);
            gref = gr;
        }
      }

    bool primary_checks(eosio::name name, uint64_t table_id, uint64_t game_id, uint64_t timestamp, uint32_t trx_index, uint8_t& player_index);

    ACTION init(name owner, std::string client_version);
    ACTION clear(name owner, uint64_t count);
    ACTION clearstats(name owner, uint64_t count);
    ACTION setparams(eosio::name owner, globalstate& gs);
    ACTION setref(eosio::name owner, uint32_t percent);
    ACTION setnewref(eosio::name owner, std::vector<eosio::name> referals, uint32_t new_percent);

    ACTION transfer(name from, name to, asset quantity, std::string memo);

    ACTION connecttable(eosio::name name, eosio::asset small_blind, uint8_t max_players, std::vector<uint8_t> client_version, 
                        uint8_t autorebuy, uint8_t buyin_sb, uint8_t wait_for_bb, uint8_t rsa_key_flag);
    ACTION outfromtable2(eosio::name name, uint64_t table_id, uint64_t game_id, std::vector<Key> keys);
    ACTION outfromtable(eosio::name name, uint64_t table_id, uint64_t game_id, std::vector<Key> keys);

    ACTION shuffleddeck(eosio::name name, uint64_t table_id, uint64_t game_id, std::vector<Card> cards, uint64_t timestamp, uint32_t trx_index);
    ACTION  crypteddeck(eosio::name name, uint64_t table_id, uint64_t game_id, std::vector<Card> cards, uint64_t timestamp, uint32_t trx_index, 
                        std::vector<Key> player_rsa_keys);
    ACTION          act(eosio::name name, uint64_t table_id, uint64_t game_id, Act act, uint64_t timestamp, uint32_t trx_index);
    ACTION      actfold(eosio::name name, uint64_t table_id, uint64_t game_id, std::vector<Key> keys, uint64_t timestamp, uint32_t trx_index);
    ACTION setcardskeys(eosio::name name, uint64_t table_id, uint64_t game_id, std::vector<Key> keys, uint64_t timestamp, uint32_t trx_index);
    ACTION   setrsakeys(eosio::name name, uint64_t table_id, uint64_t game_id, std::map<eosio::name, std::vector<Key>> players_keys);
    ACTION   resettable(eosio::name name, uint64_t table_id, uint64_t game_id, uint8_t table_status, uint64_t timestamp, uint32_t trx_index);
    ACTION  sendendgame(eosio::name name, uint64_t table_id, uint64_t game_id, uint64_t timestamp, uint32_t trx_index);
    ACTION  sendnewgame(eosio::name name, uint64_t table_id, uint64_t game_id, uint64_t timestamp, uint32_t trx_index);

    ACTION   setopenkey(eosio::name name, uint64_t table_id, RsaOpenKey open_key);
    
    ACTION     withdraw(eosio::name name);
    ACTION     sendmsg(eosio::name name, uint64_t table_id, uint64_t game_id, std::string msg);

    ACTION testcombos(eosio::name name,
                      std::vector<Card> cards1, 
                      std::vector<Card> cards2,
                      std::vector<Card> cards3,
                      std::vector<Card> cards4,
                      std::vector<Card> cards5,
                      std::vector<Card> cards6,
                      std::vector<Card> cards7,
                      std::vector<Card> cards8,
                      std::vector<Card> cards9                                      
                      );
    ACTION testrake(eosio::name name);
    ACTION cleargamesid(eosio::name owner, uint64_t count);

private:
    account_index   accounts;
    table_index     tables;

    global_state_singleton global;
    globalstate gstate;
    global_fine_singleton global_fine;
    globalfine gfine;
    global_ref_singleton global_ref;
    globalref gref;

    globalstate get_default_parameters();
};

#endif