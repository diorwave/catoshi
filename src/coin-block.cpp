// Catoshi Coin-Block tool
// NOTE: THIS IS SO UGLY RIGHT NOW. Clean later.
// NOTE2: it works, far better than LOCK(cs_main) Send patches.

/*
$ time curl btcnode:8332/rest/block/00000000000000000046c3e600648bc04b7a8da66c59cdb63cb1ea657739a715.bin > block.bin
  % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                 Dload  Upload   Total   Spent    Left  Speed
100  986k  100  986k    0     0  2373k      0 --:--:-- --:--:-- --:--:-- 2370k
real	0m0.435s
user	0m0.007s
sys	0m0.010s

$ time cat block.bin | obj/bitcoin-block > block.json

real	0m0.343s
user	0m0.292s
sys	0m0.045s

 * when run locally it's about 41 times faster to get the block and
   write to a file than it is to parse it out to json. Bitcoind holds
   a GLOBAL LOCK for the WHOLE TIME.
*/

// Copyright (c) 2009-2017 The *coin developers
// where * = (Bit, Lite, PP, Peerunity, Blu, Cat, Solar, URO, ...)
// Previously distributed under the MIT/X11 software license, see the
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
// Copyright (c) 2014-2017 Troy Benjegerdes, under AGPLv3
// Distributed under the Affero GNU General public license version 3
// file COPYING or http://www.gnu.org/licenses/agpl-3.0.html

// note3: if you use this code in a commercial setting and make money
// from it without following NOTE2 above, RMS, the FSF, and other 
// viral copyright zombies will haunt your investors to the end of
// their bankroll

#include <base58.h>
#include <clientversion.h>
#include <coins.h>
#include <consensus/consensus.h>
#include <consensus/validation.h>
#include <core_io.h>
#include <keystore.h>
#include <policy/policy.h>
#include <policy/rbf.h>
#include <primitives/transaction.h>
#include <script/script.h>
#include <script/sign.h>
#include <univalue.h>
#include <util.h>
#include <utilmoneystr.h>
#include <utilstrencodings.h>
//#include <iostream>
#include <streams.h>

#include <stdio.h>

#include <boost/algorithm/string.hpp>

static bool fCreateBlank;
static std::map<std::string,UniValue> registers;
static const int CONTINUE_EXECUTION=-1;

//
// This function returns either one of EXIT_ codes when it's expected to stop the process or
// CONTINUE_EXECUTION when it's expected to continue further.
//
static int AppInitBlock(int argc, char* argv[])
{
    //
    // Parameters
    //
    gArgs.ParseParameters(argc, argv);

    // Check for -testnet or -regtest parameter (Params() calls are only valid after this clause)
    try {
        SelectParams(ChainNameFromCommandLine());
    } catch (const std::exception& e) {
        fprintf(stderr, "Error: %s\n", e.what());
        return EXIT_FAILURE;
    }

    fCreateBlank = gArgs.GetBoolArg("-create", false);

    if (gArgs.IsArgSet("-?") || gArgs.IsArgSet("-h") || gArgs.IsArgSet("-help") || gArgs.IsArgSet("-version"))
    {
        // First part of help message is specific to this utility
        std::string strUsage = strprintf(_("%s " BRAND_lower "-block utility version"), _(PACKAGE_NAME)) + " " + FormatFullVersion() + "\n";
        if (gArgs.IsArgSet("-version")){
		fprintf(stdout, "%s", strUsage.c_str());
	}
	strUsage += "\n" +
            _("Usage:") + "\n" +
              "  curl localhost:8332/rest/block/<hash>.bin | " BRAND_lower " -block | <json receiver>\n" +
              "\n";

        fprintf(stdout, "%s", strUsage.c_str());

        strUsage = HelpMessageGroup(_("Options:"));
        strUsage += HelpMessageOpt("-?", _("This help message"));
        AppendParamsHelpMessages(strUsage);

        fprintf(stdout, "%s", strUsage.c_str());

        if (argc < 2) {
            fprintf(stderr, "Error: too few parameters\n");
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
    return CONTINUE_EXECUTION;
}


static void OutputTxJSON(const CTransaction& tx)
{
    UniValue entry(UniValue::VOBJ);
    TxToUniv(tx, uint256(), entry);

    std::string jsonOutput = entry.write(4);
    fprintf(stdout, "%s\n", jsonOutput.c_str());
}

static void OutputTxHash(const CTransaction& tx)
{
    std::string strHexHash = tx.GetHash().GetHex(); // the hex-encoded transaction hash (aka the transaction id)

    fprintf(stdout, "%s\n", strHexHash.c_str());
}

static void OutputTxHex(const CTransaction& tx)
{
    std::string strHex = EncodeHexTx(tx);

    fprintf(stdout, "%s\n", strHex.c_str());
}

static void OutputTx(const CTransaction& tx)
{
    if (gArgs.GetBoolArg("-json", false))
        OutputTxJSON(tx);
    else if (gArgs.GetBoolArg("-txid", false))
        OutputTxHash(tx);
    else
        OutputTxHex(tx);
}

/* TODO: why can haz all this duplicated code ? */
static const unsigned int DEFAULT_RPC_SERIALIZE_VERSION = 1;

int RPCSerializationFlags()
{
    int flag = 0;
    if (gArgs.GetArg("-rpcserialversion", DEFAULT_RPC_SERIALIZE_VERSION) == 0)
        flag |= SERIALIZE_TRANSACTION_NO_WITNESS;
    return flag;
}

/* TODO: use common code with main */
UniValue blockToJSON(const CBlock& block, bool txDetails, bool txHex)
{
    UniValue result(UniValue::VOBJ);
    result.pushKV("hash", block.GetHash().GetHex());
    int confirmations = -1;
    // Only report confirmations if the block is on the main chain
    result.pushKV("strippedsize", (int)::GetSerializeSize(block, SER_NETWORK, PROTOCOL_VERSION | SERIALIZE_TRANSACTION_NO_WITNESS));
    result.pushKV("size", (int)::GetSerializeSize(block, SER_NETWORK, PROTOCOL_VERSION));
    result.pushKV("weight", (int)::GetBlockWeight(block));
    result.pushKV("version", block.nVersion);
    result.pushKV("versionHex", strprintf("%08x", block.nVersion));
    result.pushKV("merkleroot", block.hashMerkleRoot.GetHex());
    UniValue txs(UniValue::VARR);
    for(const auto& tx : block.vtx)
    {
        if(txDetails) {
            UniValue objTx(UniValue::VOBJ);
            TxToUniv(*tx, uint256(), objTx, true, RPCSerializationFlags());
            txs.push_back(objTx);
        } else if(txHex) {
            UniValue objTx(UniValue::VOBJ);
            objTx.pushKV("txid", tx->GetHash().GetHex());
            objTx.pushKV("hash", tx->GetWitnessHash().GetHex());
            objTx.pushKV("hex", EncodeHexTx(*tx, RPCSerializationFlags()));
            txs.push_back(objTx);
        } else
            txs.push_back(tx->GetHash().GetHex());
    }
    result.pushKV("tx", txs);
    result.pushKV("time", block.GetBlockTime());
    result.pushKV("nonce", (uint64_t)block.nNonce);
    result.pushKV("bits", strprintf("%08x", block.nBits));

    return result;
}

static int CommandLineRawTx(int argc, char* argv[])
{
    std::string strPrint;
    int nRet = 0;
	
	CBlock block;
	CAutoFile in(stdin, SER_NETWORK, PROTOCOL_VERSION || RPCSerializationFlags());
	//CBlockIndex fakeindex;
	//std::cin >> block;
	in >> block;

	auto json = blockToJSON(block, true, false);
	std::string strJSON = json.write() + "\n";

	std::cout << strJSON;

    if (strPrint != "") {
        fprintf((nRet == 0 ? stdout : stderr), "%s\n", strPrint.c_str());
    }
    return nRet;
}

int main(int argc, char* argv[])
{
    SetupEnvironment();

    try {
        int ret = AppInitBlock(argc, argv);
        if (ret != CONTINUE_EXECUTION)
            return ret;
    }
    catch (const std::exception& e) {
        PrintExceptionContinue(&e, "AppInitRawTx()");
        return EXIT_FAILURE;
    } catch (...) {
        PrintExceptionContinue(nullptr, "AppInitRawTx()");
        return EXIT_FAILURE;
    }

    int ret = EXIT_FAILURE;
    try {
        ret = CommandLineRawTx(argc, argv);
    }
    catch (const std::exception& e) {
        PrintExceptionContinue(&e, "CommandLineRawTx()");
    } catch (...) {
        PrintExceptionContinue(nullptr, "CommandLineRawTx()");
    }
    return ret;
}
