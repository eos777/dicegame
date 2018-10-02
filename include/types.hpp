#include <eosiolib/singleton.hpp>
#include "utils.hpp"

#define MINBET 1000
#define EOS_SYMBOL S(4, EOS)
#define CASINOSEVENS N(sevensdice)

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
    double ref_bonus;
    double player_bonus;
    asset locked;
    uint64_t primary_key() const { return 0; }

    EOSLIB_SERIALIZE(environments, (pub_key)(casino_fee)(ref_bonus)(player_bonus)(locked))
};

struct results {
    uint64_t id;
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

typedef multi_index<N(bets), bets> _tbet;
typedef singleton<N(environments), environments> _tenvironments;
