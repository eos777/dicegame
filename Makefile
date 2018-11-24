BLOCKCHAIN_HOST := 127.0.0.1
BLOCKCHAIN_PORT := 8888
CONTRACT_ACCOUNT := dice
WALLET_HOST := 127.0.0.1
WALLET_PORT := 9876

all: build

build: wasm abi

wasm:
	eosio-cpp src/sevensdice.cpp -o sevensdice.wasm

abi:
	eosio-abigen src/sevensdice.cpp --contract=hello --output=sevensdice.abi

install: build
	cleos \
	  --url http://${BLOCKCHAIN_HOST}:${BLOCKCHAIN_PORT} \
		--wallet-url http://${WALLET_HOST}:${WALLET_PORT} \
		set contract ${CONTRACT_ACCOUNT} $(shell pwd) \
		-p ${CONTRACT_ACCOUNT}

clean:
	rm sevensdice.wasm sevensdice.abi
