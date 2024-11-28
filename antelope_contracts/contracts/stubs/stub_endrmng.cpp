#include <variant>
#include <eosio/asset.hpp>
#include <eosio/name.hpp>
#include <eosio/eosio.hpp>
#include <eosio/singleton.hpp>

using namespace eosio;

namespace stub {

inline std::string vec_to_hex(const checksum256& byte_array, bool with_prefix) {
    static const char* kHexDigits{"0123456789abcdef"};
    auto a = byte_array.extract_as_byte_array();
    std::string out(a.size() * 2 + (with_prefix ? 2 : 0), '\0');
    char* dest{&out[0]};
    if (with_prefix) {
        *dest++ = '0';
        *dest++ = 'x';
    }
    for (const auto& b : a) {
        *dest++ = kHexDigits[(uint8_t)b >> 4];    // Hi
        *dest++ = kHexDigits[(uint8_t)b & 0x0f];  // Lo
    }
    return out;
}

class [[eosio::contract]] stub_endrmng : public contract {
    using contract::contract;

    struct [[eosio::table("config")]] config_t {
        checksum160 proxy;
        checksum160 staker;
        name validator;
        uint64_t stake;
        bool test_xsat;

        EOSLIB_SERIALIZE(config_t, (proxy)(staker)(validator)(stake)(test_xsat));
    };
    typedef eosio::singleton<"config"_n, config_t> config_singleton_t;
    

    config_t get_config() const {
        config_singleton_t config(get_self(), get_self().value);
        eosio::check(config.exists(), "evmutil config not exist");
        return config.get();
    }
    
    void set_config(const config_t &v) {
        config_singleton_t config(get_self(), get_self().value);
        config.set(v, get_self());
    }

    public:
    /*
    * Evm stake action.
    * @auth scope is `evmcaller` whitelist account
    *
    * @param caller - the account that calls the method
    * @param proxy - proxy address
    * @param staker - staker address
    * @param validator - validator address
    * @param quantity - total number of stake
    *
    */
    [[eosio::action]]
    void evmstake(const name& caller, const checksum160& proxy, const checksum160& staker, const name& validator,
                const asset& quantity);
    

    /*
    * Evm unstake action.
    * @auth scope is evmcaller whitelist account
    *
    * @param caller - the account that calls the method
    * @param proxy - proxy address
    * @param staker - staker address
    * @param validator - validator address
    * @param quantity - cancel pledge quantity
    *
    */
    [[eosio::action]]
    void evmunstake(const name& caller, const checksum160& proxy, const checksum160& staker, const name& validator,
                    const asset& quantity);

    /*
    * Evm change stake action.
    * @auth scope is `evmcaller` whitelist account
    *
    * @param caller - the account that calls the method
    * @param proxy - proxy address
    * @param staker - staker address
    * @param old_validator - old validator address
    * @param new_validator - new validator address
    * @param quantity - change the amount of pledge
    *
    */
    [[eosio::action]]
    void evmnewstake(const name& caller, const checksum160& proxy, const checksum160& staker, const name& old_validator,
                    const name& new_validator, const asset& quantity);

    /**
    * Evm stake action for XSAT.
    * @auth scope is `evmcaller` whitelist account
    *
    * @param caller - the account that calls the method
    * @param proxy - proxy address
    * @param staker - staker address
    * @param validator - validator address
    * @param quantity - total number of stake
    *
    */
    [[eosio::action]]
    void evmstakexsat(const name& caller, const checksum160& proxy, const checksum160& staker, const name& validator,
                const asset& quantity);
    

    /**
    * Evm unstake action for XSAT.
    * @auth scope is evmcaller whitelist account
    *
    * @param caller - the account that calls the method
    * @param proxy - proxy address
    * @param staker - staker address
    * @param validator - validator address
    * @param quantity - cancel pledge quantity
    *
    */
    [[eosio::action]]
    void evmunstkxsat(const name& caller, const checksum160& proxy, const checksum160& staker, const name& validator,
                    const asset& quantity);

    /**
    * Evm change stake action for XSAT.
    * @auth scope is `evmcaller` whitelist account
    *
    * @param caller - the account that calls the method
    * @param proxy - proxy address
    * @param staker - staker address
    * @param old_validator - old validator address
    * @param new_validator - new validator address
    * @param quantity - change the amount of pledge
    *
    */
    [[eosio::action]]
    void evmrestkxsat(const name& caller, const checksum160& proxy, const checksum160& staker, const name& old_validator,
                    const name& new_validator, const asset& quantity);          

    /*
    * Evm claim reward action.
    * @auth scope is evmcaller whitelist account
    *
    * @param caller - the account that calls the method
    * @param proxy - proxy address
    * @param staker - staker address
    * @param validator - validator address
    *
    */
    [[eosio::action]]
    void evmclaim(const name& caller, const checksum160& proxy, const checksum160& staker, const name& validator);
    [[eosio::action]] void reset(const checksum160& proxy, const checksum160& staker, const name& validator, bool test_xsat);
    [[eosio::action]] void assertstake(uint64_t stake);
    [[eosio::action]] void assertval(const name& validator);  
};

void stub_endrmng::evmstake(const name& caller, const checksum160& proxy, const checksum160& staker, const name& validator, const asset& quantity) {
    config_t config = get_config();
    check(!config.test_xsat, "only non xsat should call into here" );
    check(proxy == config.proxy, "proxy not found");
    check(staker == config.staker, "staker not found");
    check(validator == config.validator, "validator not found");
    config.stake += quantity.amount;

    set_config(config);
    
}

void stub_endrmng::evmunstake(const name& caller, const checksum160& proxy, const checksum160& staker, const name& validator, const asset& quantity) {
    config_t config = get_config();
    check(!config.test_xsat, "only non xsat should call into here" );
    check(proxy == config.proxy, "proxy not found");
    check(staker == config.staker, "staker not found");
    check(validator == config.validator, "validator not found");
    check(config.stake >= quantity.amount, "no enough stake");
    config.stake -= quantity.amount;

    set_config(config);
}

void stub_endrmng::evmclaim(const name& caller, const checksum160& proxy, const checksum160& staker, const name& validator) {
    config_t config = get_config();
    check(!config.test_xsat, "only non xsat should call into here" );
    
    check(proxy == config.proxy, "proxy not found" );
    check(staker == config.staker, "staker not found");
    check(validator == config.validator, "validator not found");
    
    return;
}

void stub_endrmng::evmnewstake(const name& caller, const checksum160& proxy, const checksum160& staker, const name& validator, const name& new_validator, const asset& quantity) {
    config_t config = get_config();
    check(!config.test_xsat, "only non xsat should call into here" );
    
    check(proxy == config.proxy, "proxy not found" );
    check(staker == config.staker, "staker not found");
    check(validator == config.validator, "validator not found");
    config.validator = new_validator;
    set_config(config);
    return;
}

void stub_endrmng::evmstakexsat(const name& caller, const checksum160& proxy, const checksum160& staker, const name& validator, const asset& quantity) {
    config_t config = get_config();
    check(config.test_xsat, "only xsat should call into here" );

    check(quantity.symbol == symbol("XSAT", 8u), "xsat symbol not correct");

    check(proxy == config.proxy, "proxy not found");
    check(staker == config.staker, "staker not found");
    check(validator == config.validator, "validator not found");
    config.stake += quantity.amount;

    set_config(config);
    
}

void stub_endrmng::evmunstkxsat(const name& caller, const checksum160& proxy, const checksum160& staker, const name& validator, const asset& quantity) {
    config_t config = get_config();
    check(config.test_xsat, "only xsat should call into here" );

    check(quantity.symbol == symbol("XSAT", 8u), "xsat symbol not correct");
    
    check(proxy == config.proxy, "proxy not found");
    check(staker == config.staker, "staker not found");
    check(validator == config.validator, "validator not found");
    check(config.stake >= quantity.amount, "no enough stake");
    config.stake -= quantity.amount;

    set_config(config);
}

void stub_endrmng::evmrestkxsat(const name& caller, const checksum160& proxy, const checksum160& staker, const name& validator, const name& new_validator, const asset& quantity) {
    
    config_t config = get_config();

    check(config.test_xsat, "only xsat should call into here" );

    check(quantity.symbol == symbol("XSAT", 8u), "xsat symbol not correct");
    
    check(proxy == config.proxy, "proxy not found" );
    check(staker == config.staker, "staker not found");
    check(validator == config.validator, "validator not found");
    config.validator = new_validator;
    set_config(config);
    return;
}

void stub_endrmng::reset(const checksum160& proxy, const checksum160& staker, const name& validator, bool test_xsat) {
 

    config_t config;

    config.proxy = proxy;
    config.staker = staker;
    config.validator = validator;
    config.stake = 0;
    config.test_xsat = test_xsat;

    set_config(config);
}

void stub_endrmng::assertstake(uint64_t stake) {
    config_t config = get_config();
    check(stake == config.stake, "stake not correct" );
}

void stub_endrmng::assertval(const name& validator) {
    config_t config = get_config();
    check(validator == config.validator, "validator not correct" );
}


}  // namespace stub

