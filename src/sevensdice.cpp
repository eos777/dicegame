#include "sevensdice.hpp"

void dice::launch(public_key pub_key, uint8_t casino_fee = 2, uint8_t ref_bonus = 0.5, uint8_t player_bonus = 0.5) {
    require_auth(N(eosbetcasino));

    auto environmnet = tenvironments.begin();
    eosio_assert(environmnet == tenvironments.end(), "Contract already launch");

    environments.emplace(_self, [&](auto& g){
        g.pub_key = pub_key;
        g.casino_fee = casino_fee;
        g.ref_bonus = ref_bonus;
        g.player_bonus = player_bonus;
        g.guaranty = asset(10, EOS_SYMBOL);
    });
}

void dice::resolvebet(const uint64_t& bet_id, const signature& sig) {
    require_auth(CASINOSEVENS);

    auto current_bet = tbets.find( bet_id );
    eosio_assert(current_bet != tbets.end(), "Bet doesn't exist");

    public_key key = str_to_pub(PUB_KEY, false);
    assert_recover_key(&bet.seed, (char*)&sig.data, sizeof(sig.data), (char*)&key.data, sizeof(key.data));

    checksum256 sig_hash;
    sha256((char*)&sig, sizeof(sig), &sig_hash);

    const uint64_t random_roll = get_random_roll(sig_hash)
    uint8_t fee = CASINO_FEE;
    uint8_t ref_bonus = 0;
    asset payout = asset(0, EOS_SYMBOL);

    if (bet.referrer != CASINOSEVENS) {
        fee -= PLAYER_BONUS;
        ref_bonus = current_bet.amount * REF_BONUS / 100;
    }

    if (random_roll < current_bet.roll_under) {
        payout = calc_payout(current_bet, current_bet.roll_under, fee);
    }

}

template<typename T>
void dice::apply_transfer(T data) {
    uint64_t roll_under;
    account_name referrer;
    string player_seed;

    parse_game_params(data.memo, &roll_under, &referrer, &player_seed);

    check_quantity(data.quantity);

    uint8_t fee = CASINO_FEE - REF_BONUS;
    if (referrer.empty() || referrer == data.from || !is_account(referrer) {
        referrer = CASINOSEVENS;
        fee = CASINO_FEE;
    }

    check_roll_under(roll_under, data.quantity, fee);

    tbets.emplace(_self, [&](bets& bet) {
        bet.id = tbets.available_primary_key();
        bet.player = data.from;
        bet.roll_under = roll_under;
        bet.amount = data.quantity;
        bet.player_seed = player_seed;
        bet.referrer = referrer;
        bet.created_at = now();
    });

}
