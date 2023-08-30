#include <erc20/eosio.token.hpp>
#include <erc20/erc20.hpp>
#include <erc20/hex.hpp>

#include <erc20/bytecode.hpp>
#include <erc20/proxy_bytecode.hpp>

#include <silkworm/core/execution/address.hpp>
#include <silkworm/core/common/util.hpp>

namespace erc20 {

template <size_t Size>
void initialize_data(bytes& output, const unsigned char (&arr)[Size]) {
    output.resize(Size);
    std::memcpy(output.data(), arr, Size);
}

void erc20::init() {
    require_auth(get_self());

    bytes call_data;

    auto reserved_addr = silkworm::make_reserved_address(get_self().value);
    initialize_data(call_data, solidity::erc20::bytecode);
    bytes to = {};
    // Assume account opened in evm_runtime
    call_action call_act(evm_account, {{get_self(), "active"_n}});
    call_act.send(get_self(), to, 0, call_data, evm_init_gaslimit);

    // Assume nonce...
    auto deploy_addr = silkworm::create_address(reserved_addr, 0); 

    initialize_data(call_data, solidity::proxy::bytecode);
    call_data.insert(call_data.end(), 32 - kAddressLength, 0);  // padding for address
    call_data.insert(call_data.end(), deploy_addr.bytes, deploy_addr.bytes + kAddressLength);
    // Assume account opened in evm_runtime
    call_act.send(get_self(), to, 0, call_data, evm_init_gaslimit);

    // Assume nonce...
    deploy_addr = silkworm::create_address(reserved_addr, 1); 

    // TODO: Where can we get the addr...
    config new_config = {
        .erc20_addr = {},
    };

    new_config.erc20_addr.resize(kAddressLength);
    memcpy(new_config.erc20_addr.data(), deploy_addr.bytes, kAddressLength);

    _config.set(new_config, get_self());
}

void erc20::onbridgemsg(name receiver, const bytes& sender, const time_point& timestamp, const bytes& value, const bytes& data) {
    require_auth(evm_account);

    // TODO: this API will change

    check(data.size() == kAddressLength + 32 , "invalid data size");
    // TODO: verify from, decode data
    intx::uint256 amount = intx::be::unsafe::load<intx::uint256>((const uint8_t*)data.data() + kAddressLength);
    evmc::address dest = {};
    memcpy(dest.bytes, data.data(), kAddressLength);
    auto to = silkworm::extract_reserved_address(dest);

    check(!!to , "failed to extract destination address");

    eosio::check(amount % minimum_natively_representable == 0_u256, "transfer must not generate dust");

    eosio::token::transfer_action transfer_act(token_account, {{get_self(), "active"_n}});
    transfer_act.send(get_self(), *to, eosio::asset((uint64_t)(amount / minimum_natively_representable), token_symbol), std::string("Transfer from EVM"));
}

void erc20::transfer(eosio::name from, eosio::name to, eosio::asset quantity,
                     std::string memo) {
    eosio::check(get_first_receiver() == token_account && quantity.symbol == token_symbol,
                 "received unexpected token");

    if (to != get_self() || from == get_self()) return;

    if (memo.size() == 42 && memo[0] == '0' && memo[1] == 'x')
        handle_evm_transfer(quantity, memo);
    else
        eosio::check(false, "memo must be 0x EVM address");
    
}

void erc20::handle_evm_transfer(eosio::asset quantity, const std::string& memo) {
    const char method[4] = {'\xa9', '\x05', '\x9c', '\xbb'};  // sha3(transfer(address,uint256))[:4]

    auto address_bytes = from_hex(memo);
    eosio::check(!!address_bytes, "memo must be valid 0x EVM address");
    eosio::check(address_bytes->size() == kAddressLength, "memo must be valid 0x EVM address");

    intx::uint256 value((uint64_t)quantity.amount);
    value *= minimum_natively_representable;

    uint8_t value_buffer[32] = {};
    intx::be::store(value_buffer, value);

    bytes call_data;
    call_data.reserve(4 + 64);
    call_data.insert(call_data.end(), method, method + 4);
    call_data.insert(call_data.end(), 32 - kAddressLength, 0);  // padding for address
    call_data.insert(call_data.end(), address_bytes->begin(), address_bytes->end());
    call_data.insert(call_data.end(), value_buffer, value_buffer + 32);

    call_action call_act(evm_account, {{get_self(), "active"_n}});
    call_act.send(get_self(), *address_bytes, 0, call_data, evm_gaslimit);
}

}  // namespace erc20