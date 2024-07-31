#include <gtest/gtest.h>

#include <Lokal.hpp>

using namespace LokalSo;

TEST(Tunnel, CreateTunnel) {
	std::string address{"127.0.0.1:9999"};
	std::string name{"Gin Test"};
	std::string lanAddress{"backend.local"};
	std::string publicAddress{"mybackend2.k.lokal-so.site"};

	std::string ret;
	Lokal lk;

	Tunnel *tun = lk.newTunnel()
			  ->setName(name)
			  ->setTunnelType(Lokal::TunnelTypeHTTP)
			  ->setLANAddress(lanAddress)
			  ->setLocalAddress(address)
			  ->setPublicAddress(publicAddress)
			  ->setInpsection(true)
			  ->showStartupBanner()
			  ->ignoreDuplicate();
	ASSERT_EQ(tun->getName(), name);
	ASSERT_EQ(tun->getTunnelType(), Lokal::TunnelTypeHTTP);
	ASSERT_EQ(tun->getLANAddress(), lanAddress);
	ASSERT_EQ(tun->getLocalAddress(), address);
	ASSERT_EQ(tun->getAddressPublic(), publicAddress);
	ASSERT_TRUE(tun->getInspect());
	ASSERT_TRUE(tun->getIgnoreDuplicate());
}
