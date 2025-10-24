# Exsat EVM trustless bridge

**The contracts for staking and claiming rewards have been moved to https://github.com/exsat-network/evmutil-contracts.**
**This repository now only contains the code for the trustless bridge for ERC20 tokens.** 

This repository contains the Solidity and Antelope contracts needed to support the trustless bridges.

Those contracts (both within `solidity_contracts` and `antelope_contracts`) enable communication and tokens moves between the EVM and Native environments. 

## Dependencies

- [Spring] (https://github.com/AntelopeIO/spring) 1.1 or greater
- [CDT Compiler] (https://github.com/AntelopeIO/cdt) 4.0 or greater
- [Vaulta EVM runtime contract] (https://github.com/VaultaFoundation/evm-contract) 
- solc: (version 0.8.21 or greater)
  + Used to compile the .sol files. 
  + We chose to use solcjs because it is more actively maintained than the solc available from the package manager.
    * First install node.js and npm.
    * Then install solcjs: `npm install -g solc`
  + Make sure to install at least version 0.8.21.
    * Confirm with `solcjs --version`.
- Install `jq` used to compile solidity contracts
  + `apt-get install jq`
- Install `xxd` used to compile solidity contracts
  + `apt-get install xxd`

## Building the EVM bridge contract

```
git submodule update --init --recursive

mkdir build
cd build

export eosevm_DIR=<EVM_RUNTIME_BUILD_DIRECTORY>

cmake -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -Deosevm_DIR=<EVM_RUNTIME_BUILD_DIRECTORY> -Dspring_DIR=<SPRING_DIRECTORY> -Dcdt_DIR=<CDT_DIRECTORY> .. && make -j8

```

You will get the wasm and abi at:
```
./build/antelope_contracts/contracts/erc20/erc20.wasm
./build/antelope_contracts/contracts/erc20/erc20.abi
```


## Running tests

```
cd build && ctest --output-on-failure --verbose 
```

## Design details and deployment steps

Please refer to https://github.com/VaultaFoundation/evm-public-docs/tree/main/Trustless_bridge
