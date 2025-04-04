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
#include <graphene/chain/hardfork.hpp>
#include <graphene/chain/proposal_evaluator.hpp>
#include <graphene/chain/proposal_object.hpp>
#include <graphene/chain/account_object.hpp>
#include <graphene/chain/son_proposal_object.hpp>
#include <graphene/chain/protocol/account.hpp>
#include <graphene/chain/protocol/fee_schedule.hpp>
#include <graphene/chain/protocol/tournament.hpp>
#include <graphene/chain/exceptions.hpp>
#include <graphene/chain/hardfork.hpp>

namespace graphene { namespace chain {

struct proposal_operation_hardfork_visitor
{
   typedef void result_type;
   const fc::time_point_sec block_time;

   proposal_operation_hardfork_visitor( const fc::time_point_sec bt ) : block_time(bt) {}

   template<typename T>
   void operator()(const T &v) const {}

   void operator()(const committee_member_update_global_parameters_operation &op) const {}

   void operator()(const graphene::chain::tournament_payout_operation &o) const {
      // TODO: move check into tournament_payout_operation::validate after HARDFORK_999_TIME
      FC_ASSERT( block_time < HARDFORK_999_TIME, "Not allowed!" );
   }

   void operator()(const graphene::chain::asset_settle_cancel_operation &o) const {
      // TODO: move check into asset_settle_cancel_operation::validate after HARDFORK_999_TIME
      FC_ASSERT( block_time < HARDFORK_999_TIME, "Not allowed!" );
   }

   void operator()(const graphene::chain::account_create_operation &aco) const {
      // TODO: remove after HARDFORK_999_TIME
      if (block_time < HARDFORK_999_TIME)
         FC_ASSERT( !aco.extensions.value.affiliate_distributions.valid(), "Affiliate reward distributions not allowed yet" );
   }

   void operator()(const sport_create_operation &v) const {
       FC_ASSERT( block_time >= HARDFORK_1000_TIME, "sport_create_operation not allowed yet!" );
   }

   void operator()(const sport_update_operation &v) const {
       FC_ASSERT( block_time >= HARDFORK_1000_TIME, "sport_update_operation not allowed yet!" );
   }

   void operator()(const sport_delete_operation &v) const {
       FC_ASSERT( block_time >= HARDFORK_1000_TIME, "sport_delete_operation not allowed yet!" );
   }

   void operator()(const event_group_create_operation &v) const {
       FC_ASSERT( block_time >= HARDFORK_1000_TIME, "event_group_create_operation not allowed yet!" );
   }

   void operator()(const event_group_update_operation &v) const {
       FC_ASSERT( block_time >= HARDFORK_1000_TIME, "event_group_update_operation not allowed yet!" );
   }

   void operator()(const event_group_delete_operation &v) const {
       FC_ASSERT( block_time >= HARDFORK_1000_TIME, "event_group_delete_operation not allowed yet!" );
   }

   void operator()(const event_create_operation &v) const {
       FC_ASSERT( block_time >= HARDFORK_1000_TIME, "event_create_operation not allowed yet!" );
   }

   void operator()(const event_update_operation &v) const {
       FC_ASSERT( block_time >= HARDFORK_1000_TIME, "event_update_operation not allowed yet!" );
   }

   void operator()(const betting_market_rules_create_operation &v) const {
       FC_ASSERT( block_time >= HARDFORK_1000_TIME, "betting_market_rules_create_operation not allowed yet!" );
   }

   void operator()(const betting_market_rules_update_operation &v) const {
       FC_ASSERT( block_time >= HARDFORK_1000_TIME, "betting_market_rules_update_operation not allowed yet!" );
   }

   void operator()(const betting_market_group_create_operation &v) const {
       FC_ASSERT( block_time >= HARDFORK_1000_TIME, "betting_market_group_create_operation not allowed yet!" );
   }

   void operator()(const betting_market_create_operation &v) const {
       FC_ASSERT( block_time >= HARDFORK_1000_TIME, "betting_market_create_operation not allowed yet!" );
   }

   void operator()(const bet_place_operation &v) const {
       FC_ASSERT( block_time >= HARDFORK_1000_TIME, "bet_place_operation not allowed yet!" );
   }

   void operator()(const betting_market_group_resolve_operation &v) const {
       FC_ASSERT( block_time >= HARDFORK_1000_TIME, "betting_market_group_resolve_operation not allowed yet!" );
   }

   void operator()(const betting_market_group_cancel_unmatched_bets_operation &v) const {
       FC_ASSERT( block_time >= HARDFORK_1000_TIME, "betting_market_group_cancel_unmatched_bets_operation not allowed yet!" );
   }

   void operator()(const bet_cancel_operation &v) const {
       FC_ASSERT( block_time >= HARDFORK_1000_TIME, "bet_cancel_operation not allowed yet!" );
   }

   void operator()(const betting_market_group_update_operation &v) const {
       FC_ASSERT( block_time >= HARDFORK_1000_TIME, "betting_market_group_update_operation not allowed yet!" );
   }

   void operator()(const betting_market_update_operation &v) const {
       FC_ASSERT( block_time >= HARDFORK_1000_TIME, "betting_market_update_operation not allowed yet!" );
   }

   void operator()(const event_update_status_operation &v) const {
       FC_ASSERT( block_time >= HARDFORK_1000_TIME, "event_update_status_operation not allowed yet!" );
   }

   void operator()(const vesting_balance_create_operation &vbco) const {
      if(block_time < HARDFORK_GPOS_TIME)
         FC_ASSERT( vbco.balance_type == vesting_balance_type::normal, "balance_type in vesting create not allowed yet!" );
   }

   void operator()(const custom_permission_create_operation &v) const {
       FC_ASSERT( block_time >= HARDFORK_NFT_TIME, "custom_permission_create_operation not allowed yet!" );
   }

   void operator()(const custom_permission_update_operation &v) const {
       FC_ASSERT( block_time >= HARDFORK_NFT_TIME, "custom_permission_update_operation not allowed yet!" );
   }

   void operator()(const custom_permission_delete_operation &v) const {
       FC_ASSERT( block_time >= HARDFORK_NFT_TIME, "custom_permission_delete_operation not allowed yet!" );
   }

   void operator()(const custom_account_authority_create_operation &v) const {
       FC_ASSERT( block_time >= HARDFORK_NFT_TIME, "custom_account_authority_create_operation not allowed yet!" );
   }

   void operator()(const custom_account_authority_update_operation &v) const {
       FC_ASSERT( block_time >= HARDFORK_NFT_TIME, "custom_account_authority_update_operation not allowed yet!" );
   }

   void operator()(const custom_account_authority_delete_operation &v) const {
       FC_ASSERT( block_time >= HARDFORK_NFT_TIME, "custom_account_authority_delete_operation not allowed yet!" );
   }

   void operator()(const offer_operation &v) const {
       FC_ASSERT( block_time >= HARDFORK_NFT_TIME, "offer_operation not allowed yet!" );
   }

   void operator()(const bid_operation &v) const {
       FC_ASSERT( block_time >= HARDFORK_NFT_TIME, "bid_operation not allowed yet!" );
   }

   void operator()(const cancel_offer_operation &v) const {
       FC_ASSERT( block_time >= HARDFORK_NFT_TIME, "cancel_offer_operation not allowed yet!" );
   }

   void operator()(const finalize_offer_operation &v) const {
       FC_ASSERT( block_time >= HARDFORK_NFT_TIME, "finalize_offer_operation not allowed yet!" );
   }

   void operator()(const nft_metadata_create_operation &v) const {
       FC_ASSERT( block_time >= HARDFORK_NFT_TIME, "nft_metadata_create_operation not allowed yet!" );
   }

   void operator()(const nft_metadata_update_operation &v) const {
       FC_ASSERT( block_time >= HARDFORK_NFT_TIME, "nft_metadata_update_operation not allowed yet!" );
   }

   void operator()(const nft_mint_operation &v) const {
       FC_ASSERT( block_time >= HARDFORK_NFT_TIME, "nft_mint_operation not allowed yet!" );
   }

   void operator()(const nft_safe_transfer_from_operation &v) const {
       FC_ASSERT( block_time >= HARDFORK_NFT_TIME, "nft_safe_transfer_from_operation not allowed yet!" );
   }

   void operator()(const nft_approve_operation &v) const {
       FC_ASSERT( block_time >= HARDFORK_NFT_TIME, "nft_approve_operation not allowed yet!" );
   }

   void operator()(const nft_set_approval_for_all_operation &v) const {
       FC_ASSERT( block_time >= HARDFORK_NFT_TIME, "nft_set_approval_for_all_operation not allowed yet!" );
   }

   void operator()(const account_role_create_operation &v) const {
       FC_ASSERT( block_time >= HARDFORK_NFT_TIME, "account_role_create_operation not allowed yet!" );
   }

   void operator()(const account_role_update_operation &v) const {
       FC_ASSERT( block_time >= HARDFORK_NFT_TIME, "account_role_update_operation not allowed yet!" );
   }

   void operator()(const account_role_delete_operation &v) const {
       FC_ASSERT( block_time >= HARDFORK_NFT_TIME, "account_role_delete_operation not allowed yet!" );
   }

   void operator()(const nft_lottery_token_purchase_operation &v) const {
       FC_ASSERT( block_time >= HARDFORK_NFT_TIME, "nft_lottery_token_purchase_operation not allowed yet!" );
   }

   void operator()(const nft_lottery_reward_operation &v) const {
       FC_ASSERT( block_time >= HARDFORK_NFT_TIME, "nft_lottery_reward_operation not allowed yet!" );
   }

   void operator()(const nft_lottery_end_operation &v) const {
       FC_ASSERT( block_time >= HARDFORK_NFT_TIME, "nft_lottery_end_operation not allowed yet!" );
   }

   void operator()(const son_create_operation &v) const {
      FC_ASSERT( block_time >= HARDFORK_SON_TIME, "son_create_operation not allowed yet!" );
   }

   void operator()(const son_update_operation &v) const {
      FC_ASSERT( block_time >= HARDFORK_SON_TIME, "son_update_operation not allowed yet!" );
   }

   void operator()(const son_deregister_operation &v) const {
      FC_ASSERT( block_time >= HARDFORK_SON_TIME, "son_deregister_operation not allowed yet!" );
   }

   void operator()(const son_heartbeat_operation &v) const {
      FC_ASSERT( block_time >= HARDFORK_SON_TIME, "son_heartbeat_operation not allowed yet!" );
   }

   void operator()(const son_report_down_operation &v) const {
      FC_ASSERT( block_time >= HARDFORK_SON_TIME, "son_report_down_operation not allowed yet!" );
   }

   void operator()(const son_maintenance_operation &v) const {
      FC_ASSERT( block_time >= HARDFORK_SON_TIME, "son_maintenance_operation not allowed yet!" );
   }

   // loop and self visit in proposals
   void operator()(const proposal_create_operation &v) const {
      for (const op_wrapper &op : v.proposed_ops)
         op.op.visit(*this);
   }
};

void son_hardfork_visitor::operator()( const son_deregister_operation &v )
{
   db.create<son_proposal_object>([&]( son_proposal_object& son_prop ) {
      son_prop.proposal_type = son_proposal_type::son_deregister_proposal;
      son_prop.proposal_id = prop_id;
      son_prop.son_id = v.son_id;
   });
}

void son_hardfork_visitor::operator()( const son_report_down_operation &v )
{
   db.create<son_proposal_object>([&]( son_proposal_object& son_prop ) {
      son_prop.proposal_type = son_proposal_type::son_report_down_proposal;
      son_prop.proposal_id = prop_id;
      son_prop.son_id = v.son_id;
   });
}

void_result proposal_create_evaluator::do_evaluate( const proposal_create_operation& o )
{ try {
   const database& d = db();
   auto block_time = d.head_block_time();

   proposal_operation_hardfork_visitor vtor( block_time );
   vtor( o );

   const auto& global_parameters = d.get_global_properties().parameters;

   FC_ASSERT( o.expiration_time > block_time, "Proposal has already expired on creation." );
   FC_ASSERT( o.expiration_time <= block_time + global_parameters.maximum_proposal_lifetime,
              "Proposal expiration time is too far in the future.");
   FC_ASSERT( !o.review_period_seconds || fc::seconds(*o.review_period_seconds) < (o.expiration_time - block_time),
              "Proposal review period must be less than its overall lifetime." );

   {
      // If we're dealing with the committee authority, make sure this transaction has a sufficient review period.
      flat_set<account_id_type> auths;
      vector<authority> other;
      for( auto& op : o.proposed_ops )
      {
         operation_get_required_authorities( op.op, auths, auths, other, true );
      }

      FC_ASSERT( other.size() == 0 ); // TODO: what about other??? 

      if( auths.find(GRAPHENE_COMMITTEE_ACCOUNT) != auths.end() )
      {
         GRAPHENE_ASSERT(
            o.review_period_seconds.valid(),
            proposal_create_review_period_required,
            "Review period not given, but at least ${min} required",
            ("min", global_parameters.committee_proposal_review_period)
         );
         GRAPHENE_ASSERT(
            *o.review_period_seconds >= global_parameters.committee_proposal_review_period,
            proposal_create_review_period_insufficient,
            "Review period of ${t} specified, but at least ${min} required",
            ("t", *o.review_period_seconds)
            ("min", global_parameters.committee_proposal_review_period)
         );
      }
   }

   for (const op_wrapper& op : o.proposed_ops)
      _proposed_trx.operations.push_back(op.op);
   _proposed_trx.validate();

   return void_result();
} FC_CAPTURE_AND_RETHROW( (o) ) }

object_id_type proposal_create_evaluator::do_apply( const proposal_create_operation& o )
{ try {
   database& d = db();
   auto chain_time = d.head_block_time();

   const proposal_object& proposal = d.create<proposal_object>( [&o, this, chain_time](proposal_object& proposal) {
      _proposed_trx.expiration = o.expiration_time;
      proposal.proposed_transaction = _proposed_trx;
      proposal.proposer = o.fee_paying_account;
      proposal.expiration_time = o.expiration_time;
      if( o.review_period_seconds )
         proposal.review_period_time = o.expiration_time - *o.review_period_seconds;

      //Populate the required approval sets
      flat_set<account_id_type> required_active;
      vector<authority> other;
      
      // TODO: consider caching values from evaluate?
      for( auto& op : _proposed_trx.operations )
         operation_get_required_authorities( op, required_active, proposal.required_owner_approvals, other, true);

      //All accounts which must provide both owner and active authority should be omitted from the active authority set;
      //owner authority approval implies active authority approval.
      std::set_difference(required_active.begin(), required_active.end(),
                          proposal.required_owner_approvals.begin(), proposal.required_owner_approvals.end(),
                          std::inserter(proposal.required_active_approvals, proposal.required_active_approvals.begin()));
   });

   son_hardfork_visitor son_vtor(d, proposal.id);
   for(auto& op: o.proposed_ops)
   {
      op.op.visit(son_vtor);
   }

   return proposal.id;
} FC_CAPTURE_AND_RETHROW( (o) ) }

void_result proposal_update_evaluator::do_evaluate( const proposal_update_operation& o )
{ try {
   database& d = db();

   _proposal = &o.proposal(d);

   if( _proposal->review_period_time && d.head_block_time() >= *_proposal->review_period_time )
      FC_ASSERT( o.active_approvals_to_add.empty() && o.owner_approvals_to_add.empty(),
                 "This proposal is in its review period. No new approvals may be added." );

   for( account_id_type id : o.active_approvals_to_remove )
   {
      FC_ASSERT( _proposal->available_active_approvals.find(id) != _proposal->available_active_approvals.end(),
                 "", ("id", id)("available", _proposal->available_active_approvals) );
   }
   for( account_id_type id : o.owner_approvals_to_remove )
   {
      FC_ASSERT( _proposal->available_owner_approvals.find(id) != _proposal->available_owner_approvals.end(),
                 "", ("id", id)("available", _proposal->available_owner_approvals) );
   }

   return void_result();
} FC_CAPTURE_AND_RETHROW( (o) ) }

void_result proposal_update_evaluator::do_apply(const proposal_update_operation& o)
{ try {
   database& d = db();

   // Potential optimization: if _executed_proposal is true, we can skip the modify step and make push_proposal skip
   // signature checks. This isn't done now because I just wrote all the proposals code, and I'm not yet 100% sure the
   // required approvals are sufficient to authorize the transaction.
   d.modify(*_proposal, [&o, &d](proposal_object& p) {
      p.available_active_approvals.insert(o.active_approvals_to_add.begin(), o.active_approvals_to_add.end());
      p.available_owner_approvals.insert(o.owner_approvals_to_add.begin(), o.owner_approvals_to_add.end());
      for( account_id_type id : o.active_approvals_to_remove )
         p.available_active_approvals.erase(id);
      for( account_id_type id : o.owner_approvals_to_remove )
         p.available_owner_approvals.erase(id);
      for( const auto& id : o.key_approvals_to_add )
         p.available_key_approvals.insert(id);
      for( const auto& id : o.key_approvals_to_remove )
         p.available_key_approvals.erase(id);
   });

   // If the proposal has a review period, don't bother attempting to authorize/execute it.
   // Proposals with a review period may never be executed except at their expiration.
   if( _proposal->review_period_time )
      return void_result();

   if( _proposal->is_authorized_to_execute(d) )
   {
      // All required approvals are satisfied. Execute!
      _executed_proposal = true;
      try {
         _processed_transaction = d.push_proposal(*_proposal);
      } catch(fc::exception& e) {
         d.modify(*_proposal, [&e](proposal_object& p) {
            p.fail_reason = e.to_string(fc::log_level(fc::log_level::all));
         });
         wlog("Proposed transaction ${id} failed to apply once approved with exception:\n----\n${reason}\n----\nWill try again when it expires.",
              ("id", o.proposal)("reason", e.to_detail_string()));
         _proposal_failed = true;
      }
   }

   return void_result();
} FC_CAPTURE_AND_RETHROW( (o) ) }

void_result proposal_delete_evaluator::do_evaluate(const proposal_delete_operation& o)
{ try {
   database& d = db();

   _proposal = &o.proposal(d);

   auto required_approvals = o.using_owner_authority? &_proposal->required_owner_approvals
                                                    : &_proposal->required_active_approvals;
   FC_ASSERT( required_approvals->find(o.fee_paying_account) != required_approvals->end(),
              "Provided authority is not authoritative for this proposal.",
              ("provided", o.fee_paying_account)("required", *required_approvals));

   return void_result();
} FC_CAPTURE_AND_RETHROW( (o) ) }

void_result proposal_delete_evaluator::do_apply(const proposal_delete_operation& o)
{ try {
   db().remove(*_proposal);

   return void_result();
} FC_CAPTURE_AND_RETHROW( (o) ) }

} } // graphene::chain
