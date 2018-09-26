#include "types.hpp"

class dice : public contract {
public:
    using contract::contract;
    dice(account_name self) : contract(self), tbets(_self, _self){};//, cashbox(_self, _self){};

    /// @abi action
    void reveal(const uint64_t& bet_id, const string& seed);

    template<typename T>
    void apply_transfer(T data);
private:
    _tbet tbets;
    //_tcashbox cashbox;

    uint8_t generate_random_roll(const string seed1, const string seed2) {
        return hash<string>{}(seed1 + seed2) % 100 + 1;
    }

    void check_roll_under(uint64_t number) {
        eosio_assert(number >= 2 && number <= 96, "incorrect roll under");
        //check max_bonus
    }

    void check_quantity(asset quantity) {
        eosio_assert(quantity.symbol == EOS_SYMBOL, "EOS token required");
        eosio_assert(quantity.is_valid(), "invalid token");
        eosio_assert(quantity.amount >= 1000, "minimum transfer quantity is 0.1");
    }

    void check_hash(const checksum256& hash, const uint64_t expiration) {

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

    void parse_memo(const string memo,
                    uint64_t* roll_under,
                    string* player_seed,
                    checksum256* house_seed_hash,
                    uint64_t* expiration,
                    account_name* referrer,
                    signature* sig) {
        size_t pos;
        string part;
        pos = str_to_sep(memo, &part, '-', 0);
        eosio_assert(!part.empty(), "no roll under");
        *roll_under = stoi(part);
        pos = str_to_sep(memo, &part, '-', ++pos);
        eosio_assert(!part.empty(), "no player_seed_hash under");
        *player_seed = part;
        pos = str_to_sep(memo, &part, '-', ++pos);
        eosio_assert(!part.empty(), "no house seed hash under");
        *house_seed_hash = hex2sha256(part);//hex2sha256(part);
        pos = str_to_sep(memo, &part, '-', ++pos);
        eosio_assert(!part.empty(), "no expiration");
        *expiration = stoi(part);
        pos = str_to_sep(memo, &part, '-', ++pos);
        eosio_assert(!part.empty(), "no referrer");
        *referrer = eosio::string_to_name(part.c_str());
        part = memo.substr(++pos);
        eosio_assert(!part.empty(), "no signature");
        *sig = str_to_sig(part);
    }

    /*
    tcashbox get_cashbox(asset amount) {
        tcashbox new_cashbox{.locked = asset(10, EOS_SYMBOL)};
        return cashbox.get_or_create(_self, new_cashbox);
    }
    */
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

    switch(action) { EOSIO_API(dice, (reveal)) }
    eosio_exit(0);
}
}