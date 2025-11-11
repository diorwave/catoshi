# Copyright (c) 2009-2010 Satoshi Nakamoto
# Copyright (c) 2011-201? The *Coin developers
# where * = (Lite, PP, Peerunity, Blu, Cat, Solar, URO, ...)
# Previously distributed under the MIT/X11 software license, see the
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
# This file is part of the Catoshi Codecoin financial cryptography software
# Copyright (c) 2014-2018 7 Elements LLC AGPLv3
# Distributed under the Affero GNU General public license version 3
# file COPYING or http://www.gnu.org/licenses/agpl-3.0.html

TARGETOS := $(shell uname -s)

include make.$(TARGETOS).mk

# :=0 --> UPnP support turned off by default at runtime
# :=1 --> UPnP support turned on by default at runtime
# :=- --> No UPnP support - miniupnp not required
USE_UPNP:=-

# :=1 --> Enable IPv6 support
# :=0 --> Disable IPv6 support
USE_IPV6:=1

# Build wallet
ENABLE_WALLET:=1

# Enable ZMQ support
ENABLE_ZMQ:=1

# Enable tests
ENABLE_TESTS:=1

ARCH=$(shell arch)

# :=1 --> enable SSE
# :=0 --> disable SSE
ifeq ($(ARCH), i386)
USE_SSE2:=1
else ifeq ($(ARCH), x86_64)
USE_SSE2:=1
else
USE_SSE2:=0
endif

LINK:=$(CXX)

#COIN_BRAND:=bitcoin
COIN_BRAND:=foodchain

prefix = /usr/

DEFS=-DBOOST_SPIRIT_THREADSAFE -D_FILE_OFFSET_BITS=64 -DBRAND_$(COIN_BRAND) $(OS_DEFS)

#debian jessie libboost-filesystem1.55 needs this
DEFS += -DBOOST_NO_CXX11_SCOPED_ENUMS

# Modern Boost compatibility - allow old boost::bind placeholders in global namespace
DEFS += -DBOOST_BIND_GLOBAL_PLACEHOLDERS

OBJDIR = obj/$(COIN_BRAND)
TESTDIR = $(OBJDIR)/test
SUBDIRS = \
	$(OBJDIR)/crypto \
	$(OBJDIR)/primitives \
	$(OBJDIR)/wallet \
	$(OBJDIR)/script \
	$(OBJDIR)/compat \
	$(OBJDIR)/consensus \
	$(OBJDIR)/policy \
	$(OBJDIR)/support \
	$(OBJDIR)/rpc \
	$(OBJDIR)/zmq \
	$(OBJDIR)/$(COIN_BRAND)

DEFS += $(addprefix -I,$(CURDIR) $(CURDIR)/$(OBJDIR) $(OS_EXTRA_INCLUDE_PATH))
LIBS = -lunivalue
BOOST_LIBS = \
   -l boost_system$(BOOST_LIB_SUFFIX) \
   -l boost_filesystem$(BOOST_LIB_SUFFIX) \
   -l boost_program_options$(BOOST_LIB_SUFFIX) \
   -l boost_thread$(BOOST_LIB_SUFFIX) \
   -l boost_chrono$(BOOST_LIB_SUFFIX) 

TESTDEFS = -DTEST_DATA_DIR=$(abspath test/data) -I$(abspath $(OBJDIR)/test)



LMODE = dynamic
LMODE2 = dynamic
ifdef STATIC
	LMODE = static
	ifeq (${STATIC}, all)
		LMODE2 = static
	endif
else
	TESTDEFS += -DBOOST_TEST_DYN_LINK
endif

# for boost 1.37, add -mt to the boost libraries
D_LIBS = $(LIBS) $(BOOST_LIBS) \
   -l db_cxx$(BDB_LIB_SUFFIX) \
   -l secp256k1 \
   -l event \
   -l event_pthreads \
   -l ssl \
   -l crypto \

#TESTLIBS += \
# -Wl,-B$(LMODE) \
#   -l boost_unit_test_framework$(BOOST_LIB_SUFFIX) \
#   -l leveldb \
#   -l memenv

TESTLIBS += -lboost_unit_test_framework$(BOOST_LIB_SUFFIX) -lmemenv 

ifndef USE_UPNP
	override USE_UPNP = -
endif
ifneq (${USE_UPNP}, -)
	D_LIBS += -l miniupnpc
	DEFS += -DUSE_UPNP=$(USE_UPNP)
endif

ifneq (${USE_IPV6}, -)
	DEFS += -DUSE_IPV6=$(USE_IPV6)
endif

D_LIBS+= \
   -l z \
   -l dl \
   -l pthread

#TODO: simplify above
TX_LIBS = $(LIBS) $(BOOST_LIBS) -lsecp256k1 -lssl -lcrypto -lpthread
BLK_LIBS = $(LIBS) $(BOOST_LIBS) -lsecp256k1 -lssl -lcrypto -lpthread
CLI_LIBS = $(LIBS) $(BOOST_LIBS) -lsecp256k1 -lssl -lcrypto -levent

# Hardening
# Make some classes of vulnerabilities unexploitable in case one is discovered.
#

    # Stack Canaries
    # Put numbers at the beginning of each stack frame and check that they are the same.
    # If a stack buffer if overflowed, it writes over the canary number and then on return
    # when that number is checked, it won't be the same and the program will exit with
    # a "Stack smashing detected" error instead of being exploited.
    HARDENING+=-fstack-protector-all -Wstack-protector

    # Make some important things such as the global offset table read only as soon as
    # the dynamic linker is finished building it. This will prevent overwriting of addresses
    # which would later be jumped to.
    # moved to make.Linux.mk
    #LDHARDENING+=-Wl,-z,relro -Wl,-z,now

    # Build position independent code to take advantage of Address Space Layout Randomization
    # offered by some kernels.
    # see doc/build-unix.txt for more information.
    PIE=1
    ifdef PIE
        HARDENING+=-fPIE
        LDHARDENING+=-pie
    endif

    # -D_FORTIFY_SOURCE=2 does some checking for potentially exploitable code patterns in
    # the source such overflowing a statically defined buffer.
    HARDENING+=-D_FORTIFY_SOURCE=2
#


DEBUGFLAGS=-g

# CXXFLAGS can be specified on the make command line, so we use xCXXFLAGS that only
# adds some defaults in front. Unfortunately, CXXFLAGS=... $(CXXFLAGS) does not work.
xCXXFLAGS=-O2 -std=c++11 -pthread -Wall -Wextra -Wformat -Wformat-security -Wno-unused-parameter \
    $(DEBUGFLAGS) $(DEFS) $(HARDENING) $(CXXFLAGS)

# LDFLAGS can be specified on the make command line, so we use xLDFLAGS that only
# adds some defaults in front. Unfortunately, LDFLAGS=... $(LDFLAGS) does not work.
xLDFLAGS=$(LDHARDENING) $(OS_LDFLAGS) $(LDFLAGS)

STAKE_OBJS = \
    $(OBJDIR)/kernel.o \
    $(OBJDIR)/kernelrecord.o

UTIL_OBJS= \
    $(OBJDIR)/support/lockedpool.o \
    $(OBJDIR)/chainbaseparams.o \
    $(OBJDIR)/clientversion.o \
    $(OBJDIR)/compat/glibc_sanity.o \
    $(OBJDIR)/compat/glibcxx_sanity.o \
    $(OBJDIR)/compat/strnlen.o \
    $(OBJDIR)/fs.o \
    $(OBJDIR)/random.o \
    $(OBJDIR)/rpc/protocol.o \
    $(OBJDIR)/rpc/util.o \
    $(OBJDIR)/support/cleanse.o \
    $(OBJDIR)/sync.o \
    $(OBJDIR)/uint256.o \
    $(OBJDIR)/threadinterrupt.o \
    $(OBJDIR)/util.o \
    $(OBJDIR)/utilmoneystr.o \
    $(OBJDIR)/utilstrencodings.o \
    $(OBJDIR)/utiltime.o 

CRYPTO_OBJS = \
    $(OBJDIR)/crypto/aes.o \
    $(OBJDIR)/crypto/chacha20.o \
    $(OBJDIR)/crypto/hmac_sha256.o \
    $(OBJDIR)/crypto/hmac_sha512.o \
    $(OBJDIR)/crypto/ripemd160.o \
    $(OBJDIR)/crypto/sha256.o \
    $(OBJDIR)/crypto/sha512.o \
    $(OBJDIR)/crypto/sha1.o

INDEX_OBJS = \
    $(OBJDIR)/index.o \
    $(OBJDIR)/rpc/index.o

COMMON_OBJS = \
    $(OBJDIR)/arith_uint256.o \
    $(OBJDIR)/base58.o \
    $(OBJDIR)/bech32.o \
    $(OBJDIR)/$(COIN_BRAND)/params.o \
    $(OBJDIR)/coins.o \
    $(OBJDIR)/compressor.o \
    $(OBJDIR)/consensus/merkle.o \
    $(OBJDIR)/core_read.o \
    $(OBJDIR)/core_write.o \
    $(OBJDIR)/hash.o \
    $(OBJDIR)/key.o \
    $(OBJDIR)/keystore.o \
    $(OBJDIR)/netaddress.o \
    $(OBJDIR)/netbase.o \
    $(OBJDIR)/policy/feerate.o \
    $(OBJDIR)/primitives/block.o \
    $(OBJDIR)/primitives/transaction.o \
    $(OBJDIR)/protocol.o \
    $(OBJDIR)/pubkey.o \
    $(OBJDIR)/scheduler.o \
    $(OBJDIR)/script/interpreter.o \
    $(OBJDIR)/script/script.o \
    $(OBJDIR)/script/script_error.o \
    $(OBJDIR)/script/sign.o \
    $(OBJDIR)/script/standard.o \
    $(OBJDIR)/warnings.o


#put the longest compiling things first for parallel make
D_OBJS= \
    $(OBJDIR)/validation.o \
    $(OBJDIR)/init.o \
    $(OBJDIR)/rpc/server.o \
    $(UTIL_OBJS) \
    $(CRYPTO_OBJS) \
    $(COMMON_OBJS) \
    $(OBJDIR)/addrman.o \
    $(OBJDIR)/addrdb.o \
    $(OBJDIR)/blockencodings.o \
    $(OBJDIR)/chain.o \
    $(OBJDIR)/checkpoints.o \
    $(OBJDIR)/consensus/tx_verify.o \
    $(OBJDIR)/dbwrapper.o \
    $(OBJDIR)/httprpc.o \
    $(OBJDIR)/httpserver.o \
    $(OBJDIR)/merkleblock.o \
    $(OBJDIR)/net.o \
    $(OBJDIR)/net_processing.o \
    $(OBJDIR)/noui.o \
    $(OBJDIR)/policy/fees.o \
    $(OBJDIR)/policy/policy.o \
    $(OBJDIR)/policy/rbf.o \
    $(OBJDIR)/pow.o \
    $(OBJDIR)/$(COIN_BRAND)/trust.o \
    $(OBJDIR)/rest.o \
    $(OBJDIR)/rpc/blockchain.o \
    $(OBJDIR)/rpc/misc.o \
    $(OBJDIR)/rpc/net.o \
    $(OBJDIR)/rpc/rawtransaction.o \
    $(OBJDIR)/rpc/safemode.o \
    $(OBJDIR)/rpc/server.o \
    $(OBJDIR)/rpc/rawtransaction.o \
    $(OBJDIR)/script/bitcoinconsensus.o \
    $(OBJDIR)/script/sigcache.o \
    $(OBJDIR)/timedata.o \
    $(OBJDIR)/torcontrol.o \
    $(OBJDIR)/txdb.o \
    $(OBJDIR)/txmempool.o \
    $(OBJDIR)/threadinterrupt.o \
    $(OBJDIR)/ui_interface.o \
    $(OBJDIR)/validationinterface.o \
    $(OBJDIR)/versionbits.o \
    $(OBJDIR)/undo.o \
    $(OBJDIR)/warnings.o \
    $(OBJDIR)/bloom.o 

#MINER_OBJS
D_OBJS += \
    $(OBJDIR)/rpc/mining.o \
    $(OBJDIR)/miner.o

WALLET_OBJS = \
    $(OBJDIR)/wallet/crypter.o \
    $(OBJDIR)/wallet/db.o \
    $(OBJDIR)/wallet/wallet.o \
    $(OBJDIR)/wallet/walletdb.o \
    $(OBJDIR)/wallet/walletutil.o \
    $(OBJDIR)/wallet/feebumper.o \
    $(OBJDIR)/wallet/fees.o \
    $(OBJDIR)/wallet/init.o \
    $(OBJDIR)/wallet/rpcdump.o \
    $(OBJDIR)/wallet/rpcwallet.o \
    $(OBJDIR)/policy/rbf.o \
    $(OBJDIR)/script/ismine.o 

ZMQ_OBJS = \
    $(OBJDIR)/zmq/zmqabstractnotifier.o \
    $(OBJDIR)/zmq/zmqnotificationinterface.o \
    $(OBJDIR)/zmq/zmqpublishnotifier.o

X11_OBJS = \
    $(OBJDIR)/cubehash.o \
    $(OBJDIR)/luffa.o \
    $(OBJDIR)/aes_helper.o \
    $(OBJDIR)/echo.o \
    $(OBJDIR)/shavite.o \
    $(OBJDIR)/simd.o \
    $(OBJDIR)/blake.o \
    $(OBJDIR)/bmw.o \
    $(OBJDIR)/groestl.o \
    $(OBJDIR)/jh.o \
    $(OBJDIR)/keccak.o \
    $(OBJDIR)/skein.o

SCRYPT_OBJS = $(OBJDIR)/crypto/scrypt.o

ifeq ($(ENABLE_WALLET),1)
DEFS+= -DENABLE_WALLET
D_OBJS+= $(WALLET_OBJS)
D_LIBS += -lzmq
endif

ifeq ($(ENABLE_ZMQ),1)
DEFS+= -DENABLE_ZMQ
D_OBJS+= $(ZMQ_OBJS)
endif

DEFS+= -DFEATURE_INDEX
D_OBJS+=$(INDEX_OBJS)

ifeq ($(USE_SSE2),1)
DEFS += -DUSE_SSE2
OBJS_SSE2= $(OBJDIR)/crypto/scrypt-sse2.o
SCRYPT_OBJS += $(OBJS_SSE2)
endif

ifeq ($(COIN_BRAND),litecoin)
DEFS += -DUSE_SCRYPT
COMMON_OBJS += $(SCRYPT_OBJS)
endif
#ifeq ($(COIN_BRAND),grantcoin)
#COMMON_OBJS += $(STAKE_OBJS)
#endif
ifeq ($(COIN_BRAND),grantstake)
COMMON_OBJS += $(STAKE_OBJS)
endif
ifeq ($(COIN_BRAND),uro)
COMMON_OBJS += $(X11_OBJS)
endif
ifeq ($(COIN_BRAND),givecoin)
COMMON_OBJS += $(X11_OBJS)
endif
ifeq ($(COIN_BRAND),hamburger)
DEFS += -DUSE_SCRYPT
COMMON_OBJS += $(SCRYPT_OBJS) $(STAKE_OBJS)
endif
ifeq ($(COIN_BRAND),kittycoin)
DEFS += -DUSE_SCRYPT
COMMON_OBJS += $(SCRYPT_OBJS)
endif
ifeq ($(COIN_BRAND),bluecoin)
COMMON_OBJS += $(SCRYPT_OBJS) $(X11_OBJS) $(STAKE_OBJS)
endif

TX_OBJS= $(UTIL_OBJS) $(COMMON_OBJS) $(CRYPTO_OBJS) $(OBJDIR)/indexhack.o $(OBJDIR)/bitcoin-tx.o
BLK_OBJS= $(UTIL_OBJS) $(COMMON_OBJS) $(CRYPTO_OBJS) $(OBJDIR)/indexhack.o $(OBJDIR)/coin-block.o
# should have just use CRYPTO_OBJS and COMMON_OBJS but let's keep it simple
#CLI_OBJS= $(UTIL_OBJS) $(COMMON_OBJS) $(CRYPTO_OBJS) $(OBJDIR)/rpc/client.o $(OBJDIR)/bitcoin-cli.o
#ugh, index.
CLI_OBJS= $(UTIL_OBJS) $(COMMON_OBJS) $(CRYPTO_OBJS) \
	  $(OBJDIR)/indexhack.o $(OBJDIR)/rpc/client.o $(OBJDIR)/bitcoin-cli.o

#needed to create directories
ALL_OBJS= $(STAKE_OBJS) $(UTIL_OBJS) $(CRYPTO_OBJS) $(INDEX_OBJS) $(COMMON_OBJS) $(D_OBJS) $(TX_OBJS) $(CLI_OBJS) 

MAN= obj/$(COIN_BRAND)d.1 obj/$(COIN_BRAND)-cli.1 obj/$(COIN_BRAND)-tx.1 obj/$(COIN_BRAND)-block

BINS= obj/$(COIN_BRAND)d obj/$(COIN_BRAND)-cli obj/$(COIN_BRAND)-tx obj/$(COIN_BRAND)-block
	
#tricksy: man pages require binaries
all: $(MAN)

install: $(BINS)
	mkdir -p $(DESTDIR)$(prefix)/bin
	install -D $^ $(DESTDIR)$(prefix)/bin/

test_litecoin:
	$(MAKE) COIN_BRAND=litecoin test

test_bitcoin:	
	$(MAKE) COIN_BRAND=bitcoin test

test_grantcoin:
	$(MAKE) COIN_BRAND=grantcoin test

regression: test_litecoin test_bitcoin test_grantcoin FORCE
	

$(TESTDIR) $(SUBDIRS):
	mkdir -p $@

$(ALL_OBJS): | $(SUBDIRS)

#
# LevelDB support
#
MAKEOVERRIDES =
D_LIBS += -lleveldb

# auto-generated dependencies:
-include $(OBJDIR)/*.P
-include $(OBJDIR)/test/*.P

ifeq ($(V),1)
V_CXX=
V_LINK=
else
V_CXX=@echo  "  CXX\t$(COIN_BRAND)\t$(CXX)    $<" ;
V_LINK=@echo "  LINK\t$(COIN_BRAND)\t$(LINK)    $@" ;
endif

$(OBJDIR)/build.h: FORCE
	/bin/sh ../share/genbuild.sh $(OBJDIR)/build.h
clientversion.cpp: $(OBJDIR)/build.h
DEFS += -DHAVE_BUILD_INFO -DHAVE_SYS_SELECT_H

obj/%.1: obj/%
	# run first so we see genesis block search output
	$< -h > /dev/null
	help2man $< > $@

$(OBJDIR)/%-sse2.o: %-sse2.cpp
	$(V_CXX) $(CXX) -c $(xCXXFLAGS) -msse2 -MMD -MF $(@:%.o=%.d) -o $@ $<
	@cp $(@:%.o=%.d) $(@:%.o=%.P); \
	  sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	      -e '/^$$/ d' -e 's/$$/ :/' < $(@:%.o=%.d) >> $(@:%.o=%.P); \
	  rm -f $(@:%.o=%.d)

$(OBJDIR)/%.o: %.c
	$(V_CXX) $(CXX) -c $(xCXXFLAGS) -fpermissive -MMD -MF $(@:%.o=%.d) -o $@ $<
	@cp $(@:%.o=%.d) $(@:%.o=%.P); \
	  sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	      -e '/^$$/ d' -e 's/$$/ :/' < $(@:%.o=%.d) >> $(@:%.o=%.P); \
	  rm -f $(@:%.o=%.d)

#must be before below rule...
$(OBJDIR)/test/%.o: test/%.cpp
	$(V_CXX) $(CXX) -c $(TESTDEFS) $(xCXXFLAGS) -MMD -MF $(@:%.o=%.d) -o $@ $<
	@cp $(@:%.o=%.d) $(@:%.o=%.P); \
	  sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	      -e '/^$$/ d' -e 's/$$/ :/' < $(@:%.o=%.d) >> $(@:%.o=%.P); \
	  rm -f $(@:%.o=%.d)

$(OBJDIR)/%.o: %.cpp
	$(V_CXX) $(CXX) -c $(xCXXFLAGS) -MMD -MF $(@:%.o=%.d) -o $@ $<
	@cp $(@:%.o=%.d) $(@:%.o=%.P); \
	  sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	      -e '/^$$/ d' -e 's/$$/ :/' < $(@:%.o=%.d) >> $(@:%.o=%.P); \
	  rm -f $(@:%.o=%.d)

obj/$(COIN_BRAND)d: $(D_OBJS:$(OBJDIR)/%=$(OBJDIR)/%) $(OBJDIR)/bitcoind.o
	$(V_LINK) $(LINK) $(xCXXFLAGS) -o $@ $^ $(xLDFLAGS) $(D_LIBS)

obj/$(COIN_BRAND)-tx: $(TX_OBJS:$(OBJDIR)/%=$(OBJDIR)/%)
	$(V_LINK) $(LINK) $(xCXXFLAGS) -o $@ $^ $(xLDFLAGS) $(TX_LIBS)

obj/$(COIN_BRAND)-block: $(BLK_OBJS:$(OBJDIR)/%=$(OBJDIR)/%)
	$(V_LINK) $(LINK) $(xCXXFLAGS) -o $@ $^ $(xLDFLAGS) $(BLK_LIBS)

obj/$(COIN_BRAND)-cli: $(CLI_OBJS:$(OBJDIR)/%=$(OBJDIR)/%)
	$(V_LINK) $(LINK) $(xCXXFLAGS) -o $@ $^ $(xLDFLAGS) $(CLI_LIBS)


ifeq ($(ENABLE_TESTS),1)
MKDIR_P := mkdir -p
HEXDUMP := hexdump
SED := sed
include $(COIN_BRAND)/Makefile.test.include
include Makefile.test.include

endif

clean:
	-rm -f $(OBJDIR)/*.o $(OBJDIR)/*/*.o
	-rm -f $(OBJDIR)/*.P $(OBJDIR)/*/*.P
	-rm -f $(OBJDIR)/build.h $(OBJDIR)/*/*/*.json.h
	-rm -f $(MAN) $(BINS) obj/test_$(COIN_BRAND)

realclean:
	rm -rf obj

FORCE:


