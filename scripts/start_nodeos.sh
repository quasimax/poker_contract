nodeos -e -p eosio --config-dir . -d . --plugin eosio::producer_plugin --plugin eosio::chain_api_plugin --plugin eosio::http_plugin --plugin eosio::history_api_plugin --plugin eosio::net_api_plugin --contracts-console --verbose-http-errors 2> node.txt 1> node.txt &