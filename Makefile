BLOCKCHAIN_HOST := 127.0.0.1
BLOCKCHAIN_PORT := 8888
CONTRACT_ACCOUNT := sevensdice
WALLET_HOST := 127.0.0.1
WALLET_PORT := 8900


default: build

build: sevensdice.wast sevensdice.abi

sevensdice.wast:
	eosiocpp -o sevensdice.wast src/sevensdice.cpp

sevensdice.abi:
	eosiocpp -g sevensdice.abi src/sevensdice.cpp

install: build
	cleos \
	  --url http://${BLOCKCHAIN_HOST}:${BLOCKCHAIN_PORT} \
		--wallet-url http://${WALLET_HOST}:${WALLET_PORT} \
		set contract ${CONTRACT_ACCOUNT} $(shell pwd) \
		-p ${CONTRACT_ACCOUNT}

clean:
	rm sevensdice.wast sevensdice.wasm sevensdice.abi
