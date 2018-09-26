#include "include/sevensdice.hpp"

void dice::reveal(const uint64_t& bet_id, const string& seed) {
    /*
    eosio::print("REVEAL");  
	require_auth(OWNER);

    assert_sha256(seed, seed.length(), bet.house_seed_hash);
    uint8_t random_roll = generate_random_roll(seed, bet.player_seed);
    asset payout = asset(0, EOS_SYMBOL);
    if (random_roll < bet.roll_under) {

    }
    */
}

template<typename T>
void dice::apply_transfer(T data) {
    uint64_t roll_under;
    string player_seed;
    checksum256 house_seed_hash;
    uint64_t expiration;
    account_name referrer;
    signature sig;

    parse_memo(data.memo, &roll_under, &player_seed, &house_seed_hash, &expiration, &referrer, &sig);

    check_quantity(data.quantity);
    check_roll_under(roll_under);
    check_hash(house_seed_hash, expiration);

    check_signature(roll_under, house_seed_hash, expiration, referrer, sig);

    tbets.emplace(_self, [&](bets& bet) {
        bet.id = tbets.available_primary_key();
        bet.player = data.from;
        bet.roll_under = roll_under;
        bet.amount = data.quantity;
        bet.player_seed = player_seed;
        bet.house_seed_hash = house_seed_hash;
        bet.created_at = now();
        bet.referrer = referrer;
    });

    eosio::print("OK");
}