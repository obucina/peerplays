/*
 * Copyright (c) 2015 Cryptonomex, Inc., and contributors.
 *
 * The MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#pragma once
#include <graphene/chain/global_property_object.hpp>
#include <graphene/chain/node_property_object.hpp>
#include <graphene/chain/account_object.hpp>
#include <graphene/chain/asset_object.hpp>
#include <graphene/chain/fork_database.hpp>
#include <graphene/chain/block_database.hpp>
#include <graphene/chain/genesis_state.hpp>
#include <graphene/chain/evaluator.hpp>

#include <graphene/db/object_database.hpp>
#include <graphene/db/object.hpp>
#include <graphene/db/simple_index.hpp>
#include <fc/signals.hpp>

#include <fc/crypto/hash_ctr_rng.hpp>

#include <graphene/chain/protocol/protocol.hpp>

#include <graphene/chain/sidechain_defs.hpp>

#include <fc/log/logger.hpp>

#include <map>

namespace graphene { namespace chain {
   using graphene::db::abstract_object;
   using graphene::db::object;
   class op_evaluator;
   class transaction_evaluation_state;

   struct budget_record;

   /**
    *   @class database
    *   @brief tracks the blockchain state in an extensible manner
    */
   class database : public db::object_database
   {
      public:
         //////////////////// db_management.cpp ////////////////////

         database();
         ~database();

         std::vector<fc::time_point_sec> _hardfork_times;

         enum validation_steps
         {
            skip_nothing                = 0,
            skip_witness_signature      = 1 << 0,  ///< used while reindexing
            skip_transaction_signatures = 1 << 1,  ///< used by non-witness nodes
            skip_transaction_dupe_check = 1 << 2,  ///< used while reindexing
            skip_fork_db                = 1 << 3,  ///< used while reindexing
            skip_block_size_check       = 1 << 4,  ///< used when applying locally generated transactions
            skip_tapos_check            = 1 << 5,  ///< used while reindexing -- note this skips expiration check as well
            skip_authority_check        = 1 << 6,  ///< used while reindexing -- disables any checking of authority on transactions
            skip_merkle_check           = 1 << 7,  ///< used while reindexing
            skip_assert_evaluation      = 1 << 8,  ///< used while reindexing
            skip_undo_history_check     = 1 << 9,  ///< used while reindexing
            skip_witness_schedule_check = 1 << 10,  ///< used while reindexing
            skip_validate               = 1 << 11 ///< used prior to checkpoint, skips validate() call on transaction
         };

         /**
          * @brief Open a database, creating a new one if necessary
          *
          * Opens a database in the specified directory. If no initialized database is found, genesis_loader is called
          * and its return value is used as the genesis state when initializing the new database
          *
          * genesis_loader will not be called if an existing database is found.
          *
          * @param data_dir Path to open or create database in
          * @param genesis_loader A callable object which returns the genesis state to initialize new databases on
          * @param db_version a version string that changes when the internal database format and/or logic is modified
          */
          void open(
             const fc::path& data_dir,
             std::function<genesis_state_type()> genesis_loader,
             const std::string& db_version );

         /**
          * @brief Rebuild object graph from block history and open detabase
          *
          * This method may be called after or instead of @ref database::open, and will rebuild the object graph by
          * replaying blockchain history. When this method exits successfully, the database will be open.
          */
         void reindex(fc::path data_dir);

         /**
          * @brief wipe Delete database from disk, and potentially the raw chain as well.
          * @param include_blocks If true, delete the raw chain as well as the database.
          *
          * Will close the database before wiping. Database will be closed when this function returns.
          */
         void wipe(const fc::path& data_dir, bool include_blocks);
         void close(bool rewind = true);

         //////////////////// db_block.cpp ////////////////////

         /**
          *  @return true if the block is in our fork DB or saved to disk as
          *  part of the official chain, otherwise return false
          */
         bool                       is_known_block( const block_id_type& id )const;
         bool                       is_known_transaction( const transaction_id_type& id )const;
         block_id_type              get_block_id_for_num( uint32_t block_num )const;
         optional<signed_block>     fetch_block_by_id( const block_id_type& id )const;
         optional<signed_block>     fetch_block_by_number( uint32_t num )const;
         const signed_transaction&  get_recent_transaction( const transaction_id_type& trx_id )const;
         std::vector<block_id_type> get_block_ids_on_fork(block_id_type head_of_fork) const;

         /**
          *  Calculate the percent of block production slots that were missed in the
          *  past 128 blocks, not including the current block.
          */
         uint32_t witness_participation_rate()const;

         void                              add_checkpoints( const flat_map<uint32_t,block_id_type>& checkpts );
         const flat_map<uint32_t,block_id_type> get_checkpoints()const { return _checkpoints; }
         bool before_last_checkpoint()const;

         void check_transaction_for_duplicated_operations(const signed_transaction& trx);

         bool push_block( const signed_block& b, uint32_t skip = skip_nothing );
         processed_transaction push_transaction( const signed_transaction& trx, uint32_t skip = skip_nothing );
         bool _push_block( const signed_block& b );
         processed_transaction _push_transaction( const signed_transaction& trx );

         ///@throws fc::exception if the proposed transaction fails to apply.
         processed_transaction push_proposal( const proposal_object& proposal );

         signed_block generate_block(
            const fc::time_point_sec when,
            witness_id_type witness_id,
            const fc::ecc::private_key& block_signing_private_key,
            uint32_t skip
            );
         signed_block _generate_block(
            const fc::time_point_sec when,
            witness_id_type witness_id,
            const fc::ecc::private_key& block_signing_private_key
            );

         void pop_block();
         void clear_pending();

         /**
          *  This method is used to track appied operations during the evaluation of a block, these
          *  operations should include any operation actually included in a transaction as well
          *  as any implied/virtual operations that resulted, such as filling an order.  The
          *  applied operations is cleared after applying each block and calling the block
          *  observers which may want to index these operations.
          *
          *  @return the op_id which can be used to set the result after it has finished being applied.
          */
         uint32_t  push_applied_operation( const operation& op );
         void      set_applied_operation_result( uint32_t op_id, const operation_result& r );

         // most plugins should use the const version of get_applied_operations
         const vector<optional< operation_history_object > >& get_applied_operations()const;

         // the account_history plugin uses the non-const version.  When it decides to track an 
         // operation and assigns an operation_id to it, it will store that id into the operation
         // history object so other plugins that evaluate later can reference it.
         vector<optional< operation_history_object > >& get_applied_operations();

         // the bookie plugin depends on change notifications that are skipped during normal replays
         void force_slow_replays();

         string to_pretty_string( const asset& a )const;

         /**
          *  This signal is emitted after all operations and virtual operation for a
          *  block have been applied but before the get_applied_operations() are cleared.
          *
          *  You may not yield from this callback because the blockchain is holding
          *  the write lock and may be in an "inconstant state" until after it is
          *  released.
          */
         fc::signal<void(const signed_block&)>           applied_block;

         /**
          * This signal is emitted any time a new transaction is added to the pending
          * block state.
          */
         fc::signal<void(const signed_transaction&)>     on_pending_transaction;

         /**
          *  Emitted After a block has been applied and committed.  The callback
          *  should not yield and should execute quickly.
          */
         fc::signal<void(const vector<object_id_type>&, const flat_set<account_id_type>&)> new_objects;

         /**
          *  Emitted After a block has been applied and committed.  The callback
          *  should not yield and should execute quickly.
          */
         fc::signal<void(const vector<object_id_type>&, const flat_set<account_id_type>&)> changed_objects;

         /** this signal is emitted any time an object is removed and contains a
          * pointer to the last value of every object that was removed.
          */
         fc::signal<void(const vector<object_id_type>&, const vector<const object*>&, const flat_set<account_id_type>&)>  removed_objects;

         //////////////////// db_witness_schedule.cpp ////////////////////

         /**
          * @brief Get the witness scheduled for block production in a slot.
          *
          * slot_num always corresponds to a time in the future.
          *
          * If slot_num == 1, returns the next scheduled witness.
          * If slot_num == 2, returns the next scheduled witness after
          * 1 block gap.
          *
          * Use the get_slot_time() and get_slot_at_time() functions
          * to convert between slot_num and timestamp.
          *
          * Passing slot_num == 0 returns GRAPHENE_NULL_WITNESS
          */
         witness_id_type get_scheduled_witness(uint32_t slot_num)const;

         /**
          * @brief Get son schedule id for the given sidechain_type.
          *
          * type sidechain_type we getting schedule.
          *
          * returns Id of the schedule object.
          */
         unsigned_int get_son_schedule_id(sidechain_type type)const;

         /**
          * @brief Get the bitcoin or hive son scheduled for block production in a slot.
          *
          * slot_num always corresponds to a time in the future.
          *
          * If slot_num == 1, returns the next scheduled son.
          * If slot_num == 2, returns the next scheduled son after
          * 1 block gap.
          *
          * Use the get_slot_time() and get_slot_at_time() functions
          * to convert between slot_num and timestamp.
          *
          * Passing slot_num == 0 returns GRAPHENE_NULL_WITNESS
          */
         son_id_type get_scheduled_son(sidechain_type type, uint32_t slot_num)const;

         /**
          * Get the time at which the given slot occurs.
          *
          * If slot_num == 0, return time_point_sec().
          *
          * If slot_num == N for N > 0, return the Nth next
          * block-interval-aligned time greater than head_block_time().
          */
         fc::time_point_sec get_slot_time(uint32_t slot_num)const;

         /**
          * Get the last slot which occurs AT or BEFORE the given time.
          *
          * The return value is the greatest value N such that
          * get_slot_time( N ) <= when.
          *
          * If no such N exists, return 0.
          */
         uint32_t get_slot_at_time(fc::time_point_sec when)const;

         vector<witness_id_type> get_near_witness_schedule()const;
         void update_witness_schedule();
         void update_witness_schedule(const signed_block& next_block);
         void update_son_schedule(sidechain_type type);
         void update_son_schedule(sidechain_type type, const signed_block& next_block);
      
         void check_lottery_end_by_participants( asset_id_type asset_id );
         void check_ending_lotteries();
         void check_ending_nft_lotteries();

         //////////////////// db_getter.cpp ////////////////////

         const chain_id_type&                   get_chain_id()const;
         const asset_object&                    get_core_asset()const;
         const asset_dynamic_data_object&       get_core_dynamic_data()const;
         const chain_property_object&           get_chain_properties()const;
         const global_property_object&          get_global_properties()const;
         const dynamic_global_property_object&  get_dynamic_global_properties()const;
         const node_property_object&            get_node_properties()const;
         const fee_schedule&                    current_fee_schedule()const;
         const account_statistics_object&       get_account_stats_by_owner( account_id_type owner )const;
         const std::vector<uint32_t>            get_winner_numbers( asset_id_type for_asset, uint32_t count_members, uint8_t count_winners ) const;
         std::vector<uint32_t>                  get_seeds( asset_id_type for_asset, uint8_t count_winners )const;
         uint64_t                               get_random_bits( uint64_t bound );
         const witness_schedule_object&         get_witness_schedule_object()const;
         bool                                   item_locked(const nft_id_type& item)const;
         bool                                   account_role_valid(const account_role_object& aro, account_id_type account, optional<int> op_type = optional<int>()) const;
         std::set<son_id_type>                  get_sons_being_deregistered();
         std::set<son_id_type>                  get_sons_being_reported_down();
         std::set<son_id_type>                  get_sons_to_be_deregistered();
         fc::optional<operation>                create_son_deregister_proposal( son_id_type son_id, account_id_type paying_son );
         signed_transaction                     create_signed_transaction( const fc::ecc::private_key& signing_private_key, const operation& op );
         bool                                   is_son_dereg_valid( son_id_type son_id );
         bool                                   is_son_active( sidechain_type type, son_id_type son_id );
         bool                                   is_asset_creation_allowed(const string& symbol);

         time_point_sec   head_block_time()const;
         uint32_t         head_block_num()const;
         block_id_type    head_block_id()const;
         witness_id_type  head_block_witness()const;

         decltype( chain_parameters::block_interval ) block_interval( )const;

         node_property_object& node_properties();


         uint32_t last_non_undoable_block_num() const;
         vector<authority> get_account_custom_authorities(account_id_type account, const operation& op)const;
         vector<uint64_t> get_random_numbers(uint64_t minimum, uint64_t maximum, uint64_t selections, bool duplicates);
         //////////////////// db_init.cpp ////////////////////

         void initialize_evaluators();
         /// Reset the object graph in-memory
         void initialize_indexes();
         void initialize_hardforks();

         void init_genesis(const genesis_state_type& genesis_state = genesis_state_type());

         template<typename EvaluatorType>
         void register_evaluator()
         {
            _operation_evaluators[
               operation::tag<typename EvaluatorType::operation_type>::value].reset( new op_evaluator_impl<EvaluatorType>() );
         }

         //////////////////// db_balance.cpp ////////////////////

         /**
          * @brief Retrieve a particular account's balance in a given asset
          * @param owner Account whose balance should be retrieved
          * @param asset_id ID of the asset to get balance in
          * @return owner's balance in asset
          */
         asset get_balance(account_id_type owner, asset_id_type asset_id)const;
         /// This is an overloaded method.
         asset get_balance(const account_object& owner, const asset_object& asset_obj)const;
         /**
          * @brief Get balance connected with lottery asset; if assset isnt lottery - return asset(0, 0)
          */
         asset get_balance(asset_id_type lottery_id)const;
         /**
          * @brief Adjust a particular account's balance in a given asset by a delta
          * @param account ID of account whose balance should be adjusted
          * @param delta Asset ID and amount to adjust balance by
          */
         void adjust_balance(account_id_type account, asset delta);
         /**
          * @brief Adjust a lottery's balance in a given asset by a delta
          * @param asset ID(should be lottery) balance should be adjusted
          * @param delta Asset ID and amount to adjust balance by
          */
         void adjust_balance(asset_id_type lottery_id, asset delta);
         /**
          * @brief Adjust a particular account's sweeps vesting balance in a given asset by a delta
          */
         void adjust_sweeps_vesting_balance(account_id_type account, int64_t delta);

         /**
          * @brief Helper to make lazy deposit to CDD VBO.
          *
          * If the given optional VBID is not valid(),
          * or it does not have a CDD vesting policy,
          * or the owner / vesting_seconds of the policy
          * does not match the parameter, then credit amount
          * to newly created VBID and return it.
          *
          * Otherwise, credit amount to ovbid.
          * 
          * @return ID of newly created VBO, but only if VBO was created.
          */
         optional< vesting_balance_id_type > deposit_lazy_vesting(
            const optional< vesting_balance_id_type >& ovbid,
            share_type amount,
            uint32_t req_vesting_seconds,
            account_id_type req_owner,
            bool require_vesting );

         // helper to handle cashback rewards
         void deposit_cashback(const account_object& acct, share_type amount, bool require_vesting = true);
         // helper to handle witness pay
         void deposit_witness_pay(const witness_object& wit, share_type amount);

         //////////////////// db_debug.cpp ////////////////////

         void debug_dump();
         void apply_debug_updates();
         void debug_update( const fc::variant_object& update );

         //////////////////// db_market.cpp ////////////////////

         /// @{ @group Market Helpers
         void globally_settle_asset( const asset_object& bitasset, const price& settle_price );
         void cancel_order(const force_settlement_object& order, bool create_virtual_op = true);
         void cancel_order(const limit_order_object& order, bool create_virtual_op = true);

         /**
          * @brief Process a new limit order through the markets
          * @param order The new order to process
          * @return true if order was completely filled; false otherwise
          *
          * This function takes a new limit order, and runs the markets attempting to match it with existing orders
          * already on the books.
          */
         bool apply_order(const limit_order_object& new_order_object, bool allow_black_swan = true);

         /**
          * Matches the two orders,
          *
          * @return a bit field indicating which orders were filled (and thus removed)
          *
          * 0 - no orders were matched
          * 1 - bid was filled
          * 2 - ask was filled
          * 3 - both were filled
          */
         ///@{
         template<typename OrderType>
         int match( const limit_order_object& bid, const OrderType& ask, const price& match_price );
         int match( const limit_order_object& bid, const limit_order_object& ask, const price& trade_price );
         /// @return the amount of asset settled
         asset match(const call_order_object& call,
                   const force_settlement_object& settle,
                   const price& match_price,
                   asset max_settlement);
         ///@}

         //////////////////// db_bet.cpp ////////////////////

         /// @{ @group Betting Market Helpers
         void cancel_bet(const bet_object& bet, bool create_virtual_op = true);
         void cancel_all_unmatched_bets_on_betting_market(const betting_market_object& betting_market);
         void cancel_all_unmatched_bets_on_betting_market_group(const betting_market_group_object& betting_market_group);
         void validate_betting_market_group_resolutions(const betting_market_group_object& betting_market_group,
                                                        const std::map<betting_market_id_type, betting_market_resolution_type>& resolutions);
         void resolve_betting_market_group(const betting_market_group_object& betting_market_group,
                                           const std::map<betting_market_id_type, betting_market_resolution_type>& resolutions);
         void settle_betting_market_group(const betting_market_group_object& betting_market_group);
         void remove_completed_events();
         /**
          * @brief Process a new bet
          * @param new_bet_object The new bet to process
          * @return true if order was completely filled; false otherwise
          *
          * This function takes a new bet and attempts to match it with existing
          * bets already on the books.
          */
         bool place_bet(const bet_object& new_bet_object);
         ///@}

         /**
          * @return true if the order was completely filled and thus freed.
          */
         bool fill_order( const limit_order_object& order, const asset& pays, const asset& receives, bool cull_if_small );
         bool fill_order( const call_order_object& order, const asset& pays, const asset& receives );
         bool fill_order( const force_settlement_object& settle, const asset& pays, const asset& receives );

         bool check_call_orders( const asset_object& mia, bool enable_black_swan = true, bool for_new_limit_order = false,
                                 const asset_bitasset_data_object* bitasset_ptr = nullptr );

         // helpers to fill_order
         void pay_order( const account_object& receiver, const asset& receives, const asset& pays );

         asset calculate_market_fee(const asset_object& recv_asset, const asset& trade_amount);
         asset pay_market_fees( const asset_object& recv_asset, const asset& receives );


         ///@{
         /**
          *  This method validates transactions without adding it to the pending state.
          *  @return true if the transaction would validate
          */
         processed_transaction validate_transaction( const signed_transaction& trx );


         /** when popping a block, the transactions that were removed get cached here so they
          * can be reapplied at the proper time */
         std::deque< signed_transaction >       _popped_tx;

         /**
          * @}
          */
         /// Enable or disable tracking of votes of standby witnesses and committee members
         inline void enable_standby_votes_tracking(bool enable)  { _track_standby_votes = enable; }
   protected:
         //Mark pop_undo() as protected -- we do not want outside calling pop_undo(); it should call pop_block() instead
         void pop_undo() { object_database::pop_undo(); }
         void notify_applied_block( const signed_block& block );
         void notify_on_pending_transaction( const signed_transaction& tx );
         void notify_changed_objects();

      private:
         std::mutex                             _pending_tx_session_mutex;
         optional<undo_database::session>       _pending_tx_session;
         vector< unique_ptr<op_evaluator> >     _operation_evaluators;

         template<class Index>
         vector<std::reference_wrapper<const typename Index::object_type>> sort_votable_objects(size_t count)const;

         template<class Index>
         vector<std::reference_wrapper<const typename Index::object_type>> sort_votable_objects(sidechain_type sidechain, size_t count)const;

         //////////////////// db_block.cpp ////////////////////

       public:
         // these were formerly private, but they have a fairly well-defined API, so let's make them public
         void                  apply_block( const signed_block& next_block, uint32_t skip = skip_nothing );
         processed_transaction apply_transaction( const signed_transaction& trx, uint32_t skip = skip_nothing );
         operation_result      apply_operation( transaction_evaluation_state& eval_state, const operation& op );
      private:
         void                  _apply_block( const signed_block& next_block );
         processed_transaction _apply_transaction( const signed_transaction& trx );
      
         ///Steps involved in applying a new block
         ///@{

         const witness_object& validate_block_header( uint32_t skip, const signed_block& next_block )const;
         const witness_object& _validate_block_header( const signed_block& next_block )const;
         void verify_signing_witness( const signed_block& new_block, const fork_item& fork_entry )const;
         void update_witnesses( fork_item& fork_entry )const;
         void create_block_summary(const signed_block& next_block);

         //////////////////// db_witness_schedule.cpp ////////////////////
         uint32_t update_witness_missed_blocks( const signed_block& b );

         //////////////////// db_update.cpp ////////////////////
         void update_global_dynamic_data( const signed_block& b, const uint32_t missed_blocks );
         void update_signing_witness(const witness_object& signing_witness, const signed_block& new_block);
         void update_last_irreversible_block();
         void clear_expired_transactions();
         void place_delayed_bets();
         void clear_expired_proposals();
         void clear_expired_orders();
         void update_expired_feeds();
         void update_core_exchange_rates();
         void update_maintenance_flag( bool new_maintenance_flag );
         void update_withdraw_permissions();
         void update_tournaments();
         void update_betting_markets(fc::time_point_sec current_block_time);
         bool check_for_blackswan( const asset_object& mia, bool enable_black_swan = true,
                                   const asset_bitasset_data_object* bitasset_ptr = nullptr );
         void finalize_expired_offers();

         ///Steps performed only at maintenance intervals
         ///@{

         //////////////////// db_maint.cpp ////////////////////

         void initialize_budget_record( fc::time_point_sec now, budget_record& rec )const;
         void process_budget();
         void pay_workers( share_type& budget );
         void pay_sons_before_hf_ethereum();
         void pay_sons_after_hf_ethereum();
         void perform_son_tasks();
         void perform_chain_maintenance(const signed_block& next_block, const global_property_object& global_props);
         void update_active_witnesses();
         void update_active_committee_members();
         void update_son_metrics( const flat_map<sidechain_type, vector<son_sidechain_info> >& curr_active_sons );
         void update_active_sons();
         void remove_son_proposal( const proposal_object& proposal );
         void remove_inactive_son_down_proposals( const vector<son_id_type>& son_ids_to_remove );
         void remove_inactive_son_proposals( const vector<son_id_type>& son_ids_to_remove );
         void update_son_statuses( const flat_map<sidechain_type, vector<son_sidechain_info> >& curr_active_sons,
                                   const flat_map<sidechain_type, vector<son_sidechain_info> >& new_active_sons );
         void update_son_wallet( const flat_map<sidechain_type, vector<son_sidechain_info> >& new_active_sons );
         void update_worker_votes();

         public:
            double calculate_vesting_factor(const account_object& stake_account);
            uint32_t get_gpos_current_subperiod();

         template<class Type>
         void perform_account_maintenance(Type tally_helper);
         ///@}
         ///@}

         std::mutex                             _pending_tx_mutex;
         vector< processed_transaction >        _pending_tx;
         fork_database                          _fork_db;

         /**
          *  Note: we can probably store blocks by block num rather than
          *  block id because after the undo window is past the block ID
          *  is no longer relevant and its number is irreversible.
          *
          *  During the "fork window" we can cache blocks in memory
          *  until the fork is resolved.  This should make maintaining
          *  the fork tree relatively simple.
          */
         block_database   _block_id_to_block;

         /**
          * Contains the set of ops that are in the process of being applied from
          * the current block.  It contains real and virtual operations in the
          * order they occur and is cleared after the applied_block signal is
          * emited.
          */
         vector<optional<operation_history_object> >  _applied_ops;

         uint32_t                          _current_block_num    = 0;
         uint16_t                          _current_trx_in_block = 0;
         uint16_t                          _current_op_in_trx    = 0;
         uint32_t                          _current_virtual_op   = 0;

         vector<uint64_t>                             _vote_tally_buffer;
         vector<uint64_t>                             _witness_count_histogram_buffer;
         vector<uint64_t>                             _committee_count_histogram_buffer;
         flat_map<sidechain_type, vector<uint64_t> >  _son_count_histogram_buffer = []{
            flat_map<sidechain_type, vector<uint64_t> >  son_count_histogram_buffer;
            for(const auto& active_sidechain_type : all_sidechain_types){
               son_count_histogram_buffer[active_sidechain_type] = vector<uint64_t>{};
            }
            return son_count_histogram_buffer;
         }();
         uint64_t                                     _total_voting_stake;

         flat_map<uint32_t,block_id_type>  _checkpoints;

         node_property_object              _node_property_object;

         /// Whether to update votes of standby witnesses and committee members when performing chain maintenance.
         /// Set it to true to provide accurate data to API clients, set to false to have better performance.
         bool                              _track_standby_votes = true;

         fc::hash_ctr_rng<secret_hash_type, 20> _random_number_generator;
         bool                              _slow_replays = false;

         /**
          * Whether database is successfully opened or not.
          *
          * The database is considered open when there's no exception
          * or assertion fail during database::open() method, and
          * database::close() has not been called, or failed during execution.
          */
         bool                              _opened = false;
         /// Tracks assets affected by bitshares-core issue #453 before hard fork #615 in one block
         flat_set<asset_id_type>           _issue_453_affected_assets;

         /// Pointers to core asset object and global objects who will have immutable addresses after created
         ///@{
         const asset_object*                    _p_core_asset_obj          = nullptr;
         const asset_dynamic_data_object*       _p_core_dynamic_data_obj   = nullptr;
         const global_property_object*          _p_global_prop_obj         = nullptr;
         const dynamic_global_property_object*  _p_dyn_global_prop_obj     = nullptr;
         const chain_property_object*           _p_chain_property_obj      = nullptr;
         const witness_schedule_object*         _p_witness_schedule_obj    = nullptr;
         ///@}
   };

   namespace detail
   {
       template<int... Is>
       struct seq { };

       template<int N, int... Is>
       struct gen_seq : gen_seq<N - 1, N - 1, Is...> { };

       template<int... Is>
       struct gen_seq<0, Is...> : seq<Is...> { };

       template<typename T, int... Is>
       void for_each(T&& t, const account_object& a, seq<Is...>)
       {
           auto l = { (std::get<Is>(t)(a), 0)... };
           (void)l;
       }
   }

} }
