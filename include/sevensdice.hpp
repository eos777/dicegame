#include "types.hpp"

class [[eosio::contract]] sevensdice : public contract {
public:
    using contract::contract;
    sevensdice(name receiver, name code, datastream<const char*> ds) : contract(receiver, code, ds), tbets(_self, _self.value), tlogs(_self, _self.value), tenvironments(_self, _self.value){};

    [[eosio::action]]
    void launch(public_key pub_key, uint8_t casino_fee, double ref_bonus, double player_bonus);

    [[eosio::action]]
    void resolvebet(const uint64_t& bet_id, const signature& sig);

    void apply_transfer(name from, name to, asset quantity, string memo);

    [[eosio::action]]
    void receipt(const results& result);

    [[eosio::action]]
    void cleanlog(uint64_t game_id);
private:
    _tbet tbets;
    _tlogs tlogs;
    _tenvironments tenvironments;

    uint8_t get_random_roll(checksum256 hash) {
        auto byte_array = hash.extract_as_byte_array();
        uint64_t mix_hash = byte_array[0] + byte_array[2] * byte_array[4] + byte_array[6] * byte_array[8] + byte_array[10] * byte_array[12] + byte_array[14] * byte_array[16];
        return (mix_hash % 100) + 1;
    }

    void check_game_id(uint64_t& game_id) {
        auto iter = tbets.begin();
        while (iter != tbets.end()) {
            eosio_assert(iter->game_id != game_id, "GameID collision detected");
            iter++;
        }
        auto exist = tlogs.find(game_id);
        eosio_assert(exist == tlogs.end(), "GameID collision detected");
    }

    void check_roll_under(uint64_t& roll_under) {
        eosio_assert(roll_under >= 2 && roll_under <= 96, "Rollunder must be >= 2, <= 96.");
    }

    void check_quantity(asset quantity) {
        eosio_assert(quantity.is_valid(), "Invalid token");
        eosio_assert(quantity.symbol == EOS_SYMBOL, "EOS token required");
        eosio_assert(quantity.amount >= MINBET, "Minimum transfer quantity is 0.1");
    }

    asset calc_payout(const asset& quantity, const uint64_t& roll_under, const double& fee) {
        const double rate = (100.0 - fee) / (roll_under - 1);
        uint64_t payout = rate * (quantity.amount / 10000) * 10000;
        return asset(payout, quantity.symbol);
    }

    asset max_win() {
        return available_balance() / 10;
    }

    asset available_balance() {
        //auto token_contract = eosio::token("eosio.token"_n, "sevenscasino"_n, _ds);
        const asset balance = eosio::token::get_balance(name("eosio.token"), _self, EOS_SYMBOL.code());
        auto stenvironments = tenvironments.get(0, "Environment is not set");
        const asset available = balance - stenvironments.locked;
        eosio_assert(available.amount >= 0, "Liabilities pool overdraw");
        return available;
    }

    void unlock(const asset& amount) {
        auto stenvironments = tenvironments.get(0, "Environment is not set");
        asset locked = stenvironments.locked - amount;
        eosio_assert(locked.amount >= 0, "Fund lock error");
        tenvironments.modify(tenvironments.begin(), _self, [&](environments& e) {
            e.locked -= amount;
        });
    }

    void lock(const asset& amount) {
        asset balance = available_balance() - amount;
        eosio_assert(balance.amount >= 0, "Fund lock error");
        tenvironments.modify(tenvironments.begin(), _self, [&](environments& e) {
            e.locked += amount;
        });
    }

    uint64_t available_bet_id() {
        auto stenvironments = tenvironments.get(0, "Environment is not set");
        uint64_t bet_id = stenvironments.next_id;
        tenvironments.modify(tenvironments.begin(), _self, [&](environments& e) {
            e.next_id += 1;
        });
        return bet_id;
    }

    string winner_msg(const bets& bet) {
        string msg = "Bet id: ";
        string id = to_string(bet.id);
        msg += id;
        msg += " Player: ";
        string player = name{bet.player}.to_string();
        msg += player;
        msg += "  *** Winner *** Play: eos777.io/dice";
        return msg;
    }

    string ref_msg(const bets& bet) {
        string msg = "Referrer: ";
        string referrer = name{bet.referrer}.to_string();
        msg += referrer;
        msg += " Bet id: ";
        string id = to_string(bet.id);
        msg += id;
        msg += " Player: ";
        string player = name{bet.player}.to_string();
        msg += player;
        msg += "  *** Referral reward *** Play: eos777.io/dice";
        return msg;
    }

    void parse_game_params(const string data,
                    uint64_t* roll_under,
                    name* referrer,
                    string* player_seed,
                    uint64_t* game_id) {
        size_t sep_count = count(data.begin(), data.end(), '-');
        eosio_assert(sep_count == 3, "Invalid memo");

        size_t pos;
        string part;
        pos = str_to_sep(data, &part, '-', 0);
        eosio_assert(!part.empty(), "No rollunder");
        *roll_under = stoi(part);
        pos = str_to_sep(data, &part, '-', ++pos);
        *referrer = name(part);
        pos = str_to_sep(data, &part, '-', ++pos);
        *player_seed = part;
        part = data.substr(++pos);
        eosio_assert(!part.empty(), "No gameID");
        *game_id = stoi(part);
    }
};

extern "C" {
void apply(uint64_t receiver, uint64_t code, uint64_t action) {
    if (code == receiver && action == name("launch").value) {
        execute_action(name(receiver), name(code), &sevensdice::launch);
    }
    if (code == receiver && action == name("resolvebet").value) {
        execute_action(name(receiver), name(code), &sevensdice::resolvebet);
    }
    if (code == receiver && action == name("cleanlog").value) {
        execute_action(name(receiver), name(code), &sevensdice::cleanlog);
    }
    if (code == receiver && action == name("receipt").value) {
        execute_action(name(receiver), name(code), &sevensdice::receipt);
    }
    if (code == name("eosio.token").value && action == name("transfer").value) {
        execute_action(name(receiver), name(code), &sevensdice::apply_transfer);
    }
}
}