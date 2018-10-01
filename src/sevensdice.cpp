#include "sevensdice.hpp"

void sevensdice::launch(public_key pub_key, uint8_t casino_fee = 2, double ref_bonus = 0.5, double player_bonus = 0.5) {
    require_auth(CASINOSEVENS);

    auto environmnet = tenvironments.begin();
    eosio_assert(environmnet == tenvironments.end(), "Contract already launch");

    environments.emplace(_self, [&](auto& g){
        g.pub_key = pub_key;
        g.casino_fee = casino_fee;
        g.ref_bonus = ref_bonus;
        g.player_bonus = player_bonus;
        g.locked = asset(0, EOS_SYMBOL);
    });
}

void sevensdice::resolvebet(const uint64_t& bet_id, const signature& sig) {
    require_auth(CASINOSEVENS);

    auto current_bet = tbets.find( bet_id );
    eosio_assert(current_bet != tbets.end(), "Bet doesn't exist");

    auto stenvironments = tenvironments.get();
    public_key key = stenvironments.pub_key;
    //public_key key = str_to_pub(pub_key, false);
    assert_recover_key(&bet.seed, (char*)&sig.data, sizeof(sig.data), (char*)&key.data, sizeof(key.data));

    checksum256 sig_hash;
    sha256((char*)&sig, sizeof(sig), &sig_hash);

    const uint64_t random_roll = get_random_roll(sig_hash)
    uint8_t fee = CASINO_FEE;
    asset ref_bonus = asset(0, EOS_SYMBOL);
    asset payout = asset(0, EOS_SYMBOL);

    if (bet.referrer != CASINOSEVENS) {
        fee -= PLAYER_BONUS;
        ref_bonus.amount = current_bet.amount * REF_BONUS / 100;
    }

    if (random_roll < current_bet.roll_under) {
        payout = calc_payout(current_bet, current_bet.roll_under, fee);
        action(
                permission_level{_self, N(active)},
                N(eosio.token),
                N(transfer),
                std::make_tuple(
                        _self,
                        current_bet.player,
                        asset(payout, EOS_SYMBOL)),
                        winner_msg(current_bet)
                )
        ).send();
    }

    unlock(current_bet.amount);

    const results result{.id = next_id(),
                .player = from,
                .amount = quantity,
                .roll_under = roll_under,
                .random_roll = random_roll,
                .payout = payout,
                .player_seed = current_bet.player_seed,
                .house_seed_hash = current_bet.house_seed_hash,
                .sig = sig,
                .referrer = referrer};

    action(permission_level{_self, N(active)},
           _self,
           N(receipt),
           result).send();

    transaction ref_trx{};
    out.actions.emplace_back(permission_level{_self, N(active)},
                N(eosio.token),
                N(transfer),
                std::make_tuple(_self,
                        current_bet.referrer,
                        ref_bonus,
                        ref_msg(current_bet)));
    ref_trx.delay_sec = 5;
    ref_trx.send(current_bet.id, _self);

    tbets.erase(current_bet);

}

template<typename T>
void sevensdice::apply_transfer(T data) {
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

    lock(data.quantity);

    checksum256 player_seed_hash;
    sha256(const_cast<char*>(player_seed.c_str()), player_seed.size() * sizeof(char), &player_seed_hash);

    //test
    auto size = transaction_size();
    char buf[size];
    read_transaction(buf, size);
    checksum256 trx_hash;
    sha256(buf, size, &trx_hash);

    mixed_hash = to_hex((char*)player_seed_hash.hash, sizeof(player_seed_hash.hash)) + to_hex((char*)trx_hash.hash, sizeof(trx_hash.hash));

    checksum256 house_seed_hash;
    sha256((char*)mixed_hash, sizeof(mixed_hash), &house_seed_hash);

    tbets.emplace(_self, [&](bets& bet) {
        bet.id = tbets.available_primary_key();
        bet.player = data.from;
        bet.roll_under = roll_under;
        bet.amount = data.quantity;
        bet.player_seed = player_seed;
        bet.house_seed_hash = seed_hash;
        bet.referrer = referrer;
        bet.created_at = now();
    });

}

void sevensdice::receipt(const results& result) {
    require_auth(CASINOSEVENS);
}
