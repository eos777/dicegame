MAIN_HOST :=
MAIN_PORT := 
JUNGLE_HOST := jungle2.cryptolions.io
JUNGLE_PORT := 443
CONTRACT_ACCOUNT := casinosevens

all: build

build: wasm abi

wasm:
	eosio-cpp src/dicegame.cpp -o dicegame.wasm

abi:
	eosio-abigen src/dicegame.cpp --contract=${CONTRACT_ACCOUNT} --output=dicegame.abi

main: build
	cleos \
	  --url https://${MAIN_HOST}:${MAIN_PORT} \
		set contract ${CONTRACT_ACCOUNT} $(shell pwd) \
		-p ${CONTRACT_ACCOUNT}

jungle: build
	cleos \
	  --url https://${JUNGLE_HOST}:${JUNGLE_PORT} \
		set contract ${CONTRACT_ACCOUNT} $(shell pwd) \
		-p ${CONTRACT_ACCOUNT}

clean:
	rm dicegame.wasm dicegame.abi
