// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Copyright (c) 2018-2021 The Documentchain developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "primitives/transaction.h"

#include "hash.h"
#include "tinyformat.h"
#include "utilstrencodings.h"
#include "primitives/document.h"

std::string COutPoint::ToString() const
{
    return strprintf("COutPoint(%s, %u)", hash.ToString()/*.substr(0,10)*/, n);
}

std::string COutPoint::ToStringShort() const
{
    return strprintf("%s-%u", hash.ToString().substr(0,64), n);
}

CTxIn::CTxIn(COutPoint prevoutIn, CScript scriptSigIn, uint32_t nSequenceIn)
{
    prevout = prevoutIn;
    scriptSig = scriptSigIn;
    nSequence = nSequenceIn;
}

CTxIn::CTxIn(uint256 hashPrevTx, uint32_t nOut, CScript scriptSigIn, uint32_t nSequenceIn)
{
    prevout = COutPoint(hashPrevTx, nOut);
    scriptSig = scriptSigIn;
    nSequence = nSequenceIn;
}

std::string CTxIn::ToString() const
{
    std::string str;
    str += "CTxIn(";
    str += prevout.ToString();
    if (prevout.IsNull())
        str += strprintf(", coinbase %s", HexStr(scriptSig));
    else
        str += strprintf(", scriptSig=%s", HexStr(scriptSig).substr(0, 24));
    if (nSequence != SEQUENCE_FINAL)
        str += strprintf(", nSequence=%u", nSequence);
    str += ")";
    return str;
}

CTxOut::CTxOut(const CAmount& nValueIn, CScript scriptPubKeyIn, int nRoundsIn)
{
    nValue = nValueIn;
    scriptPubKey = scriptPubKeyIn;
    nRounds = nRoundsIn;
}

bool CTxOut::GetDocument(std::string& guid, std::string& indexhash, std::string& filehash, std::string& attrhash) const
{
    if (scriptPubKey[0] != OP_RETURN)
        return false;

    std::string rawDocument = HexStr(scriptPubKey.begin() + 2, scriptPubKey.end());
    /*
    revision v2, BIP34 disabled: rawDocument = "444d24..."
    revision v2, BIP34 enabled : rawDocument = "7d444d24..."
    The longer data in revision v2 contains the OP_PUSHDATA1 etc, skip it
    Alternative: rawDocument = ScriptToAsmStr(scriptPubKey) = "OP_RETURN 444d24..."
    */
    size_t pos = rawDocument.find("444d24");
    if (pos > 0)
        rawDocument = rawDocument.substr(pos, std::string::npos);

    if (rawDocument.substr(0, 6) != "444d24")
        return false;

    try { // incompatible or (maliciously) wrong data can cause an exception
        CDocument document(rawDocument);
        if (document.isvalid) {
            guid = document.guid;
            indexhash = document.indexhash.hash;
            filehash = document.filehash.hash;
            attrhash = document.attrhash.hash;
        }
        return document.isvalid;
    } catch (...) {
        return false;
    }
}

std::string CTxOut::ToString() const
{
    return strprintf("CTxOut(nValue=%d.%08d, scriptPubKey=%s)", nValue / COIN, nValue % COIN, HexStr(scriptPubKey).substr(0, 30));
}

CMutableTransaction::CMutableTransaction() : nVersion(CTransaction::CURRENT_VERSION), nType(TRANSACTION_NORMAL), nLockTime(0) {}
CMutableTransaction::CMutableTransaction(const CTransaction& tx) : nVersion(tx.nVersion), nType(tx.nType), vin(tx.vin), vout(tx.vout), nLockTime(tx.nLockTime), vExtraPayload(tx.vExtraPayload) {}

uint256 CMutableTransaction::GetHash() const
{
    return SerializeHash(*this);
}

std::string CMutableTransaction::ToString() const
{
    std::string str;
    str += strprintf("CMutableTransaction(hash=%s, ver=%d, type=%d, vin.size=%u, vout.size=%u, nLockTime=%u, vExtraPayload.size=%d)\n",
        GetHash().ToString().substr(0,10),
        nVersion,
        nType,
        vin.size(),
        vout.size(),
        nLockTime,
        vExtraPayload.size());
    for (unsigned int i = 0; i < vin.size(); i++)
        str += "    " + vin[i].ToString() + "\n";
    for (unsigned int i = 0; i < vout.size(); i++)
        str += "    " + vout[i].ToString() + "\n";
    return str;
}

uint256 CTransaction::ComputeHash() const
{
    return SerializeHash(*this);
}

/* For backward compatibility, the hash is initialized to 0. TODO: remove the need for this default constructor entirely. */
CTransaction::CTransaction() : nVersion(CTransaction::CURRENT_VERSION), nType(TRANSACTION_NORMAL), vin(), vout(), nLockTime(0), hash() {}
CTransaction::CTransaction(const CMutableTransaction &tx) : nVersion(tx.nVersion), nType(tx.nType), vin(tx.vin), vout(tx.vout), nLockTime(tx.nLockTime), vExtraPayload(tx.vExtraPayload), hash(ComputeHash()) {}
CTransaction::CTransaction(CMutableTransaction &&tx) : nVersion(tx.nVersion), nType(tx.nType), vin(std::move(tx.vin)), vout(std::move(tx.vout)), nLockTime(tx.nLockTime), vExtraPayload(tx.vExtraPayload), hash(ComputeHash()) {}

CAmount CTransaction::GetValueOut() const
{
    CAmount nValueOut = 0;
    for (std::vector<CTxOut>::const_iterator it(vout.begin()); it != vout.end(); ++it)
    {
        nValueOut += it->nValue;
        if (!MoneyRange(it->nValue) || !MoneyRange(nValueOut))
            throw std::runtime_error(std::string(__func__) + ": value out of range");
    }
    return nValueOut;
}

double CTransaction::ComputePriority(double dPriorityInputs, unsigned int nTxSize) const
{
    nTxSize = CalculateModifiedSize(nTxSize);
    if (nTxSize == 0) return 0.0;

    return dPriorityInputs / nTxSize;
}

unsigned int CTransaction::CalculateModifiedSize(unsigned int nTxSize) const
{
    // In order to avoid disincentivizing cleaning up the UTXO set we don't count
    // the constant overhead for each txin and up to 110 bytes of scriptSig (which
    // is enough to cover a compressed pubkey p2sh redemption) for priority.
    // Providing any more cleanup incentive than making additional inputs free would
    // risk encouraging people to create junk outputs to redeem later.
    if (nTxSize == 0)
        nTxSize = ::GetSerializeSize(*this, SER_NETWORK, PROTOCOL_VERSION);
    for (std::vector<CTxIn>::const_iterator it(vin.begin()); it != vin.end(); ++it)
    {
        unsigned int offset = 41U + std::min(110U, (unsigned int)it->scriptSig.size());
        if (nTxSize > offset)
            nTxSize -= offset;
    }
    return nTxSize;
}

unsigned int CTransaction::GetTotalSize() const
{
    return ::GetSerializeSize(*this, SER_NETWORK, PROTOCOL_VERSION);
}

std::string CTransaction::ToString() const
{
    std::string str;
    str += strprintf("CTransaction(hash=%s, ver=%d, type=%d, vin.size=%u, vout.size=%u, nLockTime=%u, vExtraPayload.size=%d)\n",
        GetHash().ToString().substr(0,10),
        nVersion,
        nType,
        vin.size(),
        vout.size(),
        nLockTime,
        vExtraPayload.size());
    for (unsigned int i = 0; i < vin.size(); i++)
        str += "    " + vin[i].ToString() + "\n";
    for (unsigned int i = 0; i < vout.size(); i++)
        str += "    " + vout[i].ToString() + "\n";
    return str;
}
