#include <eosiolib/singleton.hpp>
#include "utils.hpp"

#define MINBET 1000

#define EOS_SYMBOL S(4, EOS)
#define CASINOSEVENS N(casinosevens)
#define PUB_KEY "EOS78h6D7hSh6QJkkEJnKixEkvVaeUcxZoJfremnJge7BnE2yJ42W"

// имя нашей таблицы - это bets, а используемый тип индекса -i64 
/// @abi table bets i64
struct bets {
    uint64_t id;
    account_name player;
    uint64_t roll_under;
    asset amount;
    string player_seed;
    checksum256 house_seed_hash;
    uint64_t created_at;
    account_name referrer;
    uint64_t primary_key() const { return id; }

    EOSLIB_SERIALIZE(bets, (id)(player)(roll_under)(amount)(player_seed)(house_seed_hash)(created_at)(referrer))
};

/// @abi table environments i64
struct environments {
    public_key pub_key;
    uint8_t casino_fee;
    uint8_t ref_bonus;
    uint8_t player_bonus;
    asset guaranty;
    uint64_t primary_key() const { return 0; }

    EOSLIB_SERIALIZE(environments, (pub_key)(casino_fee)(ref_bonus)(player_bonus)(guaranty))
};

// Multi_index is a form provided in a form that has easy-to-use data structures in the Boost Library.
// typedef multi_index<N(table_name), object_template_to_use> multi_index_name;
typedef multi_index<N(bets), bets> _tbet;
typedef singleton<N(environments), environments> _tenvironments;
//typedef singleton<N(cashbox), cashbox> _tcashbox;
