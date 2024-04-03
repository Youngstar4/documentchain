#!/usr/bin/env python3
# Copyright (c) 2015-2016 The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""Utilities for manipulating blocks and transactions."""

from .mininode import *
from .script import CScript, OP_TRUE, OP_CHECKSIG

# Create a block (with regtest difficulty)
def create_block(hashprev, coinbase, nTime=None):
    block = CBlock()
    if nTime is None:
        import time
        block.nTime = int(time.time()+600)
    else:
        block.nTime = nTime
    block.hashPrevBlock = hashprev
    block.nBits = 0x207fffff # Will break after a difficulty adjustment...
    block.vtx.append(coinbase)
    block.hashMerkleRoot = block.calc_merkle_root()
    block.calc_sha256()
    return block

def serialize_script_num(value):
    r = bytearray(0)
    if value == 0:
        return r
    neg = value < 0
    absvalue = -value if neg else value
    while (absvalue):
        r.append(int(absvalue & 0xff))
        absvalue >>= 8
    if r[-1] & 0x80:
        r.append(0x80 if neg else 0)
    elif neg:
        r[-1] |= 0x80
    return r

# Create a coinbase transaction, assuming no miner fees.
# If pubkey is passed in, the coinbase output will be a P2PK output;
# otherwise an anyone-can-spend output.
def create_coinbase(height, pubkey = None, dip4_activated=False):
    coinbase = CTransaction()
    coinbase.vin.append(CTxIn(COutPoint(0, 0xffffffff),
                ser_string(serialize_script_num(height)), 0xffffffff))
    coinbaseoutput = CTxOut()
    coinbaseoutput.nValue = 500 * COIN
    halvings = int(height/150) # regtest
    coinbaseoutput.nValue >>= halvings
    if (pubkey != None):
        coinbaseoutput.scriptPubKey = CScript([pubkey, OP_CHECKSIG])
    else:
        coinbaseoutput.scriptPubKey = CScript([OP_TRUE])
    coinbase.vout = [ coinbaseoutput ]
    if dip4_activated:
        coinbase.nVersion = 3
        coinbase.nType = 5
        cbtx_payload = CCbTx(2, height, 0, 0)
        coinbase.vExtraPayload = cbtx_payload.serialize()
    coinbase.calc_sha256()
    return coinbase

# Create a transaction.
# If the scriptPubKey is not specified, make it anyone-can-spend.
def create_transaction(prevtx, n, sig, value, scriptPubKey=CScript()):
    tx = CTransaction()
    assert(n < len(prevtx.vout))
    tx.vin.append(CTxIn(COutPoint(prevtx.sha256, n), sig, 0xffffffff))
    tx.vout.append(CTxOut(value, scriptPubKey))
    tx.calc_sha256()
    return tx

def get_legacy_sigopcount_block(block, fAccurate=True):
    count = 0
    for tx in block.vtx:
        count += get_legacy_sigopcount_tx(tx, fAccurate)
    return count

def get_legacy_sigopcount_tx(tx, fAccurate=True):
    count = 0
    for i in tx.vout:
        count += i.scriptPubKey.GetSigOpCount(fAccurate)
    for j in tx.vin:
        # scriptSig might be of type bytes, so convert to CScript for the moment
        count += CScript(j.scriptSig).GetSigOpCount(fAccurate)
    return count

# Identical to GetMasternodePayment in C++ code
def get_masternode_payment(nHeight, blockValue):
    ret = int(blockValue / 5)

    nMNPIBlock = 350
    nMNPIPeriod = 10

    if nHeight > nMNPIBlock:
        ret += int(blockValue / 20)
    if nHeight > nMNPIBlock+(nMNPIPeriod* 1):
        ret += int(blockValue / 20)
    if nHeight > nMNPIBlock+(nMNPIPeriod* 2):
        ret += int(blockValue / 20)
    if nHeight > nMNPIBlock+(nMNPIPeriod* 3):
        ret += int(blockValue / 40)
    if nHeight > nMNPIBlock+(nMNPIPeriod* 4):
        ret += int(blockValue / 40)
    if nHeight > nMNPIBlock+(nMNPIPeriod* 5):
        ret += int(blockValue /100)
    if nHeight > nMNPIBlock+(nMNPIPeriod* 6):
        ret += int(blockValue /100)
    if nHeight > nMNPIBlock+(nMNPIPeriod* 7):
        ret += int(blockValue /100)
    if nHeight > nMNPIBlock+(nMNPIPeriod* 8):
        ret += int(blockValue /100)
    if nHeight > nMNPIBlock+(nMNPIPeriod* 9):
        ret += int(blockValue /100)
    if nHeight > nMNPIBlock+(nMNPIPeriod*10):
        ret += int(blockValue /100)
    if nHeight > nMNPIBlock+(nMNPIPeriod*11):
        ret += int(blockValue /100)
    if nHeight > nMNPIBlock+(nMNPIPeriod*12):
        ret += int(blockValue /100)
    if nHeight > nMNPIBlock+(nMNPIPeriod*13):
        ret += int(blockValue /100)
    if nHeight > nMNPIBlock+(nMNPIPeriod*14):
        ret += int(blockValue /100)

    return ret
