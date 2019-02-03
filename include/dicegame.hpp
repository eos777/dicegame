#include "types.hpp"

class[[eosio::contract]] dicegame : public contract
{
  public:
    using contract::contract;
    dicegame(name receiver, name code, datastream<const char *> ds) : contract(receiver, code, ds), tbets(_self, _self.value), tlogs(_self, _self.value), tenvironments(_self, _self.value){};

    [[eosio::action]] void launch(public_key pub_key, double casino_fee, double ref_bonus, double player_bonus);

    [[eosio::action]] void resolvebet(const uint64_t &bet_id, const signature &sig);

    void apply_transfer(name from, name to, asset quantity, string memo);

    void reftransfer(name to, asset quantity, string memo);

    [[eosio::action]] void receipt(const results &result);

    [[eosio::action]] void cleanlog(uint64_t game_id);

    [[eosio::action]] void deletedata();

  private:
    _tbet tbets;
    _tlogs tlogs;
    _tenvironments tenvironments;

    uint8_t get_random_roll(checksum256 hash)
    {
        auto byte_array = hash.extract_as_byte_array();
        uint64_t mix_hash = byte_array[0] + byte_array[2] * byte_array[4] + byte_array[6] * byte_array[8] + byte_array[10] * byte_array[12] + byte_array[14] * byte_array[16];
        return (mix_hash % 100) + 1;
    }

    void check_game_id(uint64_t & game_id)
    {
        auto iter = tbets.begin();
        while (iter != tbets.end())
        {
            eosio_assert(iter->game_id != game_id, "GameID collision detected");
            iter++;
        }
        auto exist = tlogs.find(game_id);
        eosio_assert(exist == tlogs.end(), "GameID collision detected");
    }

    void check_roll_under(uint64_t & roll_under)
    {
        eosio_assert(roll_under >= 2 && roll_under <= 96, "Rollunder must be >= 2, <= 96.");
    }

    void check_quantity(asset quantity)
    {
        eosio_assert(quantity.is_valid(), "Invalid token");
        eosio_assert(quantity.symbol == EOS_SYMBOL, "EOS token required");
        eosio_assert(quantity.amount >= MINBET, "Minimum transfer quantity is 0.1");
    }

    asset calc_payout(const asset &quantity, const uint64_t &roll_under, const double &fee)
    {
        const uint64_t rate = (100.0 - fee) / (roll_under - 1) * 10000;
        uint64_t payout = (rate * quantity.amount) / 10000;
        return asset(payout, quantity.symbol);
    }

    asset max_win()
    {
        return available_balance() / 5;
    }

    asset available_balance()
    {
        const asset balance = eosio::token::get_balance(name("eosio.token"), _self, EOS_SYMBOL.code());
        auto stenvironments = tenvironments.get();
        const asset available = balance - stenvironments.locked;
        eosio_assert(available.amount >= 0, "Liabilities pool overdraw");
        return available;
    }

    void unlock(const asset &amount)
    {
        auto stenvironments = tenvironments.get();
        eosio_assert(stenvironments.locked >= amount, "Fund unlock error");
        stenvironments.locked -= amount;
        tenvironments.set(stenvironments, _self);
    }

    void lock(const asset &amount)
    {
        asset balance = available_balance() - amount;
        eosio_assert(balance.amount >= 0, "Fund lock error");
        auto stenvironments = tenvironments.get();
        stenvironments.locked += amount;
        tenvironments.set(stenvironments, _self);
    }

    void airdrop_tokens(const uint64_t bet_id, const asset quantity, const name player)
    {
        uint64_t drop_amount = quantity.amount / 30;
        asset token_balance = eosio::token::get_balance(name("sevenstokens"), _self, SVNS_SYMBOL.code());

        if (token_balance.amount == 0)
        {
            return;
        }
        if (drop_amount > token_balance.amount)
        {
            drop_amount = token_balance.amount;
        }

        action(
            permission_level{_self, name("active")},
            name("sevenstokens"),
            name("transfer"),
            std::make_tuple(
                _self,
                player,
                asset(drop_amount, token_balance.symbol),
                airdrop_msg(bet_id)))
            .send();
    }

    uint64_t available_bet_id()
    {
        auto stenvironments = tenvironments.get();
        uint64_t bet_id = stenvironments.next_id;
        stenvironments.next_id += 1;
        tenvironments.set(stenvironments, _self);
        return bet_id;
    }

    string winner_msg(const bets &bet)
    {
        string msg = "Bet id: ";
        string id = to_string(bet.id);
        msg += id;
        msg += " Player: ";
        string player = name{bet.player}.to_string();
        msg += player;
        msg += "  *** Winner *** Play: eos777.io/dice";
        return msg;
    }

    string ref_msg(const bets &bet)
    {
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

    string airdrop_msg(const uint64_t bet_id)
    {
        string msg = "Bet id: ";
        string id = to_string(bet_id);
        msg += id;
        msg += " *** Enjoy airdrop! *** Play: eos777.io/dice";
        return msg;
    }

    void parse_game_params(const string data,
                           uint64_t *roll_under,
                           name *referrer,
                           string *player_seed,
                           uint64_t *game_id)
    {
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
        *game_id = stoll(part);
    }
};

#define EOSIO_DISPATCH_CUSTOM(TYPE, MEMBERS)
extern "C"
{
    void apply(uint64_t receiver, uint64_t code, uint64_t action)
    {
        if (code == receiver && action == name("launch").value)
        {
            execute_action(name(receiver), name(code), &dicegame::launch);
        }
        if (code == receiver && action == name("resolvebet").value)
        {
            execute_action(name(receiver), name(code), &dicegame::resolvebet);
        }
        if (code == receiver && action == name("reftransfer").value)
        {
            execute_action(name(receiver), name(code), &dicegame::reftransfer);
        }
        if (code == receiver && action == name("cleanlog").value)
        {
            execute_action(name(receiver), name(code), &dicegame::cleanlog);
        }
        if (code == receiver && action == name("deletedata").value)
        {
            execute_action(name(receiver), name(code), &dicegame::deletedata);
        }
        if (code == receiver && action == name("receipt").value)
        {
            execute_action(name(receiver), name(code), &dicegame::receipt);
        }
        if (code == name("eosio.token").value && action == name("transfer").value)
        {
            execute_action(name(receiver), name(code), &dicegame::apply_transfer);
        }
    }
}
