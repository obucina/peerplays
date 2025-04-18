file(GLOB_RECURSE HEADERS "include/graphene/peerplays_sidechain/*.hpp")

add_library( peerplays_sidechain
             peerplays_sidechain_plugin.cpp
             sidechain_api.cpp
             sidechain_net_handler_factory.cpp
             sidechain_net_handler.cpp
             sidechain_net_handler_bitcoin.cpp
             sidechain_net_handler_ethereum.cpp
             sidechain_net_handler_hive.cpp
             sidechain_net_handler_peerplays.cpp
             bitcoin/bech32.cpp
             bitcoin/bitcoin_address.cpp
             bitcoin/bitcoin_script.cpp
             bitcoin/bitcoin_transaction.cpp
             bitcoin/segwit_addr.cpp
             bitcoin/utils.cpp
             bitcoin/sign_bitcoin_transaction.cpp
             bitcoin/libbitcoin_client.cpp
             bitcoin/estimate_fee_external.cpp
             common/rpc_client.cpp
             common/utils.cpp
             ethereum/encoders.cpp
             ethereum/decoders.cpp
             ethereum/transaction.cpp
             ethereum/types.cpp
             ethereum/utils.cpp
             hive/asset.cpp
             hive/operations.cpp
             hive/transaction.cpp
             hive/types.cpp
           )

if (ENABLE_DEV_FEATURES)
    set(ENABLE_PEERPLAYS_ASSET_DEPOSITS 1)
endif()
unset(ENABLE_DEV_FEATURES)
unset(ENABLE_DEV_FEATURES CACHE)

if (ENABLE_PEERPLAYS_ASSET_DEPOSITS)
   message ("Depositing Peerplays assets enabled")
   target_compile_definitions(peerplays_sidechain PRIVATE ENABLE_PEERPLAYS_ASSET_DEPOSITS)
endif()
unset(ENABLE_PEERPLAYS_ASSET_DEPOSITS)
unset(ENABLE_PEERPLAYS_ASSET_DEPOSITS CACHE)

target_link_libraries( peerplays_sidechain PRIVATE curl graphene_plugin sha3 zmq bitcoin-system bitcoin-protocol bitcoin-client bitcoin-explorer )
target_include_directories( peerplays_sidechain
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

install( TARGETS
   peerplays_sidechain

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
INSTALL( FILES ${HEADERS} DESTINATION "include/graphene/peerplays_sidechain" )

