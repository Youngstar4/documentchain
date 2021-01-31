// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin Core developers
// Copyright (c) 2014-2020 The Dash Core developers
// Copyright (c) 2018-2021 The Documentchain developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <chainparams.h>
#include <consensus/merkle.h>

#include <tinyformat.h>
#include <util.h>
#include <utilstrencodings.h>

#include <arith_uint256.h>

#include <assert.h>
#include <memory>

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
{                            // DO NOT MODIFY GENESIS TEXT
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
        consensus.nMasternodeMinimumConfirmations = 15;  // Confirmations for initial Masternode transaction of 5000 DMS
        consensus.BIP34Height = 0; // 0=genesis block  // Bitcoin: 227931, Dash: 951;
        consensus.BIP34Hash = uint256S("0x00000af4a21d6e8daa4026a5eafc7132089a7dbb9d3921b12c4fa39b78c9a010");
        consensus.BIP65Height = 84100;
        consensus.BIP66Height = 84000;
        consensus.DIP0001Height = 143481; // May 18, 2020
        consensus.DIP0003Height = 3000000; // TODOv16
        consensus.DIP0003EnforcementHeight = 3000000; // TODOv16
        consensus.DIP0003EnforcementHash = uint256S("000000000000002d1734087b4c5afc3133e4e1c3e1a89218f62bcd9bb3d17f81"); // TODOv16
        consensus.powLimit = uint256S("0000ffffff000000000000000000000000000000000000000000000000000000");
        consensus.nPowTargetTimespan = 24 * 60 * 60; // PoW calculation 24 * 60 * 60;  1 day
        consensus.nPowTargetSpacing = 6 * 60; // DMS 6 minutes (initial 4 minutes); Bitcoin 10 minutes; Dash 2.5 minutes
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.nPowNTSHeight = 4794; // see commit df04016c84ee09eb716605a962cf05064c7eea9f
        consensus.nRuleChangeActivationThreshold = 1916; // 95% of 2016
        consensus.nMinerConfirmationWindow = 2016; // relevant to BIP 9 soft fork, see params.h
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // Dash: January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // Dash: December 31, 2008

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1604923200; // Nov 9th, 2020
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1609761600; // Jan 4th, 2021

        // Deployment of DIP0001
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nStartTime = 1589536800; // May 15th, 2020
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nTimeout = 1592215200; // Jun 15th, 2020
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nWindowSize = 283;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nThresholdStart = 254; // 80% of 283

        // Deployment of BIP147, Documentchain: active since block 12096
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP147].bit = 2;
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP147].nStartTime = 1524477600; // Dash: Apr 23th, 2018
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP147].nTimeout = 1556013600; // Dash: Apr 23th, 2019
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP147].nWindowSize = 4032;
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP147].nThresholdStart = 3226; // 80% of 4032

        // Deployment of DIP0003, TODOv16
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0003].bit = 3;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0003].nStartTime = 3000000000;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0003].nTimeout = 3300000000;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0003].nWindowSize = 4032;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0003].nThresholdStart = 3226; // 80% of 4032

        // Deployment of DIP0008, TODOv16
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0008].bit = 4;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0008].nStartTime = 3000000000;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0008].nTimeout = 3300000000;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0008].nWindowSize = 4032;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0008].nThresholdStart = 3226; // 80% of 4032

        // Deployment of Block Reward Reallocation, TODOv16
        consensus.vDeployments[Consensus::DEPLOYMENT_REALLOC].bit = 5;
        consensus.vDeployments[Consensus::DEPLOYMENT_REALLOC].nStartTime = 3000000000;
        consensus.vDeployments[Consensus::DEPLOYMENT_REALLOC].nTimeout = 3300000000;
        consensus.vDeployments[Consensus::DEPLOYMENT_REALLOC].nWindowSize = 4032;
        consensus.vDeployments[Consensus::DEPLOYMENT_REALLOC].nThresholdStart = 3226; // 80% of 4032
        consensus.vDeployments[Consensus::DEPLOYMENT_REALLOC].nThresholdMin = 2420; // 60% of 4032
        consensus.vDeployments[Consensus::DEPLOYMENT_REALLOC].nFalloffCoeff = 5; // this corresponds to 10 periods

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x0000000000000000000000000000000000000000000000000000032f04770142"); // = 3.500.473.254.210 total work, block 175594

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x0000000ee3871b495f8f26714200b50dbe56fb48c9bde4a9b1a4b72ba5793c1c"); // 175594

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        pchMessageStart[0] = 0x24;  // $
        pchMessageStart[1] = 0x44;  // D
        pchMessageStart[2] = 0x4d;  // M
        pchMessageStart[3] = 0x53;  // S
        nDefaultPort = 41319;
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

        // Note that of those which support the service bits prefix, most only support a subset of
        // possible options.
        // This is fine at runtime as we'll fall back to using them as a oneshot if they dont support the
        // service bits we want, but we should get them updated to support all service bits wanted by any
        // release ASAP to avoid it where possible.
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

        // Dash BIP44 coin type is '5'
        nExtCoinType = 5;

        // fixed seed nodes, use contrib/seeds/generate-seeds.py
        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        // long living quorum params
        consensus.llmqs[Consensus::LLMQ_50_60] = llmq50_60;
        consensus.llmqs[Consensus::LLMQ_400_60] = llmq400_60;
        consensus.llmqs[Consensus::LLMQ_400_85] = llmq400_85;
        consensus.llmqTypeChainLocks = Consensus::LLMQ_400_60;
        consensus.llmqTypeInstantSend = Consensus::LLMQ_50_60;

        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fRequireRoutableExternalIP = true;
        fMineBlocksOnDemand = false;
        fAllowMultipleAddressesFromGroup = false;
        fAllowMultiplePorts = false;
        nLLMQConnectionRetryTimeout = 60;

        nPoolMinParticipants = 3;      
        nPoolNewMinParticipants = 3;  // SPORK_22_PS_MORE_PARTICIPANTS
        nPoolMaxParticipants = 5;
        nPoolNewMaxParticipants = 20; // SPORK_22_PS_MORE_PARTICIPANTS
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
                { 51834, uint256S("0x00001617483dacc4cb349535a9d1806c0b749801111d705362d61d7df72269d7")}, // 2019-Apr-13
                { 59098, uint256S("0x0000062c9eeac8249ff40512763846be367c9404c2ebded2646309bb30a28f4a")}, // 2019-May-15
                { 68050, uint256S("0x000005c0ed9cec8cd6c2a23c673b3047bf2f3c205d85ca3843fb8ec4b53c27c9")}, // 2019-Jun-23
                { 73989, uint256S("0x0000101d903ace244df2686ca8005bf605910ba3b521b5217ba8ada6fa12d79f")}, // 2019-Jul-19
                { 79403, uint256S("0x00000295f0e654ec4c7ec2e4051acaca0eda31ae2ca2d7a38e56a7875cf457dd")}, // 2019-Aug-12
                { 86998, uint256S("0x0000000d6ca0904b7ec849283a177b7a25b0ed3c23413c14d9e564bfa4d2514a")}, // 2019-Sep-14
                { 94707, uint256S("0x000000d52750d7b5cf3e682106b258de7f9c0f66e011e7cf0699b6526a05339d")}, // 2019-Oct-18
                {100000, uint256S("0x00000095a23b6cc9ec9c0d7385bf4c3705220271d425a203af41163b4600e96d")}, // 2019-Nov-10
                {110034, uint256S("0x00000025fcd0421225ea0c8d5874b3040a81618c609f2419bce9d339a96faee7")}, // 2019-Dec-24
                {120004, uint256S("0x00000031fd82d41fa6fda29a405a55162304822be9b03beed79e47f25d3f3296")}, // 2020-Feb-06
                {127991, uint256S("0x00000042ec60e4badf5e3288b1dbb8da2a27af4b88b6ec299e7149586784ccfd")}, // 2020-Mar-12
                {135808, uint256S("0x00000025868ef0958009ba542ded2faf5242cd835445ca6c5556eff5a19161ab")}, // 2020-Apr-15
                {144997, uint256S("0x00000074c2b1520d116c447d2cf6170951642fc50487b300074d76952fc20a4d")}, // 2020-May-25
                {154972, uint256S("0x00000053c7c6c55129d6e9c9477ddcdbd351847d748ff77eff13dbc44bde58d4")}, // 2020-Jul-08
                {165993, uint256S("0x000000391480ecb91edb3bc72ed1a206c6640c7c8927c244536e0bc90c43b51f")}, // 2020-Aug-25
                {175594, uint256S("0x0000000ee3871b495f8f26714200b50dbe56fb48c9bde4a9b1a4b72ba5793c1c")}, // 2020-Oct-06
            }
        };

        chainTxData = ChainTxData{
            1601994910, // * UNIX timestamp of last known number of transactions
            295331,     // * total number of transactions between genesis and that timestamp
                        //   (the tx=... number in the UpdateTip debug.log lines)
            0.04        // * estimated number of transactions per second after that timestamp
        };
    }
};

/**
 * Testnet (v3)
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
        consensus.BIP34Hash = uint256S("0x00004399a114a034b2f8d742b8e7f018d3cfdec0b25150d0b7e271b63c9cd4ce");
        consensus.BIP65Height = 84100;
        consensus.BIP66Height = 84000;
        consensus.DIP0001Height = 138820;
        consensus.DIP0003Height = 3000000; // TODOv16
        consensus.DIP0003EnforcementHeight = 3000000; // TODOv16
        consensus.DIP0003EnforcementHash = uint256S("00000055ebc0e974ba3a3fb785c5ad4365a39637d4df168169ee80d313612f8f"); // TODOv16
        consensus.powLimit = uint256S("000fffffff000000000000000000000000000000000000000000000000000000");
        consensus.nPowTargetTimespan = 24 * 60 * 60;
        consensus.nPowTargetSpacing = 6 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true; // Main: false

        
        consensus.fPowNoRetargeting = false;
        consensus.nPowNTSHeight = 95937; // fast testnet, blocks 0..95936 
        consensus.nRuleChangeActivationThreshold = 1916;
        consensus.nMinerConfirmationWindow = 2016;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1588327200; // May 1th, 2020
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1591005600; // Jun 1th, 2020

        // Deployment of DIP0001
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nStartTime = 1586340000; // 2020-Apr-08
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nTimeout = 1586599200;   // 2020-Apr-11
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nWindowSize = 10;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nThresholdStart = 8; // 80% of 10 MN

        // Deployment of BIP147
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP147].bit = 2;
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP147].nStartTime = 1585821600; // 2020-Apr-02
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP147].nTimeout = 1585994400;   // 2020-Apr-04
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP147].nWindowSize = 10;
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP147].nThresholdStart = 8; // 80% of 10 MN

        // Deployment of DIP0003, TODOv16
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0003].bit = 3;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0003].nStartTime = 3000000000;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0003].nTimeout = 3300000000;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0003].nWindowSize = 100;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0003].nThresholdStart = 50; // 50% of 100

        // Deployment of DIP0008, TODOv16
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0008].bit = 4;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0008].nStartTime = 3000000000;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0008].nTimeout = 3300000000;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0008].nWindowSize = 100;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0008].nThresholdStart = 50; // 50% of 100

        // Deployment of Block Reward Reallocation, TODOv16
        consensus.vDeployments[Consensus::DEPLOYMENT_REALLOC].bit = 5;
        consensus.vDeployments[Consensus::DEPLOYMENT_REALLOC].nStartTime = 3300000000;
        consensus.vDeployments[Consensus::DEPLOYMENT_REALLOC].nTimeout = 3300000000;
        consensus.vDeployments[Consensus::DEPLOYMENT_REALLOC].nWindowSize = 100;
        consensus.vDeployments[Consensus::DEPLOYMENT_REALLOC].nThresholdStart = 80; // 80% of 100
        consensus.vDeployments[Consensus::DEPLOYMENT_REALLOC].nThresholdMin = 60; // 60% of 100
        consensus.vDeployments[Consensus::DEPLOYMENT_REALLOC].nFalloffCoeff = 5; // this corresponds to 10 periods

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x000000000000000000000000000000000000000000000000000000008e4b3670"); // = 2.387.293.808, block 183038

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x0000265f61ad4c3c7841f177c56d9616ea1f2b0ed795271139760028e8c2bd82"); // 183038

        pchMessageStart[0] = 0xce;  // same as Dash Testnet
        pchMessageStart[1] = 0x44;  // D
        pchMessageStart[2] = 0x4d;  // M
        pchMessageStart[3] = 0x53;  // S
        nDefaultPort = 41419;
        nPruneAfterHeight = 100000;

        genesis = CreateGenesisBlock(1559127600, 17832, 0x1f0fffff, 1, 10 * COIN); // 29.05.2019 13:00:00
        consensus.hashGenesisBlock = genesis.GetHash();

        assert(consensus.hashGenesisBlock == uint256S("0x00004399a114a034b2f8d742b8e7f018d3cfdec0b25150d0b7e271b63c9cd4ce"));
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

        // Testnet Dash BIP44 coin type is '1' (All coin's testnet default)
        nExtCoinType = 1;

        // long living quorum params
        consensus.llmqs[Consensus::LLMQ_50_60] = llmq50_60;
        consensus.llmqs[Consensus::LLMQ_400_60] = llmq400_60;
        consensus.llmqs[Consensus::LLMQ_400_85] = llmq400_85;
        consensus.llmqTypeChainLocks = Consensus::LLMQ_50_60;
        consensus.llmqTypeInstantSend = Consensus::LLMQ_50_60;

        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fRequireRoutableExternalIP = true;
        fMineBlocksOnDemand = false;
        fAllowMultipleAddressesFromGroup = false;
        fAllowMultiplePorts = true;
        nLLMQConnectionRetryTimeout = 60;

        nPoolMinParticipants = 3;
        nPoolNewMinParticipants = 2;
        nPoolMaxParticipants = 5;
        nPoolNewMaxParticipants = 20;
        nFulfilledRequestExpireTime = 5*60; // fulfilled requests expire in 5 minutes

        vSporkAddresses = {"t9Q5m4qupg9gmPh5dRvaTotcyuoaeE5ZZH"};
        nMinSporkKeys = 1;
        fBIP9CheckMasternodesUpgraded = true;

        checkpointData = {
            {
                {     0, uint256S("0x00004399a114a034b2f8d742b8e7f018d3cfdec0b25150d0b7e271b63c9cd4ce")},
                {  4600, uint256S("0x00024d5b6d0aa1cfda1f78360cc85d246bab6c08b9a4684094adfbfc14afee8d")},
                { 15000, uint256S("0x0006adc6713cbbdbcb4dfe4de2c6197a5ccfa061aab8abc93c6b12c161197245")},
                { 26990, uint256S("0x0001cb86d69f2fc9c519c5cf7f31b4d86fa55114efc7762b69f969c71f190141")},
                { 35993, uint256S("0x000863c9516358586c31e98a9922d269dcb60a8399b9ed755bf563c9302d9489")},
                { 46096, uint256S("0x0007ef30042db8f9dc736b2026681de15eccb8436358b47fee137a253c97ef03")},
                { 59098, uint256S("0x0007131704fbceda251f397072bd55ab8f36f534e66330dc2d30b9c3efb87951")},
                { 68050, uint256S("0x00055754a80e12e666d0ace953812acbf703eb1adf5a741096dda8a3cd55161a")},
                { 73989, uint256S("0x0003ecfd52a44d853c3e9967764482a10e6f16dacb0a634dd62d993ef2b22cf8")},
                { 86998, uint256S("0x0007fc0bf4654559a160911cb8cabf494ffb1010867a6141ee48bd130812cea0")},
                { 94707, uint256S("0x000b602b64a3d1ef2d001bbcb5466ab68a1536e1bcddb193311c64ef14ac628d")},
                {100000, uint256S("0x00001db74c7881d3294ed79f695085086e80c17c7d042555a569b4440f734e9d")},
                {110000, uint256S("0x0001ceae3f2079f119b4e8486c6d456986963c1b36f3bfbc1a480a7e47d1cbe0")},
                {120000, uint256S("0x0000a59203584f8fa8b9556595dc01c4df53d6c1e19810d7761129ebf8f2ec1c")},
                {130004, uint256S("0x00004646f1ca07c4dc563368ac427c930fc22d2e1f7b689eddbbdf5c2026e97a")},
                {137044, uint256S("0x000090f59b07ccd5285bd79dd9a4190cd23cd6efd3d0b7b1c4069e9ce9897c12")},
                {140464, uint256S("0x00000174ee13b777c20a7aa6aef53650f0e42777788ec2c00e36a76658dfc6f6")},
                {160000, uint256S("0x0002f859981fb98abbde73b1574a0e3c05676185de4c0c385c23392ba1703cf2")}, // 2020-Jul-03
                {174000, uint256S("0x0002beb9fd3c0207b45f5825811b7a847cd043b8c1222f64e7dd172e201a6bf5")}, // 2020-Aug-30
                {183038, uint256S("0x0000265f61ad4c3c7841f177c56d9616ea1f2b0ed795271139760028e8c2bd82")}, // 2020-Oct-06            
}
        };

        chainTxData = ChainTxData{
            1601997461, // * UNIX timestamp of last known number of transactions
            185643,     // * total number of transactions between genesis and that timestamp
                        //   (the tx=... number in the UpdateTip debug.log lines)
            0.003       // * estimated number of transactions per second after that timestamp
        };

    }
};

/**
 * Devnet
 */
class CDevNetParams : public CChainParams {
public:
    CDevNetParams() {
        strNetworkID = "devnet";
        consensus.nSubsidyDecreaseStart = 1001;
        consensus.nMasternodePaymentsStartBlock = 4010; // not true, but it's ok as long as it's less then nMasternodePaymentsIncreaseBlock
        consensus.nMasternodePaymentsIncreaseBlock = 4030;
        consensus.nMasternodePaymentsIncreasePeriod = 10;
        consensus.nInstantSendConfirmationsRequired = 2;
        consensus.nInstantSendKeepLock = 6;
        consensus.nBudgetPaymentsStartBlock = 4100;
        consensus.nBudgetPaymentsCycleBlocks = 50;
        consensus.nBudgetPaymentsWindowBlocks = 10;
        consensus.nSuperblockStartBlock = 4200; // NOTE: Should satisfy nSuperblockStartBlock > nBudgetPeymentsStartBlock
        consensus.nSuperblockStartHash = uint256(); // do not check this on devnet
        consensus.nSuperblockCycle = 24; // Superblocks can be issued hourly on devnet
        consensus.nGovernanceMinQuorum = 1;
        consensus.nGovernanceFilterElements = 500;
        consensus.nMasternodeMinimumConfirmations = 1;
        consensus.BIP34Height = 1; // BIP34 activated immediately on devnet
        consensus.BIP65Height = 1; // BIP65 activated immediately on devnet
        consensus.BIP66Height = 1; // BIP66 activated immediately on devnet
        consensus.DIP0001Height = 2; // DIP0001 activated immediately on devnet
        consensus.DIP0003Height = 2; // DIP0003 activated immediately on devnet
        consensus.DIP0003EnforcementHeight = 2; // DIP0003 activated immediately on devnet
        consensus.DIP0003EnforcementHash = uint256();
        consensus.powLimit = uint256S("7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"); // ~uint256(0) >> 1
        consensus.nPowTargetTimespan = 24 * 60 * 60; // Dash: 1 day
        consensus.nPowTargetSpacing = 6* 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = false;
		consensus.nPowNTSHeight = 0;
        consensus.nRuleChangeActivationThreshold = 1512; // 75% for testchains
        consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1506556800; // September 28th, 2017
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1538092800; // September 28th, 2018

        // Deployment of DIP0001
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nStartTime = 1505692800; // Sep 18th, 2017
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nTimeout = 1537228800; // Sep 18th, 2018
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nWindowSize = 100;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nThresholdStart = 50; // 50% of 100

        // Deployment of BIP147
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP147].bit = 2;
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP147].nStartTime = 1517792400; // Feb 5th, 2018
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP147].nTimeout = 1549328400; // Feb 5th, 2019
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP147].nWindowSize = 100;
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP147].nThresholdStart = 50; // 50% of 100

        // Deployment of DIP0003, TODOv16
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0003].bit = 3;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0003].nStartTime = 3000000000;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0003].nTimeout = 3300000000;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0003].nWindowSize = 100;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0003].nThresholdStart = 50; // 50% of 100

        // Deployment of DIP0008, TODOv16
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0008].bit = 4;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0008].nStartTime = 3000000000;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0008].nTimeout = 3300000000;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0008].nWindowSize = 100;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0008].nThresholdStart = 50; // 50% of 100

        // Deployment of Block Reward Reallocation, TODOv16
        consensus.vDeployments[Consensus::DEPLOYMENT_REALLOC].bit = 5;
        consensus.vDeployments[Consensus::DEPLOYMENT_REALLOC].nStartTime = 3000000000; // Sep 1st, 2020
        consensus.vDeployments[Consensus::DEPLOYMENT_REALLOC].nTimeout = 3300000000; // Mar 21st, 2030
        consensus.vDeployments[Consensus::DEPLOYMENT_REALLOC].nWindowSize = 100;
        consensus.vDeployments[Consensus::DEPLOYMENT_REALLOC].nThresholdStart = 80; // 80% of 100
        consensus.vDeployments[Consensus::DEPLOYMENT_REALLOC].nThresholdMin = 60; // 60% of 100
        consensus.vDeployments[Consensus::DEPLOYMENT_REALLOC].nFalloffCoeff = 5; // this corresponds to 10 periods

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x000000000000000000000000000000000000000000000000000000000000000");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x000000000000000000000000000000000000000000000000000000000000000");

        pchMessageStart[0] = 0xce;  // same as Dash
        pchMessageStart[1] = 0x44;  // D
        pchMessageStart[2] = 0x4d;  // M
        pchMessageStart[3] = 0x53;  // S
        nDefaultPort = 41417;
        nPruneAfterHeight = 1000;

        genesis = CreateGenesisBlock(1535270402, 0, 0x207fffff, 1, 10 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x500f24a842f7b81dfa1d11d46f37b819812ae1f745b45c86f28249c1c669d5dc"));
        assert(genesis.hashMerkleRoot == uint256S("0xa500729aa7e2874bdb829c2e1ea99ef3542b7c43d426bca2f6720b9a9688308a"));
		/* temp: create genesis
            printf("(CDevNetParams)\n");
            printf("hashMerkleRoot %s\n", genesis.hashMerkleRoot.ToString().c_str());
            printf("genesis.nBits 0x%x\n", genesis.nBits);
            printf("hashGenesisBlock %s\n", consensus.hashGenesisBlock.ToString().c_str());
            printf("genesis.nNonce %d\n", genesis.nNonce);
		*/

        devnetGenesis = FindDevNetGenesisBlock(genesis, 50 * COIN);
        consensus.hashDevnetGenesisBlock = devnetGenesis.GetHash();

        vFixedSeeds.clear();
        vSeeds.clear();
        //vSeeds.push_back(CDNSSeedData("dashevo.org",  "devnet-seed.dashevo.org"));

        // Testnet Dash addresses start with 'y'
        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,140);
        // Testnet Dash script addresses start with '8' or '9'
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,19);
        // Testnet private keys start with '9' or 'c' (Bitcoin defaults)
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        // Testnet Dash BIP32 pubkeys start with 'tpub' (Bitcoin defaults)
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        // Testnet Dash BIP32 prvkeys start with 'tprv' (Bitcoin defaults)
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        // Testnet Dash BIP44 coin type is '1' (All coin's testnet default)
        nExtCoinType = 1;

        // long living quorum params
        consensus.llmqs[Consensus::LLMQ_DEVNET] = llmq_devnet;
        consensus.llmqs[Consensus::LLMQ_50_60] = llmq50_60;
        consensus.llmqs[Consensus::LLMQ_400_60] = llmq400_60;
        consensus.llmqs[Consensus::LLMQ_400_85] = llmq400_85;
        consensus.llmqTypeChainLocks = Consensus::LLMQ_50_60;
        consensus.llmqTypeInstantSend = Consensus::LLMQ_50_60;

        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fRequireRoutableExternalIP = true;
        fMineBlocksOnDemand = false;
        fAllowMultipleAddressesFromGroup = true;
        fAllowMultiplePorts = true;
        nLLMQConnectionRetryTimeout = 60;

        nPoolMinParticipants = 3;
        nPoolNewMinParticipants = 2;
        nPoolMaxParticipants = 5;
        nPoolNewMaxParticipants = 20;
        nFulfilledRequestExpireTime = 5*60; // fulfilled requests expire in 5 minutes

        vSporkAddresses = {"DJ7YjNsc8TYHetDbwsxeNZTzyQzQPKCaxW"};
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
        nPoolNewMinParticipants = 2;
        nPoolMaxParticipants = 5;
        nPoolNewMaxParticipants = 20;

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

        // Regtest DMS addresses start with 'y' (Dash defaults)
        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,140);
        // Regtest DMS script addresses start with '8' or '9' (Dash defaults)
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,19);
        // Regtest private keys start with '9' or 'c' (Bitcoin and Dash defaults)
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        // Regtest DMS BIP32 pubkeys start with 'tpub' (Bitcoin and Dash defaults)
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        // Regtest DMS BIP32 prvkeys start with 'tprv' (Bitcoin and Dash defaults)
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        // Regtest DMS BIP44 coin type is '1' (All coin's testnet default)
        nExtCoinType = 1;

        // long living quorum params
        consensus.llmqs[Consensus::LLMQ_TEST] = llmq_test;
        consensus.llmqTypeChainLocks = Consensus::LLMQ_TEST;
        consensus.llmqTypeInstantSend = Consensus::LLMQ_TEST;
    }
};

static std::unique_ptr<CChainParams> globalChainParams;

const CChainParams &Params() {
    assert(globalChainParams);
    return *globalChainParams;
}

std::unique_ptr<CChainParams> CreateChainParams(const std::string& chain)
{
    if (chain == CBaseChainParams::MAIN)
        return std::unique_ptr<CChainParams>(new CMainParams());
    else if (chain == CBaseChainParams::TESTNET)
        return std::unique_ptr<CChainParams>(new CTestNetParams());
    else if (chain == CBaseChainParams::DEVNET) {
        return std::unique_ptr<CChainParams>(new CDevNetParams());
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

void UpdateLLMQTestParams(int size, int threshold)
{
    globalChainParams->UpdateLLMQTestParams(size, threshold);
}

void UpdateLLMQDevnetParams(int size, int threshold)
{
    globalChainParams->UpdateLLMQDevnetParams(size, threshold);
}
