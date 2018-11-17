#include <eosiolib/singleton.hpp>
#include "utils.hpp"

#define MINBET 1000
#define EOS_SYMBOL S(4, EOS)
#define CASINOSEVENS N(sevenscasino)

/// @abi table tbets i64
struct bets {
    uint64_t id;
    uint64_t game_id;
    account_name player;
    uint64_t roll_under;
    asset amount;
    string player_seed;
    checksum256 house_seed_hash;
    uint64_t created_at;
    account_name referrer;
    uint64_t primary_key() const { return id; }
    account_name by_player() const { return player; }

    EOSLIB_SERIALIZE(bets, (id)(game_id)(player)(roll_under)(amount)(player_seed)(house_seed_hash)(created_at)(referrer))
};

/// @abi table tenv i64
struct environments {
    public_key pub_key;
    uint8_t casino_fee;
    double ref_bonus;
    double player_bonus;
    asset locked;
    uint64_t next_id;
    uint64_t primary_key() const { return 0; }

    EOSLIB_SERIALIZE(environments, (pub_key)(casino_fee)(ref_bonus)(player_bonus)(locked)(next_id))
};

///@abi table tlogs i64
struct logs {
    uint64_t game_id;
    asset amount;
    asset payout;
    uint64_t created_at;
    uint64_t primary_key() const { return game_id; }
    //string by_game_id() const { return game_id; }

    EOSLIB_SERIALIZE(logs, (game_id)(amount)(payout)(created_at))
};

struct results {
    uint64_t id;
    uint64_t game_id;
    account_name player;
    asset amount;
    uint64_t roll_under;
    uint64_t random_roll;
    asset payout;
    string player_seed;
    checksum256 house_seed_hash;
    signature sig;
    account_name referrer;
};

typedef multi_index<N(tbets), bets, indexed_by<N(player), const_mem_fun<bets, account_name, &bets::by_player>>> _tbet;
typedef multi_index<N(tlogs), logs> _tlogs;
typedef singleton<N(tenv), environments> _tenvironments;
