#include <eosiolib/singleton.hpp>
#include "utils.hpp"

#define MINBET 1000
#define EOS_SYMBOL symbol("EOS", 4)
#define SVNS_SYMBOL symbol("SVNS", 4)
#define HOUSE name("casinosevens")
#define SEVENSHELPER name("sevenshelper")

struct info
{
    uint64_t id;
    uint64_t game_id;
    name player;
    asset amount;
    uint64_t roll_under;
    uint64_t random_roll;
    asset payout;
    asset ref_payout;
    string player_seed;
    checksum256 house_seed_hash;
    signature sig;
    uint64_t created_at;
    name referrer;
};

struct [[ eosio::table, eosio::contract("dicegame") ]] bets
{
    uint64_t id;
    uint64_t game_id;
    name player;
    uint64_t roll_under;
    asset amount;
    string player_seed;
    checksum256 house_seed_hash;
    uint64_t created_at;
    name referrer;
    uint64_t primary_key() const { return id; }

    EOSLIB_SERIALIZE(bets, (id)(game_id)(player)(roll_under)(amount)(player_seed)(house_seed_hash)(created_at)(referrer))
};

struct [[ eosio::table("envs"), eosio::contract("dicegame") ]] environments
{
    public_key pub_key;
    double casino_fee;
    double ref_bonus;
    double player_bonus;
    asset locked;
    uint64_t next_id;

    EOSLIB_SERIALIZE(environments, (pub_key)(casino_fee)(ref_bonus)(player_bonus)(locked)(next_id))
};

struct [[ eosio::table, eosio::contract("dicegame") ]] logs
{
    uint64_t game_id;
    asset amount;
    asset payout;
    uint64_t random_roll;
    signature sig;
    asset ref_payout;
    uint64_t created_at;

    uint64_t primary_key() const { return game_id; }

    EOSLIB_SERIALIZE(logs, (game_id)(amount)(payout)(random_roll)(sig)(ref_payout)(created_at))
};

typedef multi_index<"bets"_n, bets> _tbet;
typedef multi_index<"results"_n, logs> _tlogs;
typedef eosio::singleton<"envs"_n, environments> _tenvironments;
