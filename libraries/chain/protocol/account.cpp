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
#include <graphene/chain/protocol/account.hpp>
#include <graphene/chain/hardfork.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <fc/io/raw.hpp>

namespace graphene { namespace chain {

/**
 * Names must comply with the following grammar (RFC 1035):
 * <domain> ::= <subdomain> | " "
 * <subdomain> ::= <label> | <subdomain> "." <label>
 * <label> ::= <letter> [ [ <ldh-str> ] <let-dig> ]
 * <ldh-str> ::= <let-dig-hyp> | <let-dig-hyp> <ldh-str>
 * <let-dig-hyp> ::= <let-dig> | "-"
 * <let-dig> ::= <letter> | <digit>
 *
 * Which is equivalent to the following:
 *
 * <domain> ::= <subdomain> | " "
 * <subdomain> ::= <label> ("." <label>)*
 * <label> ::= <letter> [ [ <let-dig-hyp>+ ] <let-dig> ]
 * <let-dig-hyp> ::= <let-dig> | "-"
 * <let-dig> ::= <letter> | <digit>
 *
 * I.e. a valid name consists of a dot-separated sequence
 * of one or more labels consisting of the following rules:
 *
 * - Each label is three characters or more
 * - Each label begins with a letter
 * - Each label ends with a letter or digit
 * - Each label contains only letters, digits or hyphens
 *
 * In addition we require the following:
 *
 * - All letters are lowercase
 * - Length is between (inclusive) GRAPHENE_MIN_ACCOUNT_NAME_LENGTH and GRAPHENE_MAX_ACCOUNT_NAME_LENGTH
 */
bool is_valid_name( const string& name )
{ try {
    const size_t len = name.size();

    /** this condition will prevent witnesses from including new names before this time, but
     * allow them after this time.   This check can be removed from the code after HARDFORK_385_TIME
     * has passed.
     */
    if( fc::time_point::now() < fc::time_point(HARDFORK_385_TIME) )
       FC_ASSERT( len >= 3 );

    if( len < GRAPHENE_MIN_ACCOUNT_NAME_LENGTH )
    {
          ilog( ".");
        return false;
    }

    if( len > GRAPHENE_MAX_ACCOUNT_NAME_LENGTH )
    {
          ilog( ".");
        return false;
    }

    size_t begin = 0;
    while( true )
    {
       size_t end = name.find_first_of( '.', begin );
       if( end == std::string::npos )
          end = len;
       if( (end - begin) < GRAPHENE_MIN_ACCOUNT_NAME_LENGTH )
       {
          idump( (name) (end)(len)(begin)(GRAPHENE_MAX_ACCOUNT_NAME_LENGTH) );
          return false;
       }
       switch( name[begin] )
       {
          case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h':
          case 'i': case 'j': case 'k': case 'l': case 'm': case 'n': case 'o': case 'p':
          case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w': case 'x':
          case 'y': case 'z':
             break;
          default:
          ilog( ".");
             return false;
       }
       switch( name[end-1] )
       {
          case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h':
          case 'i': case 'j': case 'k': case 'l': case 'm': case 'n': case 'o': case 'p':
          case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w': case 'x':
          case 'y': case 'z':
          case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7':
          case '8': case '9':
             break;
          default:
          ilog( ".");
             return false;
       }
       for( size_t i=begin+1; i<end-1; i++ )
       {
          switch( name[i] )
          {
             case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h':
             case 'i': case 'j': case 'k': case 'l': case 'm': case 'n': case 'o': case 'p':
             case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w': case 'x':
             case 'y': case 'z':
             case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7':
             case '8': case '9':
             case '-':
                break;
             default:
          ilog( ".");
                return false;
          }
       }
       if( end == len )
          break;
       begin = end+1;
    }

    // only dividend distribution accounts linked to a dividend asset can end in -dividend-distribution, and
    // these can only be created as a side-effect of the asset_update_dividend_operation
    if( boost::algorithm::ends_with(name, "-dividend-distribution") )
       return false;

    return true;
} FC_CAPTURE_AND_RETHROW( (name) ) }

bool is_cheap_name( const string& n )
{
   bool v = false;
   for( auto c : n )
   {
      if( c >= '0' && c <= '9' ) return true;
      if( c == '.' || c == '-' || c == '/' ) return true;
      switch( c )
      {
         case 'a':
         case 'e':
         case 'i':
         case 'o':
         case 'u':
         case 'y':
            v = true;
      }
   }
   if( !v )
      return true;
   return false;
}

void account_options::validate() const
{
   auto needed_witnesses = num_witness;
   auto needed_committee = num_committee;

   for( vote_id_type id : votes )
      if( id.type() == vote_id_type::witness && needed_witnesses )
         --needed_witnesses;
      else if ( id.type() == vote_id_type::committee && needed_committee )
         --needed_committee;

   FC_ASSERT( needed_witnesses == 0,
              "May not specify fewer witnesses than the number voted for.");
   FC_ASSERT( needed_committee == 0,
              "May not specify fewer committee members than the number voted for.");

   if ( extensions.value.num_son.valid() )
   {
      flat_map<sidechain_type, uint16_t> needed_sons = *extensions.value.num_son;

      for( vote_id_type id : votes )
         if ( id.type() == vote_id_type::son_bitcoin && needed_sons[sidechain_type::bitcoin] )
               --needed_sons[sidechain_type::bitcoin];
         else if ( id.type() == vote_id_type::son_hive && needed_sons[sidechain_type::hive] )
               --needed_sons[sidechain_type::hive];
         else if ( id.type() == vote_id_type::son_ethereum && needed_sons[sidechain_type::ethereum] )
               --needed_sons[sidechain_type::ethereum];

      FC_ASSERT( needed_sons[sidechain_type::bitcoin] == 0,
                "May not specify fewer Bitcoin SONs than the number voted for.");
      FC_ASSERT( needed_sons[sidechain_type::hive] == 0,
                "May not specify fewer Hive SONs than the number voted for.");
      FC_ASSERT( needed_sons[sidechain_type::ethereum] == 0,
                "May not specify fewer Ethereum SONs than the number voted for.");
   }
}

void affiliate_reward_distribution::validate() const
{
   // sum of weights must equal 100%
   uint32_t sum = 0;
   for( const auto& share : _dist )
   {
      FC_ASSERT( share.second > 0, "Must leave out affilates who receive 0%!" );
      FC_ASSERT( share.second <= GRAPHENE_100_PERCENT, "Can't pay out more than 100% per affiliate!" );
      sum += share.second;
      FC_ASSERT( sum <= GRAPHENE_100_PERCENT, "Can't pay out more than 100% total!" );
   }
   FC_ASSERT( sum == GRAPHENE_100_PERCENT, "Total affiliate distributions must cover 100%!" );
}

void affiliate_reward_distributions::validate() const
{
   FC_ASSERT( !_dists.empty(), "Empty affiliate reward distributions not allowed!" );
   for( const auto& dist: _dists )
      dist.second.validate();
}

share_type account_create_operation::calculate_fee( const fee_parameters_type& k )const
{
   auto core_fee_required = k.basic_fee;

   if( !is_cheap_name(name) )
      core_fee_required = k.premium_fee;

   // Authorities and vote lists can be arbitrarily large, so charge a data fee for big ones
   auto data_fee =  calculate_data_fee( fc::raw::pack_size(*this), k.price_per_kbyte ); 
   core_fee_required += data_fee;

   return core_fee_required;
}


void account_create_operation::validate()const
{
   FC_ASSERT( fee.amount >= 0 );
   FC_ASSERT( is_valid_name( name ) );
   FC_ASSERT( referrer_percent <= GRAPHENE_100_PERCENT );
   FC_ASSERT( owner.num_auths() != 0 );
   FC_ASSERT( owner.address_auths.size() == 0 );
   FC_ASSERT( active.num_auths() != 0 );
   FC_ASSERT( active.address_auths.size() == 0 );
   FC_ASSERT( !owner.is_impossible(), "cannot create an account with an imposible owner authority threshold" );
   FC_ASSERT( !active.is_impossible(), "cannot create an account with an imposible active authority threshold" );
   options.validate();
   if( extensions.value.owner_special_authority.valid() )
      validate_special_authority( *extensions.value.owner_special_authority );
   if( extensions.value.active_special_authority.valid() )
      validate_special_authority( *extensions.value.active_special_authority );
   if( extensions.value.buyback_options.valid() )
   {
      FC_ASSERT( !(extensions.value.owner_special_authority.valid()) );
      FC_ASSERT( !(extensions.value.active_special_authority.valid()) );
      FC_ASSERT( owner == authority::null_authority() );
      FC_ASSERT( active == authority::null_authority() );
      size_t n_markets = extensions.value.buyback_options->markets.size();
      FC_ASSERT( n_markets > 0 );
      for( const asset_id_type m : extensions.value.buyback_options->markets )
      {
         FC_ASSERT( m != extensions.value.buyback_options->asset_to_buy );
      }
   }
   if( extensions.value.affiliate_distributions.valid() )
      extensions.value.affiliate_distributions->validate();
}




share_type account_update_operation::calculate_fee( const fee_parameters_type& k )const
{
   auto core_fee_required = k.fee;  
   if( new_options )
      core_fee_required += calculate_data_fee( fc::raw::pack_size(*this), k.price_per_kbyte );
   return core_fee_required;
}

void account_update_operation::validate()const
{
   FC_ASSERT( account != GRAPHENE_TEMP_ACCOUNT );
   FC_ASSERT( fee.amount >= 0 );
   FC_ASSERT( account != account_id_type() );

   bool has_action = (
         owner.valid()
      || active.valid()
      || new_options.valid()
      || extensions.value.owner_special_authority.valid()
      || extensions.value.active_special_authority.valid()
      || extensions.value.update_last_voting_time.valid()
      );

   FC_ASSERT( has_action );

   if( owner )
   {
      FC_ASSERT( owner->num_auths() != 0 );
      FC_ASSERT( owner->address_auths.size() == 0 );
      FC_ASSERT( !owner->is_impossible(), "cannot update an account with an imposible owner authority threshold" );
   }
   if( active )
   {
      FC_ASSERT( active->num_auths() != 0 );
      FC_ASSERT( active->address_auths.size() == 0 );
      FC_ASSERT( !active->is_impossible(), "cannot update an account with an imposible active authority threshold" );
   }

   if( new_options )
      new_options->validate();
   if( extensions.value.owner_special_authority.valid() )
      validate_special_authority( *extensions.value.owner_special_authority );
   if( extensions.value.active_special_authority.valid() )
      validate_special_authority( *extensions.value.active_special_authority );
}

share_type account_upgrade_operation::calculate_fee(const fee_parameters_type& k) const
{
   if( upgrade_to_lifetime_member )
      return k.membership_lifetime_fee;
   return k.membership_annual_fee;
}


void account_upgrade_operation::validate() const
{
   FC_ASSERT( fee.amount >= 0 );
}

void account_transfer_operation::validate()const
{
   FC_ASSERT( fee.amount >= 0 );
}


} } // graphene::chain

GRAPHENE_EXTERNAL_SERIALIZATION( /*not extern*/, graphene::chain::account_options )
GRAPHENE_EXTERNAL_SERIALIZATION( /*not extern*/, graphene::chain::account_create_operation::fee_parameters_type )
GRAPHENE_EXTERNAL_SERIALIZATION( /*not extern*/, graphene::chain::account_whitelist_operation::fee_parameters_type )
GRAPHENE_EXTERNAL_SERIALIZATION( /*not extern*/, graphene::chain::account_update_operation::fee_parameters_type )
GRAPHENE_EXTERNAL_SERIALIZATION( /*not extern*/, graphene::chain::account_upgrade_operation::fee_parameters_type )
GRAPHENE_EXTERNAL_SERIALIZATION( /*not extern*/, graphene::chain::account_transfer_operation::fee_parameters_type )
GRAPHENE_EXTERNAL_SERIALIZATION( /*not extern*/, graphene::chain::account_create_operation )
GRAPHENE_EXTERNAL_SERIALIZATION( /*not extern*/, graphene::chain::account_whitelist_operation )
GRAPHENE_EXTERNAL_SERIALIZATION( /*not extern*/, graphene::chain::account_update_operation )
GRAPHENE_EXTERNAL_SERIALIZATION( /*not extern*/, graphene::chain::account_upgrade_operation )
GRAPHENE_EXTERNAL_SERIALIZATION( /*not extern*/, graphene::chain::account_transfer_operation )
