#include <iostream>
#include <eosiolib/crypto.h>
#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/types.hpp>
#include <eosiolib/transaction.hpp>
#include <eosio.token/eosio.token.hpp>

using namespace eosio;
using std::string;
using std::to_string;
using std::count;

size_t str_to_sep(const string& str,
                  string* part,
                  const char& sep,
                  const size_t& pos) {
    size_t tmp = str.find(sep, pos);
    eosio_assert(tmp != std::string::npos, "parse error");
    *part = str.substr(pos, tmp - pos);
    return tmp;
}

string to_hex(const char* d, uint32_t s) {
    std::string r;
    const char* to_hex = "0123456789abcdef";
    uint8_t* c = (uint8_t*)d;
    for (uint32_t i = 0; i < s; ++i)
        (r += to_hex[(c[i] >> 4)]) += to_hex[(c[i] & 0x0f)];
    return r;
}
