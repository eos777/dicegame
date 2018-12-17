#include "../include/dicegame.hpp"

void dicegame::launch(public_key pub_key, double casino_fee, double ref_bonus, double player_bonus)
{
    require_auth(CASINOSEVENS);

    auto stenv = tenvironments.get_or_default(environments{.locked = asset(0, EOS_SYMBOL), .next_id = 0});
    stenv.pub_key = pub_key;
    stenv.casino_fee = casino_fee;
    stenv.ref_bonus = ref_bonus;
    stenv.player_bonus = player_bonus;
    tenvironments.set(stenv, _self);
}

void dicegame::resolvebet(const uint64_t &bet_id, const signature &sig)
{
    require_auth(CASINOSEVENS);

    auto current_bet = tbets.find(bet_id);
    eosio_assert(current_bet != tbets.end(), "Bet doesn't exist");

    auto stenvironments = tenvironments.get();
    public_key key = stenvironments.pub_key;
    assert_recover_key(current_bet->house_seed_hash, sig, key);

    checksum256 sig_hash = sha256((char *)&sig, sizeof(sig));

    const uint64_t random_roll = get_random_roll(sig_hash);
    double fee = (double)stenvironments.casino_fee;
    asset ref_bonus = asset(0, EOS_SYMBOL);
    asset payout = asset(0, EOS_SYMBOL);
    asset possible_payout = calc_payout(current_bet->amount, current_bet->roll_under, fee);

    if (current_bet->referrer != CASINOSEVENS)
    {
        fee -= stenvironments.player_bonus;
        ref_bonus.amount = current_bet->amount.amount * stenvironments.ref_bonus / 100;
    }

    if (random_roll < current_bet->roll_under)
    {
        payout = possible_payout;
        action(
            permission_level{_self, name("active")},
            name("eosio.token"),
            name("transfer"),
            std::make_tuple(
                _self,
                current_bet->player,
                payout,
                winner_msg(*current_bet)))
            .send();
    }

    unlock(possible_payout);

    const results result{.id = current_bet->id,
                         .game_id = current_bet->game_id,
                         .player = current_bet->player,
                         .amount = current_bet->amount,
                         .roll_under = current_bet->roll_under,
                         .random_roll = random_roll,
                         .payout = payout,
                         .player_seed = current_bet->player_seed,
                         .house_seed_hash = current_bet->house_seed_hash,
                         .sig = sig,
                         .referrer = current_bet->referrer};

    SEND_INLINE_ACTION(*this, receipt, {CASINOSEVENS, name("active")}, {result});

    if (ref_bonus.amount > 0)
    {
        transaction ref_trx{};
        ref_trx.actions.emplace_back(permission_level{_self, name("active")},
                                     name("eosio.token"),
                                     name("transfer"),
                                     std::make_tuple(_self,
                                                     current_bet->referrer,
                                                     ref_bonus,
                                                     ref_msg(*current_bet)));
        ref_trx.delay_sec = 5;
        ref_trx.send(current_bet->id, _self);
    }

    tbets.erase(current_bet);

    tlogs.emplace(_self, [&](logs &entry) {
        entry.game_id = result.game_id;
        entry.amount = result.amount;
        entry.payout = result.payout;
        entry.random_roll = result.random_roll;
        entry.created_at = now();
    });
}

void dicegame::apply_transfer(name from, name to, asset quantity, string memo)
{
    if (from == _self || to != _self)
    {
        return;
    }

    uint64_t roll_under;
    name referrer;
    string player_seed;
    uint64_t game_id;

    parse_game_params(memo, &roll_under, &referrer, &player_seed, &game_id);

    check_quantity(quantity);
    check_game_id(game_id);

    auto stenvironments = tenvironments.get();

    double fee = stenvironments.casino_fee - stenvironments.player_bonus;
    if (name(referrer) == CASINOSEVENS || referrer == from || !is_account(referrer))
    {
        referrer = CASINOSEVENS;
        fee = stenvironments.casino_fee;
    }

    check_roll_under(roll_under);

    asset player_possible_win = calc_payout(quantity, roll_under, fee);

    eosio_assert(player_possible_win <= max_win(), "Available fund overflow");

    lock(player_possible_win);

    checksum256 player_seed_hash = sha256(const_cast<char *>(player_seed.c_str()), player_seed.size() * sizeof(char));

    auto size = transaction_size();
    char buf[size];
    read_transaction(buf, size);
    checksum256 trx_hash = sha256(buf, size);

    auto arr1 = player_seed_hash.extract_as_byte_array();
    auto arr2 = trx_hash.extract_as_byte_array();

    string mixed_hash = to_hex((char *)arr1.data(), arr1.size()) + to_hex((char *)arr2.data(), arr2.size());
    checksum256 house_seed_hash = sha256(const_cast<char *>(mixed_hash.c_str()), mixed_hash.size() * sizeof(char));

    tbets.emplace(_self, [&](bets &bet) {
        bet.id = available_bet_id();
        bet.game_id = game_id;
        bet.player = from;
        bet.roll_under = roll_under;
        bet.amount = quantity;
        bet.player_seed = player_seed;
        bet.house_seed_hash = house_seed_hash;
        bet.referrer = referrer;
        bet.created_at = now();
    });
}

void dicegame::cleanlog(uint64_t game_id)
{
    require_auth(CASINOSEVENS);
    auto entry = tlogs.find(game_id);
    tlogs.erase(entry);
}

void dicegame::receipt(const results &result)
{
    require_auth(CASINOSEVENS);
}

void dicegame::deletedata()
{
    require_auth(CASINOSEVENS);

    auto itr = tbets.begin();
    while (itr != tbets.end())
    {
        itr = tbets.erase(itr);
    }

    auto itr2 = tlogs.begin();
    while (itr2 != tlogs.end())
    {
        itr2 = tlogs.erase(itr2);
    }

    tenvironments.remove();
}