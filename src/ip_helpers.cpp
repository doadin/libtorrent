/*

Copyright (c) 2007-2018, Arvid Norberg
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in
      the documentation and/or other materials provided with the distribution.
    * Neither the name of the author nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

*/

#include "libtorrent/config.hpp"
#include "libtorrent/address.hpp"
#include "libtorrent/error_code.hpp"
#include "libtorrent/io_context.hpp"
#include "libtorrent/socket.hpp"
#include "libtorrent/aux_/ip_helpers.hpp"

namespace libtorrent {
namespace aux {

	bool is_ip_address(std::string const& host)
	{
		error_code ec;
		make_address(host, ec);
		return !ec;
	}

	bool is_local(address const& a)
	{
		TORRENT_TRY {
			if (a.is_v6())
			{
				// NOTE: site local is deprecated but by
				// https://www.ietf.org/rfc/rfc3879.txt:
				// routers SHOULD be configured to prevent
				// routing of this prefix by default.

				address_v6 const a6 = a.to_v6();
				return a6.is_loopback()
					|| a6.is_link_local()
					|| a6.is_site_local()
					|| a6.is_multicast_link_local()
					|| a6.is_multicast_site_local()
					//  fc00::/7, unique local address
					|| (a6.to_bytes()[0] & 0xfe) == 0xfc;
			}
			address_v4 a4 = a.to_v4();
			std::uint32_t const ip = a4.to_uint();
			return ((ip & 0xff000000) == 0x0a000000 // 10.x.x.x
				|| (ip & 0xfff00000) == 0xac100000 // 172.16.x.x
				|| (ip & 0xffff0000) == 0xc0a80000 // 192.168.x.x
				|| (ip & 0xffff0000) == 0xa9fe0000 // 169.254.x.x
				|| (ip & 0xff000000) == 0x7f000000); // 127.x.x.x
		} TORRENT_CATCH(std::exception const&) { return false; }
	}

	bool is_loopback(address const& addr)
	{
		TORRENT_TRY {
			if (addr.is_v4())
				return addr.to_v4() == address_v4::loopback();
			else
				return addr.to_v6() == address_v6::loopback();
		} TORRENT_CATCH(std::exception const&) { return false; }
	}

	bool is_any(address const& addr)
	{
		TORRENT_TRY {
		if (addr.is_v4())
			return addr.to_v4() == address_v4::any();
		else if (addr.to_v6().is_v4_mapped())
			return (make_address_v4(v4_mapped, addr.to_v6()) == address_v4::any());
		else
			return addr.to_v6() == address_v6::any();
		} TORRENT_CATCH(std::exception const&) { return false; }
	}

	bool is_teredo(address const& addr)
	{
		TORRENT_TRY {
			if (!addr.is_v6()) return false;
			static const std::uint8_t teredo_prefix[] = {0x20, 0x01, 0, 0};
			address_v6::bytes_type b = addr.to_v6().to_bytes();
			return std::memcmp(b.data(), teredo_prefix, 4) == 0;
		} TORRENT_CATCH(std::exception const&) { return false; }
	}

	address ensure_v6(address const& a)
	{
		return a == address_v4() ? address_v6() : a;
	}

}
}

