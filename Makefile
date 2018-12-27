MAIN_HOST :=
MAIN_PORT := 
JUNGLE_HOST := jungle2.cryptolions.io
JUNGLE_PORT := 443
CONTRACT_ACCOUNT := casinosevens

all: wasmabi

wasmabi:
	eosio-cpp --abigen src/dicegame.cpp -o dicegame.wasm

main: wasmabi
	cleos \
	  --url https://${MAIN_HOST}:${MAIN_PORT} \
		set contract ${CONTRACT_ACCOUNT} $(shell pwd) \
		-p ${CONTRACT_ACCOUNT}

jungle: wasmabi
	cleos \
	  --url https://${JUNGLE_HOST}:${JUNGLE_PORT} \
		set contract ${CONTRACT_ACCOUNT} $(shell pwd) \
		-p ${CONTRACT_ACCOUNT}

clean:
	rm dicegame.wasm dicegame.abi
