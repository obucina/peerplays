/*
 * Copyright (c) 2019 BitShares Blockchain Foundation, and contributors.
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

#include <graphene/chain/protocol/balance.hpp>
#include <graphene/chain/protocol/buyback.hpp>
#include <graphene/chain/protocol/fba.hpp>
#include <graphene/chain/protocol/fee_schedule.hpp>
#include <graphene/chain/protocol/vesting.hpp>
#include <graphene/chain/protocol/chain_parameters.hpp>

#include <fc/io/raw.hpp>

GRAPHENE_EXTERNAL_SERIALIZATION( /*not extern*/, graphene::chain::balance_claim_operation )
GRAPHENE_EXTERNAL_SERIALIZATION( /*not extern*/, graphene::chain::buyback_account_options )
GRAPHENE_EXTERNAL_SERIALIZATION( /*not extern*/, graphene::chain::fba_distribute_operation )

GRAPHENE_EXTERNAL_SERIALIZATION( /*not extern*/, graphene::chain::vesting_balance_create_operation::fee_parameters_type )
GRAPHENE_EXTERNAL_SERIALIZATION( /*not extern*/, graphene::chain::vesting_balance_withdraw_operation::fee_parameters_type )
GRAPHENE_EXTERNAL_SERIALIZATION( /*not extern*/, graphene::chain::vesting_balance_create_operation )
GRAPHENE_EXTERNAL_SERIALIZATION( /*not extern*/, graphene::chain::vesting_balance_withdraw_operation )

GRAPHENE_EXTERNAL_SERIALIZATION( /*not extern*/, graphene::chain::chain_parameters )
