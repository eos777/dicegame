#include "types.hpp"

class dice : public contract {
public:
    using contract::contract;

    dice(account_name self) : contract(self), tbets(_self, _self), tenvironments(_self, _self){};//, cashbox(_self, _self){};

    /// @abi action
    void launch(public_key pub_key, uint8_t casino_fee = 2, uint8_t ref_bonus = 0.5, uint8_t player_bonus = 0.5);

    /// @abi action
    void reveal(const uint64_t& bet_id, const string& seed);

    template<typename T>
    void apply_transfer(T data);
private:
    _tbet tbets;
    _tenvironments tenvironments;

    uint8_t get_random_roll(checksum256 hash) {
        uint64_t mix_hash = hash.hash[0] + hash.hash[2] * hash.hash[4] + hash.hash[6] * hash.hash[8] + hash.hash[10] * hash.hash[12] + hash.hash[14] * hash.hash[16]
        return (mix_hash % 100) + 1;
    }

    void check_roll_under(uint64_t& roll_under, asset& quantity, uint8_t& fee) {
        eosio_assert(roll_under >= 2 && roll_under <= 96, "Rollunder must be >= 2, <= 96.");
        asset player_win_amount = calc_payout(quantity, roll_under, fee) - quantity.amount;
        eosio_assert(player_win_amount <= max_win(), "Available payout overflow");
    }

    void check_quantity(asset quantity) {
        eosio_assert(quantity.is_valid(), "Invalid token");
        eosio_assert(quantity.symbol == EOS_SYMBOL, "EOS token required");
        eosio_assert(quantity.amount >= MINBET, "Minimum transfer quantity is 0.1");
    }

    asset calc_payout(const asset& quantity, uint64_t roll_under, const uint8_t& fee) {
        const double rate = (100 - fee) / ((double)roll_under - 1.0);
        return asset(rate * quantity.amount, quantity.symbol);
    }

    asset max_win() {
        return available_balance() / 100;
    }

    asset available_balance() {
        auto token_contract = eosio::token(N(eosio.token));
        const asset balance = token.get_balance(_self, symbol_type(EOS_SYMBOL).name());
        auto stenvironments = environments.get();
        const asset available = balance - stenvironments.guaranty;
        eosio_assert(available.amount >= 0, "Guaranty overdraw")
        return available;
    }

    void check_signature(const uint64_t& roll_under,
                         const checksum256& house_seed_hash,
                         const uint64_t& expiration,
                         const account_name& referrer,
			 const signature& sig) {
        string data = to_string(roll_under);
        data += '-';
        data += to_hex((char *) house_seed_hash.hash, sizeof(house_seed_hash.hash));
        data += '-';
        data += to_string(expiration);
        data += '-';
        data += name{referrer}.to_string();

        checksum256 digest;
        sha256(data.c_str(), strlen(data.c_str()), &digest);
        public_key key = str_to_pub(PUB_KEY, false);
        assert_recover_key(&digest, (char *) &sig.data, sizeof(sig.data), (char *) &key.data, sizeof(key.data));
    }

    void parse_game_params(const string data,
                    uint64_t* roll_under,
                    account_name* referrer,
                    string* player_seed) {
        size_t sep_count = count(data.begin(), data.end(), '-');
        eosio_assert(sep_count == 2, "Invalid memo");

        size_t pos;
        string part;
        pos = str_to_sep(data, &part, '-', 0);
        eosio_assert(!part.empty(), "No rollunder");
        *roll_under = stoi(part);
        pos = str_to_sep(data, &part, '-', ++pos);
        eosio_assert(!part.empty(), "No referrer");
        *referrer = eosio::string_to_name(part.c_str());
        part = data.substr(++pos);
        *player_seed = part;
    }

    tcashbox get_cashbox(asset amount) {
        tcashbox new_cashbox{.locked = asset(10, EOS_SYMBOL)};
        return cashbox.get_or_create(_self, new_cashbox);
    }

};

// apply - это обработчик действий, он прослушивает все входящие действия и реагирует в соответствии со спецификациями внутри функции
// EOSIO_ABI инкапсулирует логику метода apply
extern "C" {
void apply(uint64_t receiver, uint64_t code, uint64_t action) {
    dice thiscontract(receiver);

    if ((code == N(eosio.token)) && (action == N(transfer))) {
        thiscontract.apply_transfer(unpack_action_data<eosio::token::transfer_args>());
    }

    if (code != receiver) return;

    switch(action) { EOSIO_API(dice, (resolvebet)) }
    eosio_exit(0);
}
}
