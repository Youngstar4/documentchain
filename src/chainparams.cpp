// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin Core developers
// Copyright (c) 2014-2021 The Dash Core developers
// Copyright (c) 2018-2022 The Documentchain developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <chainparams.h>
#include <consensus/merkle.h>

#include <tinyformat.h>
#include <util.h>
#include <utilstrencodings.h>

#include <arith_uint256.h>

#include <assert.h>

#include <chainparamsseeds.h>

static CBlock CreateGenesisBlock(const char* pszTimestamp, const CScript& genesisOutputScript, uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

static CBlock CreateDevNetGenesisBlock(const uint256 &prevBlockHash, const std::string& devNetName, uint32_t nTime, uint32_t nNonce, uint32_t nBits, const CAmount& genesisReward)
{
    assert(!devNetName.empty());

    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    // put height (BIP34) and devnet name into coinbase
    txNew.vin[0].scriptSig = CScript() << 1 << std::vector<unsigned char>(devNetName.begin(), devNetName.end());
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = CScript() << OP_RETURN;

    CBlock genesis;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.nVersion = 4;
    genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));
    genesis.hashPrevBlock = prevBlockHash;
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

/**
 * Build the genesis block. Note that the output of its generation
 * transaction cannot be spent since it did not originally exist in the
 * database.
 *
 * CBlock(hash=00000ffd590b14, ver=1, hashPrevBlock=00000000000000, hashMerkleRoot=e0028e, nTime=1390095618, nBits=1e0ffff0, nNonce=28917698, vtx=1)
 *   CTransaction(hash=e0028e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
 *     CTxIn(COutPoint(000000, -1), coinbase 04ffff001d01044c5957697265642030392f4a616e2f3230313420546865204772616e64204578706572696d656e7420476f6573204c6976653a204f76657273746f636b2e636f6d204973204e6f7720416363657074696e6720426974636f696e73)
 *     CTxOut(nValue=50.00000000, scriptPubKey=0xA9037BAC7050C479B121CF)
 *   vMerkleTree: e0028e
 */
static CBlock CreateGenesisBlock(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    const char* pszTimestamp = "26 Aug 2018. Document Chain - The Blockchain for your Document Management System";
    const CScript genesisOutputScript = CScript() << ParseHex("045de9ebe4e50d9b595ac6ba52d79836c9d5b3fba54c0cd1078221df2d94866e40a1cb5939dc4c465b96fa759fec6ce0f6a70bf76b6bd517c068d0cca0819f71ca") << OP_CHECKSIG;
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward);
}


void CChainParams::UpdateVersionBitsParameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout, int64_t nWindowSize, int64_t nThresholdStart, int64_t nThresholdMin, int64_t nFalloffCoeff)
{
    consensus.vDeployments[d].nStartTime = nStartTime;
    consensus.vDeployments[d].nTimeout = nTimeout;
    if (nWindowSize != -1) {
            consensus.vDeployments[d].nWindowSize = nWindowSize;
    }
    if (nThresholdStart != -1) {
        consensus.vDeployments[d].nThresholdStart = nThresholdStart;
    }
    if (nThresholdMin != -1) {
        consensus.vDeployments[d].nThresholdMin = nThresholdMin;
    }
    if (nFalloffCoeff != -1) {
        consensus.vDeployments[d].nFalloffCoeff = nFalloffCoeff;
    }
}

void CChainParams::UpdateDIP3Parameters(int nActivationHeight, int nEnforcementHeight)
{
    consensus.DIP0003Height = nActivationHeight;
    consensus.DIP0003EnforcementHeight = nEnforcementHeight;
}

void CChainParams::UpdateDIP8Parameters(int nActivationHeight)
{
    consensus.DIP0008Height = nActivationHeight;
}

void CChainParams::UpdateBudgetParameters(int nMasternodePaymentsStartBlock, int nBudgetPaymentsStartBlock, int nSuperblockStartBlock)
{
    consensus.nMasternodePaymentsStartBlock = nMasternodePaymentsStartBlock;
    consensus.nBudgetPaymentsStartBlock = nBudgetPaymentsStartBlock;
    consensus.nSuperblockStartBlock = nSuperblockStartBlock;
}

void CChainParams::UpdateSubsidyAndDiffParams(int nMinimumDifficultyBlocks, int nHighSubsidyBlocks, int nHighSubsidyFactor)
{
    consensus.nMinimumDifficultyBlocks = nMinimumDifficultyBlocks;
    consensus.nHighSubsidyBlocks = nHighSubsidyBlocks;
    consensus.nHighSubsidyFactor = nHighSubsidyFactor;
}

void CChainParams::UpdateLLMQChainLocks(Consensus::LLMQType llmqType) {
    consensus.llmqTypeChainLocks = llmqType;
}

void CChainParams::UpdateLLMQInstantSend(Consensus::LLMQType llmqType) {
    consensus.llmqTypeInstantSend = llmqType;
}

void CChainParams::UpdateLLMQTestParams(int size, int threshold) {
    auto& params = consensus.llmqs.at(Consensus::LLMQ_TEST);
    params.size = size;
    params.minSize = threshold;
    params.threshold = threshold;
    params.dkgBadVotesThreshold = threshold;
}

void CChainParams::UpdateLLMQDevnetParams(int size, int threshold)
{
    auto& params = consensus.llmqs.at(Consensus::LLMQ_DEVNET);
    params.size = size;
    params.minSize = threshold;
    params.threshold = threshold;
    params.dkgBadVotesThreshold = threshold;
}

static CBlock FindDevNetGenesisBlock(const CBlock &prevBlock, const CAmount& reward)
{
    std::string devNetName = gArgs.GetDevNetName();
    assert(!devNetName.empty());

    CBlock block = CreateDevNetGenesisBlock(prevBlock.GetHash(), devNetName.c_str(), prevBlock.nTime + 1, 0, prevBlock.nBits, reward);

    arith_uint256 bnTarget;
    bnTarget.SetCompact(block.nBits);

    for (uint32_t nNonce = 0; nNonce < UINT32_MAX; nNonce++) {
        block.nNonce = nNonce;

        uint256 hash = block.GetHash();
        if (UintToArith256(hash) <= bnTarget)
            return block;
    }

    // This is very unlikely to happen as we start the devnet with a very low difficulty. In many cases even the first
    // iteration of the above loop will give a result already
    error("FindDevNetGenesisBlock: could not find devnet genesis block for %s", devNetName);
    assert(false);
}

// this one is for testing only
static Consensus::LLMQParams llmq_test = { // llmq5_60
        .type = Consensus::LLMQ_TEST,
        .name = "llmq_test",
        .size = 5,
        .minSize = 4,
        .threshold = 3,

        .dkgInterval = 20, // one DKG every 2 hours
        .dkgPhaseBlocks = 2,
        .dkgMiningWindowStart = 10, // dkgPhaseBlocks * 5 = after finalization
        .dkgMiningWindowEnd = 18,
        .dkgBadVotesThreshold = 3,

        .signingActiveQuorumCount = 2, // just a few ones to allow easier testing

        .keepOldConnections = 3, // min signingActiveQuorumCount + 1
        .recoveryMembers = 5,
};
// this one is for devnets only
static Consensus::LLMQParams llmq_devnet = { // llmq10_60
        .type = Consensus::LLMQ_DEVNET,
        .name = "llmq_devnet",
        .size = 10,
        .minSize = 8,
        .threshold = 6,

        .dkgInterval = 20, // one DKG every 2 hours
        .dkgPhaseBlocks = 2,
        .dkgMiningWindowStart = 10, // dkgPhaseBlocks * 5 = after finalization
        .dkgMiningWindowEnd = 18,
        .dkgBadVotesThreshold = 7,

        .signingActiveQuorumCount = 3, // just a few ones to allow easier testing

        .keepOldConnections = 4, // min signingActiveQuorumCount + 1
        .recoveryMembers = 6,
};

static Consensus::LLMQParams llmq30_60 = {
        .type = Consensus::LLMQ_30_60,
        .name = "llmq_30_60",
        .size = 30,
        .minSize = 24,
        .threshold = 18,

        .dkgInterval = 20, // one DKG every 2 hours
        .dkgPhaseBlocks = 2,
        .dkgMiningWindowStart = 10, // dkgPhaseBlocks * 5 = after finalization
        .dkgMiningWindowEnd = 18,
        .dkgBadVotesThreshold = 22,

        .signingActiveQuorumCount = 12, // a full day worth of LLMQs

        .keepOldConnections = 13, // min signingActiveQuorumCount + 1
        .recoveryMembers = 16,
};

static Consensus::LLMQParams llmq150_60 = {
        .type = Consensus::LLMQ_150_60,
        .name = "llmq_150_60",
        .size = 150,
        .minSize = 120,
        .threshold = 90,

        .dkgInterval = 10 * 12, // one DKG every 12 hours
        .dkgPhaseBlocks = 4,
        .dkgMiningWindowStart = 20, // dkgPhaseBlocks * 5 = after finalization
        .dkgMiningWindowEnd = 28,
        .dkgBadVotesThreshold = 114,

        .signingActiveQuorumCount = 4, // two days worth of LLMQs,

        .keepOldConnections = 5, // min signingActiveQuorumCount + 1
        .recoveryMembers = 50,
};

// Used for deployment and min-proto-version signalling, so it needs a higher threshold
static Consensus::LLMQParams llmq150_85 = {
        .type = Consensus::LLMQ_150_85,
        .name = "llmq_150_85",
        .size = 150,
        .minSize = 135,
        .threshold = 120,

        .dkgInterval = 10 * 24, // one DKG every 24 hours
        .dkgPhaseBlocks = 4,
        .dkgMiningWindowStart = 20, // dkgPhaseBlocks * 5 = after finalization
        .dkgMiningWindowEnd = 48, // give it a larger mining window to make sure it is mined
        .dkgBadVotesThreshold = 114,

        .signingActiveQuorumCount = 4, // four days worth of LLMQs,

        .keepOldConnections = 5, // min signingActiveQuorumCount + 1
        .recoveryMembers = 50,
};

// Used for Platform
static Consensus::LLMQParams llmq100_67 = {
        .type = Consensus::LLMQ_100_67,
        .name = "llmq_100_67",
        .size = 100,
        .minSize = 80,
        .threshold = 67,

        .dkgInterval = 20, // one DKG every 2 hours
        .dkgPhaseBlocks = 2,
        .dkgMiningWindowStart = 10, // dkgPhaseBlocks * 5 = after finalization
        .dkgMiningWindowEnd = 18,
        .dkgBadVotesThreshold = 80,

        .signingActiveQuorumCount = 12, // a full day worth of LLMQs

        .keepOldConnections = 25,
        .recoveryMembers = 50,
};

/* Dash v0.17:
// this one is for testing only
static Consensus::LLMQParams llmq_test = {
        .type = Consensus::LLMQ_TEST,
        .name = "llmq_test",
        .size = 3,
        .minSize = 2,
        .threshold = 2,

        .dkgInterval = 24, // one DKG per hour
        .dkgPhaseBlocks = 2,
        .dkgMiningWindowStart = 10, // dkgPhaseBlocks * 5 = after finalization
        .dkgMiningWindowEnd = 18,
        .dkgBadVotesThreshold = 2,

        .signingActiveQuorumCount = 2, // just a few ones to allow easier testing

        .keepOldConnections = 3,
        .recoveryMembers = 3,
};

// this one is for testing only
static Consensus::LLMQParams llmq_test_v17 = {
        .type = Consensus::LLMQ_TEST_V17,
        .name = "llmq_test_v17",
        .size = 3,
        .minSize = 2,
        .threshold = 2,

        .dkgInterval = 24, // one DKG per hour
        .dkgPhaseBlocks = 2,
        .dkgMiningWindowStart = 10, // dkgPhaseBlocks * 5 = after finalization
        .dkgMiningWindowEnd = 18,
        .dkgBadVotesThreshold = 2,

        .signingActiveQuorumCount = 2, // just a few ones to allow easier testing

        .keepOldConnections = 3,
        .recoveryMembers = 3,
};

// this one is for devnets only
static Consensus::LLMQParams llmq_devnet = {
        .type = Consensus::LLMQ_DEVNET,
        .name = "llmq_devnet",
        .size = 10,
        .minSize = 7,
        .threshold = 6,

        .dkgInterval = 24, // one DKG per hour
        .dkgPhaseBlocks = 2,
        .dkgMiningWindowStart = 10, // dkgPhaseBlocks * 5 = after finalization
        .dkgMiningWindowEnd = 18,
        .dkgBadVotesThreshold = 7,

        .signingActiveQuorumCount = 3, // just a few ones to allow easier testing

        .keepOldConnections = 4,
        .recoveryMembers = 6,
};

static Consensus::LLMQParams llmq50_60 = {
        .type = Consensus::LLMQ_50_60,
        .name = "llmq_50_60",
        .size = 50,
        .minSize = 40,
        .threshold = 30,

        .dkgInterval = 24, // one DKG per hour
        .dkgPhaseBlocks = 2,
        .dkgMiningWindowStart = 10, // dkgPhaseBlocks * 5 = after finalization
        .dkgMiningWindowEnd = 18,
        .dkgBadVotesThreshold = 40,

        .signingActiveQuorumCount = 24, // a full day worth of LLMQs

        .keepOldConnections = 25,
        .recoveryMembers = 25,
};

static Consensus::LLMQParams llmq400_60 = {
        .type = Consensus::LLMQ_400_60,
        .name = "llmq_400_60",
        .size = 400,
        .minSize = 300,
        .threshold = 240,

        .dkgInterval = 24 * 12, // one DKG every 12 hours
        .dkgPhaseBlocks = 4,
        .dkgMiningWindowStart = 20, // dkgPhaseBlocks * 5 = after finalization
        .dkgMiningWindowEnd = 28,
        .dkgBadVotesThreshold = 300,

        .signingActiveQuorumCount = 4, // two days worth of LLMQs

        .keepOldConnections = 5,
        .recoveryMembers = 100,
};

// Used for deployment and min-proto-version signalling, so it needs a higher threshold
static Consensus::LLMQParams llmq400_85 = {
        .type = Consensus::LLMQ_400_85,
        .name = "llmq_400_85",
        .size = 400,
        .minSize = 350,
        .threshold = 340,

        .dkgInterval = 24 * 24, // one DKG every 24 hours
        .dkgPhaseBlocks = 4,
        .dkgMiningWindowStart = 20, // dkgPhaseBlocks * 5 = after finalization
        .dkgMiningWindowEnd = 48, // give it a larger mining window to make sure it is mined
        .dkgBadVotesThreshold = 300,

        .signingActiveQuorumCount = 4, // four days worth of LLMQs

        .keepOldConnections = 5,
        .recoveryMembers = 100,
};

// Used for Platform
static Consensus::LLMQParams llmq100_67 = {
        .type = Consensus::LLMQ_100_67,
        .name = "llmq_100_67",
        .size = 100,
        .minSize = 80,
        .threshold = 67,

        .dkgInterval = 24, // one DKG per hour
        .dkgPhaseBlocks = 2,
        .dkgMiningWindowStart = 10, // dkgPhaseBlocks * 5 = after finalization
        .dkgMiningWindowEnd = 18,
        .dkgBadVotesThreshold = 80,

        .signingActiveQuorumCount = 24, // a full day worth of LLMQs

        .keepOldConnections = 25,
        .recoveryMembers = 50,
};
*/

/**
 * Main network
 */
/**
 * What makes a good checkpoint block?
 * + Is surrounded by blocks with reasonable timestamps
 *   (no blocks before with a timestamp after, none after with
 *    timestamp before)
 * + Contains no strange transactions
 */


class CMainParams : public CChainParams {
public:
    CMainParams() {
        strNetworkID = "main";
        consensus.nSubsidyDecreaseStart = 44001;
        consensus.nMasternodePaymentsStartBlock = 5000; // Dash=100000;
        consensus.nMasternodePaymentsIncreaseBlock = 60000; // Dash: 158000
        consensus.nMasternodePaymentsIncreasePeriod = 7000; // ~29 days, Dash: 17280
        consensus.nInstantSendConfirmationsRequired = 6;
        consensus.nInstantSendKeepLock = 24;
        consensus.nBudgetPaymentsStartBlock = 100000000; // >1000 years: DMS does not use this yet
        consensus.nBudgetPaymentsCycleBlocks = 7200; // DMS: (60*24*30)/6; Dash: ~(60*24*30)/2.6, actual number of blocks per month is 200700 / 12 = 16725
        consensus.nBudgetPaymentsWindowBlocks = 100;
        consensus.nSuperblockStartBlock = 100000000; // >1000 years: DMS does not use superblocks, but the code is not removed yet because it could be of interest for other purposes
        consensus.nSuperblockStartHash = uint256S("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"); // - " -
        consensus.nSuperblockCycle = 7200; // DMS: (60*24*30)/6; Dash: ~(60*24*30)/2.6, actual number of blocks per month is 200700 / 12 = 16725
        consensus.nGovernanceMinQuorum = 10;
        consensus.nGovernanceFilterElements = 20000;
        consensus.nMasternodeMinimumConfirmations = 15;
        consensus.BIP34Height = 0; // 0=genesis block  // Bitcoin: 227931, Dash: 951;
        consensus.BIP34Hash = uint256S("0x00000af4a21d6e8daa4026a5eafc7132089a7dbb9d3921b12c4fa39b78c9a010");
        consensus.BIP65Height = 84100;
        consensus.BIP66Height = 84000;
        consensus.DIP0001Height = 143481; // May 18, 2020
        consensus.DIP0003Height = 237600; // getblockchaininfo dip0003 "since"
        consensus.DIP0003EnforcementHeight = 100000000;  // TODO : set after spork 15
        consensus.DIP0003EnforcementHash = uint256S("000000000000002d1734087b4c5afc3133e4e1c3e1a89218f62bcd9bb3d17f81");  // TODO : set after spork 15
        consensus.DIP0008Height = 100000000; // TODO, Dash: 1088640; // 00000000000000112e41e4b3afda8b233b8cc07c532d2eac5de097b68358c43e
        consensus.powLimit = uint256S("0000ffffff000000000000000000000000000000000000000000000000000000");
        consensus.nPowTargetTimespan = 24 * 60 * 60; // PoW calculation 24 * 60 * 60;  1 day
        consensus.nPowTargetSpacing = 6 * 60; // DMS 6 minutes (initial 4 minutes); Bitcoin 10 minutes; Dash 2.5 minutes
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.nPowNTSHeight = 4794; // see commit df04016c84ee09eb716605a962cf05064c7eea9f
        consensus.nRuleChangeActivationThreshold = 1916; // 95% of 2016
        consensus.nMinerConfirmationWindow = 2016; // 8,4 days, Bitcoin: nPowTargetTimespan / nPowTargetSpacing, relevant to BIP 9 soft forks, see params.h
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // Dash: January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // Dash: December 31, 2008

        // Deployment of BIP68, BIP112, and BIP113. Documentchain: active since block 241920
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1622505600; // Jun 1th, 2021
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1654041600;   // Jun 1th, 2022

        // Deployment of DIP0001, Documentchain: active since block 143481
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nStartTime = 1589536800; // May 15th, 2020
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nTimeout = 1592215200; // Jun 15th, 2020
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nWindowSize = 283;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nThresholdStart = 254; // 80% of 283

        // Deployment of BIP147. Documentchain: active since block 12096
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP147].bit = 2;
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP147].nStartTime = 1524477600; // Dash: Apr 23th, 2018
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP147].nTimeout = 1556013600; // Dash: Apr 23th, 2019
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP147].nWindowSize = 4032;
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP147].nThresholdStart = 3226; // 80% of 4032

        // Deployment of DIP0003. Documentchain: active since block 237600
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0003].bit = 3;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0003].nStartTime = 1625097600; // Jul 1th, 2021
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0003].nTimeout = 1656633600;   // Jul 1th, 2022
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0003].nWindowSize = 400;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0003].nThresholdStart = 320; // 80% of 400

        // Deployment of DIP0008 - TODO
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0008].bit = 4;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0008].nStartTime = 3000000000;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0008].nTimeout = 3300000000;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0008].nWindowSize = 4032;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0008].nThresholdStart = 3226; // 80% of 4032

        // Deployment of Block Reward Reallocation - TODO
        consensus.vDeployments[Consensus::DEPLOYMENT_REALLOC].bit = 5;
        consensus.vDeployments[Consensus::DEPLOYMENT_REALLOC].nStartTime = 3000000000;
        consensus.vDeployments[Consensus::DEPLOYMENT_REALLOC].nTimeout = 3300000000;
        consensus.vDeployments[Consensus::DEPLOYMENT_REALLOC].nWindowSize = 4032;
        consensus.vDeployments[Consensus::DEPLOYMENT_REALLOC].nThresholdStart = 3226; // 80% of 4032
        consensus.vDeployments[Consensus::DEPLOYMENT_REALLOC].nThresholdMin = 2420; // 60% of 4032
        consensus.vDeployments[Consensus::DEPLOYMENT_REALLOC].nFalloffCoeff = 5; // this corresponds to 10 periods

        // Deployment of DIP0020, DIP0021 and LLMQ_100_67 quorums - TODO
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0020].bit = 6;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0020].nStartTime = 3000000000;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0020].nTimeout = 3300000000;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0020].nWindowSize = 4032;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0020].nThresholdStart = 3226; // 80% of 4032
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0020].nThresholdMin = 2420; // 60% of 4032
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0020].nFalloffCoeff = 5; // this corresponds to 10 periods

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00000000000000000000000000000000000000000000000000000562b792bb20"); // = 5.921.544.780.576 total work, block 222876

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x00000003bd8ad5f5f5373342f9c2fddfb33f9e0344eb743521631875b596a20a"); // 222876

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        pchMessageStart[0] = 0x24;  // $
        pchMessageStart[1] = 0x44;  // D
        pchMessageStart[2] = 0x4d;  // M
        pchMessageStart[3] = 0x53;  // S
        nDefaultPort = 41319; // Ascii ^D ^M ^S = 4 13 19
        nPruneAfterHeight = 100000;

        genesis = CreateGenesisBlock(1535270400, 675612, 0x1e0ffff0, 1, 10 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();      
        assert(consensus.hashGenesisBlock == uint256S("0x00000af4a21d6e8daa4026a5eafc7132089a7dbb9d3921b12c4fa39b78c9a010"));
        assert(genesis.hashMerkleRoot == uint256S("0xa500729aa7e2874bdb829c2e1ea99ef3542b7c43d426bca2f6720b9a9688308a"));
		/* temp: create genesis with dmsd.exe
            printf("(CMainParams)\n");
            printf("hashMerkleRoot %s\n", genesis.hashMerkleRoot.ToString().c_str());
            printf("genesis.nBits 0x%x\n", genesis.nBits);
            for (genesis.nNonce = 0; ; genesis.nNonce++) {
                consensus.hashGenesisBlock = genesis.GetHash();
                uint256 hash = genesis.GetHash();
                if (UintToArith256(hash) <= UintToArith256(consensus.powLimit)) break;				
            }
            printf("hashGenesisBlock %s\n", consensus.hashGenesisBlock.ToString().c_str());
            printf("genesis.nNonce %d\n", genesis.nNonce);
            printf("genesis.nTime %d\n", genesis.nTime);
		*/

        vSeeds.clear();

        // DMS addresses start with 'D'
        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,30);
        // DMS script addresses start
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,15);
        // DMS private keys start
        base58Prefixes[SECRET_KEY]     = std::vector<unsigned char>(1,133);
        // DMS BIP32 pubkeys start with 'xpub' (Bitcoin and Dash defaults)
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x88, 0xB2, 0x1E};
        // DMS BIP32 prvkeys start with 'xprv' (Bitcoin and Dash defaults)
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x88, 0xAD, 0xE4};

        // DMS/Dash BIP44 coin type is '5'
        nExtCoinType = 5;

        // fixed seed nodes, use contrib/seeds/generate-seeds.py
        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        // long living quorum params
        /* TODO : v0.17 activate llmq30_60 on devnet, testnet and later on mainet via spork
           in v0.13 a never used llmq with too short interval 10 was defined
        consensus.llmqs[Consensus::LLMQ_30_60] = llmq30_60; 
        */
        consensus.llmqs[Consensus::LLMQ_150_60] = llmq150_60;
        consensus.llmqs[Consensus::LLMQ_150_85] = llmq150_85;
        consensus.llmqTypeChainLocks = Consensus::LLMQ_150_60;
        consensus.llmqTypeInstantSend = Consensus::LLMQ_30_60;
        consensus.llmqTypePlatform = Consensus::LLMQ_100_67;
        /* Dash v0.17 long living quorum params
        consensus.llmqs[Consensus::LLMQ_50_60] = llmq50_60;
        consensus.llmqs[Consensus::LLMQ_400_60] = llmq400_60;
        consensus.llmqs[Consensus::LLMQ_400_85] = llmq400_85;
        consensus.llmqs[Consensus::LLMQ_100_67] = llmq100_67;
        consensus.llmqTypeChainLocks = Consensus::LLMQ_400_60;
        consensus.llmqTypeInstantSend = Consensus::LLMQ_50_60;
        consensus.llmqTypePlatform = Consensus::LLMQ_100_67;
        */

        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fRequireRoutableExternalIP = true;
        fMineBlocksOnDemand = false;
        fAllowMultipleAddressesFromGroup = false;
        fAllowMultiplePorts = false;
        nLLMQConnectionRetryTimeout = 60;

        nPoolMinParticipants = 3;
        nPoolMaxParticipants = 20;
        nFulfilledRequestExpireTime = 25*60; // fulfilled requests expire in 25 min, Dash: 60*60

        vSporkAddresses = {"D6JKVPFgRhYHPDvuQ1ocwDBsk2bDrx8CXo"};
        nMinSporkKeys = 1;
        fBIP9CheckMasternodesUpgraded = true;

        checkpointData = {
            {
                {     0, uint256S("0x00000af4a21d6e8daa4026a5eafc7132089a7dbb9d3921b12c4fa39b78c9a010")}, // 2018-Aug-26
                {   133, uint256S("0x00003c1aa0d920b6a665b71fb8ebde8dff0426ed02483a1f3165a1b9533f339f")}, // 2018-Aug-27
                {  4600, uint256S("0x00003e6d58a4974170d74f8ccb55aca3afeca21fc6a25789c64a88ac3b44c239")}, // 2018-Sep-14
                { 15000, uint256S("0x0000024a04c4ac9eb28385fa0a18b6911b7ee22c64c3af476cd72bcf27fa136f")}, // 2018-Oct-31
                { 20013, uint256S("0x000000465d652bda5d5746ec620b0926de583a622011b61404af02fd07176236")}, // 2018-Nov-24
                { 26990, uint256S("0x00000ac7439f65441970eed887d5bad2ecf6a1bbde4f39a4a21f50aaa648d049")}, // 2018-Dec-25
                { 35993, uint256S("0x000000d3ea06a03deccce15641c113ce59bc9ed662d878eb1d971fb0478df9cc")}, // 2019-Feb-03
                { 46096, uint256S("0x000000a402d9343d7b70df237cfedadefbd002d18f95ce775d33956e7f7e4141")}, // 2019-Mar-19
                { 59098, uint256S("0x0000062c9eeac8249ff40512763846be367c9404c2ebded2646309bb30a28f4a")}, // 2019-May-15
                { 68050, uint256S("0x000005c0ed9cec8cd6c2a23c673b3047bf2f3c205d85ca3843fb8ec4b53c27c9")}, // 2019-Jun-23
                { 79403, uint256S("0x00000295f0e654ec4c7ec2e4051acaca0eda31ae2ca2d7a38e56a7875cf457dd")}, // 2019-Aug-12
                { 86998, uint256S("0x0000000d6ca0904b7ec849283a177b7a25b0ed3c23413c14d9e564bfa4d2514a")}, // 2019-Sep-14
                {100000, uint256S("0x00000095a23b6cc9ec9c0d7385bf4c3705220271d425a203af41163b4600e96d")}, // 2019-Nov-10
                {110034, uint256S("0x00000025fcd0421225ea0c8d5874b3040a81618c609f2419bce9d339a96faee7")}, // 2019-Dec-24
                {120004, uint256S("0x00000031fd82d41fa6fda29a405a55162304822be9b03beed79e47f25d3f3296")}, // 2020-Feb-06
                {127991, uint256S("0x00000042ec60e4badf5e3288b1dbb8da2a27af4b88b6ec299e7149586784ccfd")}, // 2020-Mar-12
                {135808, uint256S("0x00000025868ef0958009ba542ded2faf5242cd835445ca6c5556eff5a19161ab")}, // 2020-Apr-15
                {144997, uint256S("0x00000074c2b1520d116c447d2cf6170951642fc50487b300074d76952fc20a4d")}, // 2020-May-25
                {154972, uint256S("0x00000053c7c6c55129d6e9c9477ddcdbd351847d748ff77eff13dbc44bde58d4")}, // 2020-Jul-08
                {165993, uint256S("0x000000391480ecb91edb3bc72ed1a206c6640c7c8927c244536e0bc90c43b51f")}, // 2020-Aug-25
                {175594, uint256S("0x0000000ee3871b495f8f26714200b50dbe56fb48c9bde4a9b1a4b72ba5793c1c")}, // 2020-Oct-06
                {189933, uint256S("0x000000019cf3a28ce96ad31a446d0fe3fe796f80d1df0e315805daf8ec671b4e")}, // 2020-Dec-08
                {199995, uint256S("0x00000009b7ca8806f82b23aa8464967c4f60134941208daa69c320a0c4e98be8")}, // 2021-Jan-21
                {212723, uint256S("0x0000000540497ce3a503d7f546f5faafe4c28e0e7a15cd81074416428b57a114")}, // 2021-Mar-18
                {222876, uint256S("0x00000003bd8ad5f5f5373342f9c2fddfb33f9e0344eb743521631875b596a20a")}, // 2021-May-01
            }
        };

        chainTxData = ChainTxData{
            1619902162, // * UNIX timestamp of last known number of transactions
            387457,     // * total number of transactions between genesis and that timestamp
                        //   (the tx=... number in the UpdateTip debug.log lines)
            0.04        // * estimated number of transactions per second after that timestamp
        };
    }
};

/**
 * Testnet (v4)
 */
class CTestNetParams : public CChainParams {
public:
    CTestNetParams() {
        strNetworkID = "test";
        consensus.nSubsidyDecreaseStart = 44001;
        consensus.nMasternodePaymentsStartBlock = 5000;
        consensus.nMasternodePaymentsIncreaseBlock = 60000;
        consensus.nMasternodePaymentsIncreasePeriod = 7000;
        consensus.nInstantSendConfirmationsRequired = 6;
        consensus.nInstantSendKeepLock = 24;
        consensus.nBudgetPaymentsStartBlock = 100000000;
        consensus.nBudgetPaymentsCycleBlocks = 7200;
        consensus.nBudgetPaymentsWindowBlocks = 100;
        consensus.nSuperblockStartBlock = 100000000;
        consensus.nSuperblockStartHash = uint256S("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
        consensus.nSuperblockCycle = 7200;
        consensus.nGovernanceMinQuorum = 10;
        consensus.nGovernanceFilterElements = 20000;
        consensus.nMasternodeMinimumConfirmations = 15;
        consensus.BIP34Height = 0;
        consensus.BIP34Hash = uint256S("0x0006eb6114355f76dd011fb716cdf087a2b4336aa4419d77d915c74e2e679da1");
        consensus.BIP65Height = 200;
        consensus.BIP66Height = 100;
        consensus.DIP0001Height = 1170;
        consensus.DIP0003Height = 6350;
        consensus.DIP0003EnforcementHeight = 100000000; // TODO spork show SPORK_15_DETERMINISTIC_MNS_ENABLED
        consensus.DIP0003EnforcementHash = uint256S("000000000000000000000000000000000000000000000000000000000000000"); // TODO
        consensus.DIP0008Height = 100000000; // TODO
        consensus.powLimit = uint256S("000fffffff000000000000000000000000000000000000000000000000000000");
        consensus.nPowTargetTimespan = 24 * 60 * 60;
        consensus.nPowTargetSpacing = 6 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true; // Main: false
        consensus.fPowNoRetargeting = false;
        consensus.nPowNTSHeight = 4794; // like mainnet
        consensus.nRuleChangeActivationThreshold = 1916;
        consensus.nMinerConfirmationWindow = 2016; // 8,4 days
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999;

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1642554000; // 2022 Jan 19th
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1674090000; //  2023 Jan 19th

        // Deployment of DIP0001
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nStartTime = 1642208400; // 2022 Jan 15th
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nTimeout = 1673744400;   // 2023 Jan 15th
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nWindowSize = 10;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nThresholdStart = 8; // 80% of 10 MN

        // Deployment of BIP147
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP147].bit = 2;
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP147].nStartTime = 1641862800; // 2022 Jan 11th
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP147].nTimeout = 1673398800;   // 2023 Jan 11th
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP147].nWindowSize = 10;
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP147].nThresholdStart = 8; // 80% of 10 MN

        // Deployment of DIP0003
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0003].bit = 3;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0003].nStartTime = 1643677200; // 2022 Feb 1th
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0003].nTimeout = 1675213200; // 2023 Feb 1th
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0003].nWindowSize = 10;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0003].nThresholdStart = 7; // 70% of 10

        // Deployment of DIP0008 - TODO
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0008].bit = 4;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0008].nStartTime = 3000000000;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0008].nTimeout = 3300000000;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0008].nWindowSize = 100;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0008].nThresholdStart = 50; // 50% of 100

        // Deployment of Block Reward Reallocation - TODO
        consensus.vDeployments[Consensus::DEPLOYMENT_REALLOC].bit = 5;
        consensus.vDeployments[Consensus::DEPLOYMENT_REALLOC].nStartTime = 3000000000;
        consensus.vDeployments[Consensus::DEPLOYMENT_REALLOC].nTimeout = 3300000000;
        consensus.vDeployments[Consensus::DEPLOYMENT_REALLOC].nWindowSize = 100;
        consensus.vDeployments[Consensus::DEPLOYMENT_REALLOC].nThresholdStart = 80; // 80% of 100
        consensus.vDeployments[Consensus::DEPLOYMENT_REALLOC].nThresholdMin = 60; // 60% of 100
        consensus.vDeployments[Consensus::DEPLOYMENT_REALLOC].nFalloffCoeff = 5; // this corresponds to 10 periods

        // Deployment of DIP0020, DIP0021 and LLMQ_100_67 quorums - TODO
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0020].bit = 6;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0020].nStartTime = 3000000000;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0020].nTimeout = 3300000000;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0020].nWindowSize = 100;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0020].nThresholdStart = 80; // 80% of 100
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0020].nThresholdMin = 60; // 60% of 100
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0020].nFalloffCoeff = 5; // this corresponds to 10 periods

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00000000000000000000000000000000000000000000000000000000009df47a"); // block 768

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x0001a493d1f95c60541d553e8f1b748d48e344a0a2c8c27460196a727555cea8");

        pchMessageStart[0] = 0x74;  // t in testnet4 (testnet3 used 0xce)
        pchMessageStart[1] = 0x44;  // D
        pchMessageStart[2] = 0x4d;  // M
        pchMessageStart[3] = 0x53;  // S
        nDefaultPort = 41419;
        nPruneAfterHeight = 100000;

        genesis = CreateGenesisBlock(1641650400, 2012, 0x1f0fffff, 1, 10 * COIN); // 2022 Jan 8th 14:00:00 UTC
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x0006eb6114355f76dd011fb716cdf087a2b4336aa4419d77d915c74e2e679da1"));
        assert(genesis.hashMerkleRoot == uint256S("0xa500729aa7e2874bdb829c2e1ea99ef3542b7c43d426bca2f6720b9a9688308a"));
        /* temp: create genesis
            printf("(CTestNetParams)\n");
            printf("hashMerkleRoot %s\n", genesis.hashMerkleRoot.ToString().c_str());
            printf("genesis.nBits 0x%x\n", genesis.nBits);
            for (genesis.nNonce = 0; ; genesis.nNonce++) {
                consensus.hashGenesisBlock = genesis.GetHash();
                uint256 hash = genesis.GetHash();
                if (UintToArith256(hash) <= UintToArith256(consensus.powLimit)) break;				
            }
            printf("hashGenesisBlock %s\n", consensus.hashGenesisBlock.ToString().c_str());
            printf("genesis.nNonce %d\n", genesis.nNonce);
            printf("genesis.nTime %d\n", genesis.nTime);
        */

        vFixedSeeds.clear();
        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));

        vSeeds.clear();

        // Testnet DMS addresses start with 't'
        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,127);
        // Testnet DMS script addresses start with '8' or '9' (Dash defaults)
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,19);
        // Testnet private keys start with '9' or 'c' (Bitcoin and Dash defaults)
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        // Testnet DMS BIP32 pubkeys start with 'tpub' (Bitcoin and Dash defaults)
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        // Testnet DMS BIP32 prvkeys start with 'tprv' (Bitcoin and Dash defaults)
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        // Testnet DMS BIP44 coin type is '1' (All coin's testnet default)
        nExtCoinType = 1;

        // long living quorum params in update period v0.13 to v0.17
        /* TODO : v0.17 activate llmq30_60 on devnet, testnet and later on mainet via spork
        consensus.llmqs[Consensus::LLMQ_30_60] = llmq30_60;
        */
        consensus.llmqs[Consensus::LLMQ_150_60] = llmq150_60;
        consensus.llmqs[Consensus::LLMQ_150_85] = llmq150_85;
        /* Dash v0.17 long living quorum params
        consensus.llmqs[Consensus::LLMQ_50_60] = llmq50_60;
        consensus.llmqs[Consensus::LLMQ_400_60] = llmq400_60;
        consensus.llmqs[Consensus::LLMQ_400_85] = llmq400_85;
        consensus.llmqs[Consensus::LLMQ_100_67] = llmq100_67;
        consensus.llmqTypeChainLocks = Consensus::LLMQ_50_60;
        consensus.llmqTypeInstantSend = Consensus::LLMQ_50_60;
        consensus.llmqTypePlatform = Consensus::LLMQ_100_67;
        */

        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fRequireRoutableExternalIP = true;
        fMineBlocksOnDemand = false;
        fAllowMultipleAddressesFromGroup = false;
        fAllowMultiplePorts = false;
        nLLMQConnectionRetryTimeout = 60;

        nPoolMinParticipants = 2;
        nPoolMaxParticipants = 20;
        nFulfilledRequestExpireTime = 5*60; // fulfilled requests expire in 5 minutes

        vSporkAddresses = {"tP8UJJGyQ64kLDDHxrP3QYF8xt9ta2myPk"}; // testnet4
        nMinSporkKeys = 1;
        fBIP9CheckMasternodesUpgraded = true;

        checkpointData = {
            {
                {     0, uint256S("0x0006eb6114355f76dd011fb716cdf087a2b4336aa4419d77d915c74e2e679da1")}, // 2022-Jan-08
                {   768, uint256S("0x0001a493d1f95c60541d553e8f1b748d48e344a0a2c8c27460196a727555cea8")}, // 2022-Jan-13
            }
        };

        chainTxData = ChainTxData{
            1642072314, // * UNIX timestamp of last known number of transactions
            815,        // * total number of transactions between genesis and that timestamp
                        //   (the tx=... number in the UpdateTip debug.log lines)
            0.02        // * estimated number of transactions per second after that timestamp
        };

    }
};

/**
 * Devnet
 */
class CDevNetParams : public CChainParams {
public:
    CDevNetParams(bool fHelpOnly = false) {
        strNetworkID = "devnet";
        consensus.nSubsidyDecreaseStart = 1001;
        consensus.nMasternodePaymentsStartBlock = 2010; // not true, but it's ok as long as it's less then nMasternodePaymentsIncreaseBlock
        consensus.nMasternodePaymentsIncreaseBlock = 2030;
        consensus.nMasternodePaymentsIncreasePeriod = 10;
        consensus.nInstantSendConfirmationsRequired = 6;
        consensus.nInstantSendKeepLock = 24;
        consensus.nBudgetPaymentsStartBlock = 100000000;
        consensus.nBudgetPaymentsCycleBlocks = 7200;
        consensus.nBudgetPaymentsWindowBlocks = 100;
        consensus.nSuperblockStartBlock = 100000000;
        consensus.nSuperblockStartHash = uint256(); // do not check this on devnet
        consensus.nSuperblockCycle = 24; // Superblocks can be issued hourly on devnet
        consensus.nGovernanceMinQuorum = 1;
        consensus.nGovernanceFilterElements = 500;
        consensus.nMasternodeMinimumConfirmations = 1;
        consensus.BIP34Height = 1; // BIP34 activated immediately on devnet
        consensus.BIP65Height = 1; // BIP65 activated immediately on devnet
        consensus.BIP66Height = 1; // BIP66 activated immediately on devnet
        consensus.DIP0001Height = 2; // DIP0001 activated immediately on devnet
        consensus.DIP0003Height = 230; // devnet-4
        consensus.DIP0003EnforcementHeight = 5105;
        consensus.DIP0003EnforcementHash = uint256S("000350a84ef790275c1f2a1f6428ca36199b45fd1d0fcdb34ca4c5d6bc670798");
        consensus.DIP0008Height = 100000000; // TODO
        consensus.powLimit = uint256S("7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"); // ~uint256(0) >> 1
        consensus.nPowTargetTimespan = 24 * 60 * 60; // Dash: 1 day
        consensus.nPowTargetSpacing = 6 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = false;
        consensus.nPowNTSHeight = 0; // or 4794 like main and testnet
        consensus.nRuleChangeActivationThreshold = 1512; // 75% for testchains
        consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1648558800; // 2022-Mar-29
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1680429600; // 2023-Apr-02

        // Deployment of DIP0001
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nStartTime = 1640188800; // 2021-Dec-22
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nTimeout = 1647129600; // 2022-Mar-13
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nWindowSize = 10;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nThresholdStart = 8; // 80% of 10

        // Deployment of BIP147
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP147].bit = 2;
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP147].nStartTime = 1640354400; // 2021-Dec-24
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP147].nTimeout = 1647129600; // 2022-Mar-13
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP147].nWindowSize = 10;
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP147].nThresholdStart = 8;

        // Deployment of DIP0003
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0003].bit = 3;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0003].nStartTime = 1640275200; // 2021-Dec-23
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0003].nTimeout = 1647129600; // 2022-Mar-13
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0003].nWindowSize = 10;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0003].nThresholdStart = 8;

        // Deployment of DIP0008
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0008].bit = 4;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0008].nStartTime = 1648465200; // 2022-Mar-28
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0008].nTimeout = 1680346800; // 2023-Apr-01
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0008].nWindowSize = 100;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0008].nThresholdStart = 50; // 50% of 100

        // Deployment of Block Reward Reallocation
        consensus.vDeployments[Consensus::DEPLOYMENT_REALLOC].bit = 5;
        consensus.vDeployments[Consensus::DEPLOYMENT_REALLOC].nStartTime = 1648638000; // 2022-Mar-30
        consensus.vDeployments[Consensus::DEPLOYMENT_REALLOC].nTimeout = 1680346800; // 2023-Apr-01
        consensus.vDeployments[Consensus::DEPLOYMENT_REALLOC].nWindowSize = 100;
        consensus.vDeployments[Consensus::DEPLOYMENT_REALLOC].nThresholdStart = 80; // 80% of 100
        consensus.vDeployments[Consensus::DEPLOYMENT_REALLOC].nThresholdMin = 60; // 60% of 100
        consensus.vDeployments[Consensus::DEPLOYMENT_REALLOC].nFalloffCoeff = 5; // this corresponds to 10 periods

        // Deployment of DIP0020, DIP0021 and LLMQ_30_60, LLMQ_100_67 quorums
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0020].bit = 6;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0020].nStartTime = 1648810800; // 2022-Apr-01
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0020].nTimeout = 1680346800; // 2023-Apr-01
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0020].nWindowSize = 100;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0020].nThresholdStart = 80; // 80% of 100
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0020].nThresholdMin = 60; // 60% of 100
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0020].nFalloffCoeff = 5; // this corresponds to 10 periods

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x000000000000000000000000000000000000000000000000000000000000000");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x000000000000000000000000000000000000000000000000000000000000000");

        pchMessageStart[0] = 0xce;
        pchMessageStart[1] = 0x44;  // D
        pchMessageStart[2] = 0x4d;  // M
        pchMessageStart[3] = 0x53;  // S
        nDefaultPort = 41417;
        nPruneAfterHeight = 1000;

        genesis = CreateGenesisBlock(1535270402, 0, 0x207fffff, 1, 10 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x500f24a842f7b81dfa1d11d46f37b819812ae1f745b45c86f28249c1c669d5dc"));
        assert(genesis.hashMerkleRoot == uint256S("0xa500729aa7e2874bdb829c2e1ea99ef3542b7c43d426bca2f6720b9a9688308a"));

        if (!fHelpOnly) {
            devnetGenesis = FindDevNetGenesisBlock(genesis, 50 * COIN);  // TODO : set to 10 in next devnet?
            consensus.hashDevnetGenesisBlock = devnetGenesis.GetHash();
        }

        vFixedSeeds.clear();
        vSeeds.clear();
        //vSeeds.push_back(CDNSSeedData("dashevo.org",  "devnet-seed.dashevo.org"));

        // Devnet DMS addresses start with 'y' (Dash default)
        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,140);
        // Devnet DMS script addresses start with '8' or '9'
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,19);
        // Devnet private keys start with '9' or 'c' (Bitcoin defaults)
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        // Devnet DMS BIP32 pubkeys start with 'tpub' (Bitcoin defaults)
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        // Devnet DMS BIP32 prvkeys start with 'tprv' (Bitcoin defaults)
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        // Devnet DMS BIP44 coin type is '1' (All coin's testnet default)
        nExtCoinType = 1;

        // long living quorum params in update period v0.13 to v0.17
        consensus.llmqs[Consensus::LLMQ_DEVNET] = llmq_devnet; // LLMQ_10_60
        consensus.llmqs[Consensus::LLMQ_30_60] = llmq30_60;    // used in devnet-4, not in devnet-5
        /* TODO : v0.17 activate llmq30_60 on devnet-5, testnet and later on mainet via spork
        consensus.llmqs[Consensus::LLMQ_30_60] = llmq30_60;    // used in devnet-4, not in devnet-5
        */
        consensus.llmqs[Consensus::LLMQ_150_60] = llmq150_60;
        consensus.llmqs[Consensus::LLMQ_150_85] = llmq150_85;
        consensus.llmqTypeChainLocks = Consensus::LLMQ_150_60;  // or LLMQ_30_60?
        consensus.llmqTypeInstantSend = Consensus::LLMQ_30_60;
        consensus.llmqTypePlatform = Consensus::LLMQ_100_67;
        /* Dash v0.17 long living quorum params
        consensus.llmqs[Consensus::LLMQ_DEVNET] = llmq_devnet;
        consensus.llmqs[Consensus::LLMQ_50_60] = llmq50_60;
        consensus.llmqs[Consensus::LLMQ_400_60] = llmq400_60;
        consensus.llmqs[Consensus::LLMQ_400_85] = llmq400_85;
        consensus.llmqs[Consensus::LLMQ_100_67] = llmq100_67;
        consensus.llmqTypeChainLocks = Consensus::LLMQ_50_60;
        consensus.llmqTypeInstantSend = Consensus::LLMQ_50_60;
        consensus.llmqTypePlatform = Consensus::LLMQ_100_67;
        */

        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fRequireRoutableExternalIP = false;
        fMineBlocksOnDemand = false;
        fAllowMultipleAddressesFromGroup = true;
        fAllowMultiplePorts = true;
        nLLMQConnectionRetryTimeout = 60;

        nPoolMinParticipants = 2;
        nPoolMaxParticipants = 20;
        nFulfilledRequestExpireTime = 5*60; // fulfilled requests expire in 5 minutes

        vSporkAddresses = {"ybUKxfDwPA2RvmwtTRSYMZagQ3STq2kkgg"}; // devnet-5
        nMinSporkKeys = 1;
        // devnets are started with no blocks and no MN, so we can't check for upgraded MN (as there are none)
        fBIP9CheckMasternodesUpgraded = false;

        checkpointData = (CCheckpointData) {
            {
                { 0, uint256S("0x500f24a842f7b81dfa1d11d46f37b819812ae1f745b45c86f28249c1c669d5dc")},
                { 1, devnetGenesis.GetHash() },
            }
        };

        chainTxData = ChainTxData{
            devnetGenesis.GetBlockTime(), // * UNIX timestamp of devnet genesis block
            2,                            // * we only have 2 coinbase transactions when a devnet is started up
            0.01                          // * estimated number of transactions per second
        };
    }
};

/**
 * Regression test
 */
class CRegTestParams : public CChainParams {
public:
    CRegTestParams() {
        strNetworkID = "regtest";
        consensus.nSubsidyDecreaseStart = 1001;
        consensus.nMasternodePaymentsStartBlock = 240;
        consensus.nMasternodePaymentsIncreaseBlock = 350;
        consensus.nMasternodePaymentsIncreasePeriod = 10;
        consensus.nInstantSendConfirmationsRequired = 6;
        consensus.nInstantSendKeepLock = 6;
        consensus.nBudgetPaymentsStartBlock = 100000000;
        consensus.nBudgetPaymentsCycleBlocks = 50;
        consensus.nBudgetPaymentsWindowBlocks = 10;
        consensus.nSuperblockStartBlock = 100000000;
        consensus.nSuperblockStartHash = uint256(); // do not check this on regtest
        consensus.nSuperblockCycle = 10;
        consensus.nGovernanceMinQuorum = 1;
        consensus.nGovernanceFilterElements = 100;
        consensus.nMasternodeMinimumConfirmations = 1;
        consensus.BIP34Height = 100000000; // BIP34 has not activated on regtest (far in the future so block v1 are not rejected in tests)
        consensus.BIP34Hash = uint256();
        consensus.BIP65Height = 1351; // BIP65 activated on regtest (Used in rpc activation tests)
        consensus.BIP66Height = 1251; // BIP66 activated on regtest (Used in rpc activation tests)
        consensus.DIP0001Height = 2000;
        consensus.DIP0003Height = 432;
        consensus.DIP0003EnforcementHeight = 500;
        consensus.DIP0003EnforcementHash = uint256();
        consensus.DIP0008Height = 432;
        consensus.powLimit = uint256S("7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"); // ~uint256(0) >> 1
        consensus.nPowTargetTimespan = 24 * 60 * 60; // Dash: 1 day
        consensus.nPowTargetSpacing = 6 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = true;
        consensus.nPowNTSHeight = 0;
        consensus.nRuleChangeActivationThreshold = 108; // 75% for testchains
        consensus.nMinerConfirmationWindow = 144; // Faster than normal for regtest (144 instead of 2016)
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nTimeout = 999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP147].bit = 2;
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP147].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP147].nTimeout = 999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0003].bit = 3;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0003].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0003].nTimeout = 999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0008].bit = 4;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0008].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0008].nTimeout = 999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_REALLOC].bit = 5;
        consensus.vDeployments[Consensus::DEPLOYMENT_REALLOC].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_REALLOC].nTimeout = 999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_REALLOC].nWindowSize = 500;
        consensus.vDeployments[Consensus::DEPLOYMENT_REALLOC].nThresholdStart = 400; // 80%
        consensus.vDeployments[Consensus::DEPLOYMENT_REALLOC].nThresholdMin = 300; // 60%
        consensus.vDeployments[Consensus::DEPLOYMENT_REALLOC].nFalloffCoeff = 5;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0020].bit = 6;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0020].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0020].nTimeout = 999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0020].nWindowSize = 100;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0020].nThresholdStart = 80;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0020].nThresholdMin = 60;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0020].nFalloffCoeff = 5;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x00");

        pchMessageStart[0] = 0xfc;
        pchMessageStart[1] = 0x44;  // D
        pchMessageStart[2] = 0x4d;  // M
        pchMessageStart[3] = 0x53;  // S
        nDefaultPort = 41418;
        nPruneAfterHeight = 1000;

        genesis = CreateGenesisBlock(1535270402, 0, 0x207fffff, 1, 10 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x500f24a842f7b81dfa1d11d46f37b819812ae1f745b45c86f28249c1c669d5dc"));
        assert(genesis.hashMerkleRoot == uint256S("0xa500729aa7e2874bdb829c2e1ea99ef3542b7c43d426bca2f6720b9a9688308a"));

        vFixedSeeds.clear(); //!< Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();      //!< Regtest mode doesn't have any DNS seeds.

        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fRequireRoutableExternalIP = false;
        fMineBlocksOnDemand = true;
        fAllowMultipleAddressesFromGroup = true;
        fAllowMultiplePorts = true;
        nLLMQConnectionRetryTimeout = 1; // must be lower then the LLMQ signing session timeout so that tests have control over failing behavior

        nFulfilledRequestExpireTime = 5*60; // fulfilled requests expire in 5 minutes
        nPoolMinParticipants = 2;
        nPoolMaxParticipants = 20;

        // privKey: cP4EKFyJsHT39LDqgdcB43Y3YXjNyjb5Fuas1GQSeAtjnZWmZEQK
        vSporkAddresses = {"yj949n1UH6fDhw6HtVE5VMj2iSTaSWBMcW"};
        nMinSporkKeys = 1;
        // regtest usually has no masternodes in most tests, so don't check for upgraged MNs
        fBIP9CheckMasternodesUpgraded = false;

        checkpointData = {
            {
                {0, uint256S("0x500f24a842f7b81dfa1d11d46f37b819812ae1f745b45c86f28249c1c669d5dc")},
            }
        };

        chainTxData = ChainTxData{
            0,
            0,
            0
        };

        // Regtest DMS addresses start with 'y'
        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,140);
        // Regtest DMS script addresses start with '8' or '9'
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,19);
        // Regtest private keys start with '9' or 'c' (Bitcoin defaults)
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        // Regtest BIP32 pubkeys start with 'tpub' (Bitcoin defaults)
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        // Regtest DMS BIP32 prvkeys start with 'tprv' (Bitcoin defaults)
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        // Regtest DMS BIP44 coin type is '1' (All coin's testnet default)
        nExtCoinType = 1;

        // long living quorum params
        consensus.llmqs[Consensus::LLMQ_TEST] = llmq_test;
        consensus.llmqs[Consensus::LLMQ_30_60] = llmq30_60;
        consensus.llmqs[Consensus::LLMQ_150_60] = llmq150_60;
        consensus.llmqs[Consensus::LLMQ_150_85] = llmq150_85;
        consensus.llmqTypeChainLocks = Consensus::LLMQ_150_60;
        consensus.llmqTypeInstantSend = Consensus::LLMQ_30_60;
        consensus.llmqTypePlatform = Consensus::LLMQ_100_67;
        /* Dash v0.17 long living quorum params
        consensus.llmqs[Consensus::LLMQ_TEST] = llmq_test;
        consensus.llmqs[Consensus::LLMQ_TEST_V17] = llmq_test_v17;
        consensus.llmqTypeChainLocks = Consensus::LLMQ_TEST;
        consensus.llmqTypeInstantSend = Consensus::LLMQ_TEST;
        consensus.llmqTypePlatform = Consensus::LLMQ_TEST;
        */
    }
};

static std::unique_ptr<CChainParams> globalChainParams;

const CChainParams &Params() {
    assert(globalChainParams);
    return *globalChainParams;
}

std::unique_ptr<CChainParams> CreateChainParams(const std::string& chain, bool fHelpOnly)
{
    if (chain == CBaseChainParams::MAIN)
        return std::unique_ptr<CChainParams>(new CMainParams());
    else if (chain == CBaseChainParams::TESTNET)
        return std::unique_ptr<CChainParams>(new CTestNetParams());
    else if (chain == CBaseChainParams::DEVNET) {
        return std::unique_ptr<CChainParams>(new CDevNetParams(fHelpOnly));
    } else if (chain == CBaseChainParams::REGTEST)
        return std::unique_ptr<CChainParams>(new CRegTestParams());
    throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string& network)
{
    SelectBaseParams(network);
    globalChainParams = CreateChainParams(network);
}

void UpdateVersionBitsParameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout, int64_t nWindowSize, int64_t nThresholdStart, int64_t nThresholdMin, int64_t nFalloffCoeff)
{
    globalChainParams->UpdateVersionBitsParameters(d, nStartTime, nTimeout, nWindowSize, nThresholdStart, nThresholdMin, nFalloffCoeff);
}

void UpdateDIP3Parameters(int nActivationHeight, int nEnforcementHeight)
{
    globalChainParams->UpdateDIP3Parameters(nActivationHeight, nEnforcementHeight);
}

void UpdateDIP8Parameters(int nActivationHeight)
{
    globalChainParams->UpdateDIP8Parameters(nActivationHeight);
}

void UpdateBudgetParameters(int nMasternodePaymentsStartBlock, int nBudgetPaymentsStartBlock, int nSuperblockStartBlock)
{
    globalChainParams->UpdateBudgetParameters(nMasternodePaymentsStartBlock, nBudgetPaymentsStartBlock, nSuperblockStartBlock);
}

void UpdateDevnetSubsidyAndDiffParams(int nMinimumDifficultyBlocks, int nHighSubsidyBlocks, int nHighSubsidyFactor)
{
    globalChainParams->UpdateSubsidyAndDiffParams(nMinimumDifficultyBlocks, nHighSubsidyBlocks, nHighSubsidyFactor);
}

void UpdateDevnetLLMQChainLocks(Consensus::LLMQType llmqType)
{
    globalChainParams->UpdateLLMQChainLocks(llmqType);
}

void UpdateDevnetLLMQInstantSend(Consensus::LLMQType llmqType)
{
    globalChainParams->UpdateLLMQInstantSend(llmqType);
}

void UpdateLLMQTestParams(int size, int threshold)
{
    globalChainParams->UpdateLLMQTestParams(size, threshold);
}

void UpdateLLMQDevnetParams(int size, int threshold)
{
    globalChainParams->UpdateLLMQDevnetParams(size, threshold);
}
