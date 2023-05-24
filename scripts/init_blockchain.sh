rm -f ~/eosio-wallet/default.wallet

cleos wallet create -f wallet_key

printf '\nfor eosio root account\n'
cleos wallet import --private-key 5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3

printf '\nfor eosio.tocken account\n'
cleos wallet import --private-key 5Jmsawgsp1tQ3GD6JyGCwy1dcvqKZgX6ugMVMdjirx85iv5VyPR

printf '\nCreate token account\n'
cleos create account eosio eosio.token EOS7ijWCBmoXBi3CgtK7DJxentZZeTkeUnaSDvyro9dq7Sd1C3dC4 EOS7ijWCBmoXBi3CgtK7DJxentZZeTkeUnaSDvyro9dq7Sd1C3dC4

printf '\nLoad eosio.token contract\n'
cleos set contract eosio.token ~/src/eos/build/contracts/eosio.token -p eosio.token@active
 
printf '\nCreate msig account\n'
cleos create account eosio eosio.msig EOS7ijWCBmoXBi3CgtK7DJxentZZeTkeUnaSDvyro9dq7Sd1C3dC4 EOS7ijWCBmoXBi3CgtK7DJxentZZeTkeUnaSDvyro9dq7Sd1C3dC4

printf '\nLoad eosio.msig contract\n'
cleos set contract eosio.msig ~/src/eos/build/contracts/eosio.msig -p eosio.msig@active

printf 'Create SYS currency token'
cleos push action eosio.token create '[ "eosio", "1000000000.0000 SYS"]' -p eosio.token@active

printf 'Issuer SYS currency'
cleos push action eosio.token issue  '[ "eosio", "1000000000.0000 SYS", "memo" ]' -p eosio

printf 'Create another important accounts'
printf 'for eosio.bpay'
cleos wallet import --private-key 5KWyhzdQn8XF2BuxRqe1GHBtDA55QwKGgSHciA9jcA6pYuGBa5a
cleos create account eosio eosio.bpay EOS5e8jcy3sEEPj4qfsWHzqX4UM4m6hYXUpbeAKh5aAWRp7wAPbEt EOS5e8jcy3sEEPj4qfsWHzqX4UM4m6hYXUpbeAKh5aAWRp7wAPbEt

printf 'for eosio.names'
cleos wallet import --private-key 5JffvPfN2vFFwmnMXkyif7o98ABGjoGvATRr2pmC9oHRSvfF6Q3
cleos create account eosio eosio.names EOS5aXa6tsqKFBc68zqFX34qtD2NnR5SP8V22dSD9QSKjNZ8dpHYB EOS5aXa6tsqKFBc68zqFX34qtD2NnR5SP8V22dSD9QSKjNZ8dpHYB

printf 'for eosio.ram'
cleos wallet import --private-key 5Jsez43KM8NaU2pfLosHvEvVV1wq7Mf4noqqGFy7cUExEx2wj2S
cleos create account eosio eosio.ram EOS51bhwpDzqMK56WYLNt216PzsM1bBjccytK3UF5qWDLZXTJ9Tvr EOS51bhwpDzqMK56WYLNt216PzsM1bBjccytK3UF5qWDLZXTJ9Tvr

printf 'for eosio.ramfee'
cleos wallet import --private-key 5KhbGBBunVSn4NK8ckcDhCX4HV6hAEeSttxALeigq1Mo4aCk59x
cleos create account eosio eosio.ramfee EOS5ijjo5Z1qfLrucHwJ3bmreEZeH19JpAxboaY8N6T5TM2sTdnJy EOS5ijjo5Z1qfLrucHwJ3bmreEZeH19JpAxboaY8N6T5TM2sTdnJy

printf 'for eosio.saving'
cleos wallet import --private-key 5JyaVVXYhULsEYAFnbogbTZ71yYF5bnTxhYuzgqmZHhQQvZhd6a
cleos create account eosio eosio.saving EOS7Uwi5qBxUSBDSGtUcN5JXkLMYU6dMdD1mHqEUybRrEWQLxeoWx EOS7Uwi5qBxUSBDSGtUcN5JXkLMYU6dMdD1mHqEUybRrEWQLxeoWx

printf 'for eosio.stake'
cleos wallet import --private-key 5JruRkFSkT7ZYAGCPopusLQnYAohd3jg7kto2MAfctrgFATwCAJ
cleos create account eosio eosio.stake EOS75FcH5T4b9ZCdknQTSiCCj4PmK4Swnhy12uJpPFXjMr6EHn5tc EOS75FcH5T4b9ZCdknQTSiCCj4PmK4Swnhy12uJpPFXjMr6EHn5tc

printf 'for eosio.vpay'
cleos wallet import --private-key 5JcMY1XZEJXZ3Q8inoon3Vc73AAC3hCa9o2pV7atSFAe2hf41i7
cleos create account eosio eosio.vpay EOS76AEdQzMbfN5TB9vaNRwJvWUt8VTk1cD5iucpY49dv4PpVkgLy EOS76AEdQzMbfN5TB9vaNRwJvWUt8VTk1cD5iucpY49dv4PpVkgLy

printf 'Load eosio.system contract'
cleos set contract eosio ~/src/eos/build/contracts/eosio.system -p eosio@active

printf 'Make eosio.msig priveledged account'
cleos push action eosio setpriv '["eosio.msig", 1]' -p eosio@active

printf 'Create users'
printf 'accountnum11'
cleos wallet import --private-key 5KApmWa8RNcrFX2tCyDwS3FgJvNRwJsSdqrJGJ3sXBusL1kWgVy
cleos system newaccount --transfer --stake-net "100000.0000 SYS" --stake-cpu "100000.0000 SYS" --buy-ram-kbytes 1000 eosio accountnum11 EOS5tcDvCzu6c14UDCFEVwscmQo7Nt1yFBfUyRr4e6dRv4dKo6pqu

printf 'accountnum12'
cleos wallet import --private-key 5JPH9w8HZty3qQfqyaGcJbKFQSc7shhcnqTUWCdd14pDgYo9tdy

cleos system newaccount --transfer --stake-net "100000.0000 SYS" --stake-cpu "100000.0000 SYS" --buy-ram-kbytes 10 eosio accountnum12 EOS834QZ9jCrHkDt5tcH5RK4qZXk4dysMtowQJ9icqpRDYR2xoP7c

printf 'accountnum13'
cleos wallet import --private-key 5KCgkbGzFfDoj76oikPJYRSV8AB3UPGZJMnfzGGUSk3M71Egp2h

cleos system newaccount --transfer --stake-net "100000.0000 SYS" --stake-cpu "100000.0000 SYS" --buy-ram-kbytes 10 eosio accountnum13 EOS6TgH2hXzr5ZszEXmwMmMT1vn2bZRuYErrYj4crkzYTYsZsTP4Q

printf 'accountnum14'
cleos wallet import --private-key 5Jr1oDeN4Ur9Eik478qPvMgAXQQmYtSFdB5p2iyNLffvKksFMSY

cleos system newaccount --transfer --stake-net "100000.0000 SYS" --stake-cpu "100000.0000 SYS" --buy-ram-kbytes 10 eosio accountnum14 EOS6gChGhpgNs19mf7A9K4S9jsFNQPDzjhmSe4x1A9eUCAoxQCDVA

printf 'accountnum15'
cleos wallet import --private-key 5KECJsWYJ7mWrFCcdySxEEcfxT7TpBEhtUcPYLg6z41HwVnu1tT

cleos system newaccount --transfer --stake-net "100000.0000 SYS" --stake-cpu "100000.0000 SYS" --buy-ram-kbytes 10 eosio accountnum15 EOS5xTxYoUerxeoevFTDRea4Axhx7tWMGFzrRYpBfdh6b1twH4dB7

printf 'accountnum21'
cleos wallet import --private-key 5JmPH7io3W1FngvARC51Q8JgWtpSZcKmvuGF72x9TMUDfX8gKmw

cleos system newaccount --transfer --stake-net "100000.0000 SYS" --stake-cpu "100000.0000 SYS" --buy-ram-kbytes 10 eosio accountnum21 EOS852UZxeVVgqaXRhpUh6LD1g9AmTXJPjwiHTfpTMxHKowQzgoX1

printf 'accountnum22'
cleos wallet import --private-key 5KTjCvtGnJQqA7EtwW6KQSU1eFTSRhFSrJxdhF1eLun87ubAtLS

cleos system newaccount --transfer --stake-net "100000.0000 SYS" --stake-cpu "100000.0000 SYS" --buy-ram-kbytes 10 eosio accountnum22 EOS6H7D9i5LFitor78joFxXc9ux3V2qy5pjrwinzcefsafvEN4Vga

printf 'accountnum23'
cleos wallet import --private-key 5KKZooE31YXUbgt5ZBSVyoh5KUoSajzsdt3VsERwh39fwntdSp6

cleos system newaccount --transfer --stake-net "100000.0000 SYS" --stake-cpu "100000.0000 SYS" --buy-ram-kbytes 10 eosio accountnum23 EOS7YdkP1JzKCEhCJGZnZXQVNkgtnZfq3Hmn8H5VmMjBnXL1YWdnx

printf 'accountnum24'
cleos wallet import --private-key 5JUvSwNnEYbUupDYRsJGr4XaVrNzWiBNoSDdmWByaJjkj7dqRHB

cleos system newaccount --transfer --stake-net "100000.0000 SYS" --stake-cpu "100000.0000 SYS" --buy-ram-kbytes 10 eosio accountnum24 EOS8fjDpSwUimBQ9kaBZ91WnHAT9gK6zZ5aEjshP4T87DxF4PQ3x1

printf 'accountnum25'
cleos wallet import --private-key 5KVmupw9SZW3dkNiMpV1kySBTfnkSUS89PY8j4CopFesL2XLGCH

cleos system newaccount --transfer --stake-net "100000.0000 SYS" --stake-cpu "100000.0000 SYS" --buy-ram-kbytes 10 eosio accountnum25 EOS5vwPLocnBdL2JuPxCRds1v5Nr8xHE4xaSHQkeg2sV3NNsyHdC1


printf '10 пользователей создали'

printf 'Regproducers'
cleos system regproducer accountnum11 EOS5tcDvCzu6c14UDCFEVwscmQo7Nt1yFBfUyRr4e6dRv4dKo6pqu http://accountnum11.com/EOS5tcDvCzu6c14UDCFEVwscmQo7Nt1yFBfUyRr4e6dRv4dKo6pqu
cleos system regproducer accountnum12 EOS834QZ9jCrHkDt5tcH5RK4qZXk4dysMtowQJ9icqpRDYR2xoP7c http://accountnum12.com/EOS834QZ9jCrHkDt5tcH5RK4qZXk4dysMtowQJ9icqpRDYR2xoP7c
cleos system regproducer accountnum13 EOS6TgH2hXzr5ZszEXmwMmMT1vn2bZRuYErrYj4crkzYTYsZsTP4Q http://accountnum13.com/EOS6TgH2hXzr5ZszEXmwMmMT1vn2bZRuYErrYj4crkzYTYsZsTP4Q
cleos system regproducer accountnum14 EOS6gChGhpgNs19mf7A9K4S9jsFNQPDzjhmSe4x1A9eUCAoxQCDVA http://accountnum14.com/EOS6gChGhpgNs19mf7A9K4S9jsFNQPDzjhmSe4x1A9eUCAoxQCDVA
cleos system regproducer accountnum15 EOS5xTxYoUerxeoevFTDRea4Axhx7tWMGFzrRYpBfdh6b1twH4dB7 http://accountnum15.com/EOS5xTxYoUerxeoevFTDRea4Axhx7tWMGFzrRYpBfdh6b1twH4dB7
cleos system regproducer accountnum21 EOS852UZxeVVgqaXRhpUh6LD1g9AmTXJPjwiHTfpTMxHKowQzgoX1 http://accountnum21.com/EOS852UZxeVVgqaXRhpUh6LD1g9AmTXJPjwiHTfpTMxHKowQzgoX1
cleos system regproducer accountnum22 EOS6H7D9i5LFitor78joFxXc9ux3V2qy5pjrwinzcefsafvEN4Vga http://accountnum22.com/EOS6H7D9i5LFitor78joFxXc9ux3V2qy5pjrwinzcefsafvEN4Vga

printf 'transfer tockens to users'
cleos push action eosio.token transfer '["eosio", "accountnum11","99000000.0000 SYS",""]' -p eosio

cleos push action eosio.token transfer '["eosio", "accountnum12","99000000.0000 SYS",""]' -p eosio

cleos push action eosio.token transfer '["eosio", "accountnum13","99000000.0000 SYS",""]' -p eosio

cleos push action eosio.token transfer '["eosio", "accountnum14","99000000.0000 SYS",""]' -p eosio

cleos push action eosio.token transfer '["eosio", "accountnum15","99000000.0000 SYS",""]' -p eosio

cleos push action eosio.token transfer '["eosio", "accountnum21","99000000.0000 SYS",""]' -p eosio

cleos push action eosio.token transfer '["eosio", "accountnum22","99000000.0000 SYS",""]' -p eosio

cleos push action eosio.token transfer '["eosio", "accountnum23","99000000.0000 SYS",""]' -p eosio

cleos push action eosio.token transfer '["eosio", "accountnum24","99000000.0000 SYS",""]' -p eosio

cleos push action eosio.token transfer '["eosio", "accountnum25","99000000.0000 SYS",""]' -p eosio

fprintf '\nstake our maney\n'
cleos system delegatebw accountnum11 accountnum11 "80000000 SYS" "9000000 SYS"
cleos system delegatebw accountnum12 accountnum12 "80000000 SYS" "9000000 SYS"
cleos system delegatebw accountnum13 accountnum13 "80000000 SYS" "9000000 SYS"
cleos system delegatebw accountnum14 accountnum14 "80000000 SYS" "9000000 SYS"
cleos system delegatebw accountnum15 accountnum15 "80000000 SYS" "9000000 SYS"
cleos system delegatebw accountnum21 accountnum21 "80000000 SYS" "9000000 SYS"
cleos system delegatebw accountnum22 accountnum22 "80000000 SYS" "9000000 SYS"
cleos system delegatebw accountnum23 accountnum23 "80000000 SYS" "9000000 SYS"
cleos system delegatebw accountnum24 accountnum24 "80000000 SYS" "9000000 SYS"
cleos system delegatebw accountnum25 accountnum25 "80000000 SYS" "9000000 SYS"

cleos system voteproducer prods  accountnum11 accountnum11 accountnum12 accountnum13 accountnum14 accountnum15 accountnum21 accountnum22  
cleos system voteproducer prods  accountnum12 accountnum11 accountnum12 accountnum13 accountnum14 accountnum15 accountnum21 accountnum22  
cleos system voteproducer prods  accountnum13 accountnum11 accountnum12 accountnum13 accountnum14 accountnum15 accountnum21 accountnum22  
cleos system voteproducer prods  accountnum14 accountnum11 accountnum12 accountnum13 accountnum14 accountnum15 accountnum21 accountnum22  
cleos system voteproducer prods  accountnum15 accountnum11 accountnum12 accountnum13 accountnum14 accountnum15 accountnum21 accountnum22  
cleos system voteproducer prods  accountnum21 accountnum11 accountnum12 accountnum13 accountnum14 accountnum15 accountnum21 accountnum22  
cleos system voteproducer prods  accountnum22 accountnum11 accountnum12 accountnum13 accountnum14 accountnum15 accountnum21 accountnum22  
cleos system voteproducer prods  accountnum23 accountnum11 accountnum12 accountnum13 accountnum14 accountnum15 accountnum21 accountnum22  
cleos system voteproducer prods  accountnum24 accountnum11 accountnum12 accountnum13 accountnum14 accountnum15 accountnum21 accountnum22  
cleos system voteproducer prods  accountnum25 accountnum11 accountnum12 accountnum13 accountnum14 accountnum15 accountnum21 accountnum22  

