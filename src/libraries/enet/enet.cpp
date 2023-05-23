/**
 *
 * Copyright (C) 2014 by Leaf Corcoran
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

#ifdef _WIN32
#define NOMINMAX
#endif

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#include <cstdio>
#include <cstddef>
#include <algorithm>

extern "C" {
#define LUA_COMPAT_ALL
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include <enet/enet.h>
}

#define check_host(l, idx)\
	*(ENetHost**)luaL_checkudata(l, idx, "enet_host")

#define check_peer(l, idx)\
	*(ENetPeer**)luaL_checkudata(l, idx, "enet_peer")

/**
 * Parse address string, eg:
 *	*:5959
 *	127.0.0.1:*
 *	website.com:8080
 */
static void parse_address(lua_State *l, const char *addr_str, ENetAddress *address) {
	int host_i = 0, port_i = 0;
	char host_str[128] = {0};
	char port_str[32] = {0};
	int scanning_port = 0;

	char *c = (char *)addr_str;

	while (*c != 0) {
		if (host_i >= 128 || port_i >= 32 ) luaL_error(l, "Hostname too long");
		if (scanning_port) {
			port_str[port_i++] = *c;
		} else {
			if (*c == ':') {
				scanning_port = 1;
			} else {
				host_str[host_i++] = *c;
			}
		}
		c++;
	}
	host_str[host_i] = '\0';
	port_str[port_i] = '\0';

	if (host_i == 0) luaL_error(l, "Failed to parse address");
	if (port_i == 0) luaL_error(l, "Missing port in address");

	if (strcmp("*", host_str) == 0) {
		address->host = ENET_HOST_ANY;
	} else {
		if (enet_address_set_host(address, host_str) != 0) {
			luaL_error(l, "Failed to resolve host name");
		}
	}

	if (strcmp("*", port_str) == 0) {
		address->port = ENET_PORT_ANY;
	} else {
		address->port = atoi(port_str);
	}
}

/**
 * Find the index of a given peer for which we only have the pointer.
 */
static size_t find_peer_index(lua_State *l, ENetHost *enet_host, ENetPeer *peer) {
	size_t peer_index;
	for (peer_index = 0; peer_index < enet_host->peerCount; peer_index++) {
		if (peer == &(enet_host->peers[peer_index]))
			return peer_index;
	}

	luaL_error (l, "enet: could not find peer id!");

	return peer_index;
}

// VS2013 doesn't support alignof
#if defined(_MSC_VER) && _MSC_VER <= 1800
#define ENET_ALIGNOF(x) __alignof(x)
#else
#define ENET_ALIGNOF(x) alignof(x)
#endif

static bool supports_full_lightuserdata(lua_State *L)
{
	static bool checked = false;
	static bool supported = false;

	if (sizeof(void*) == 4)
		// 32-bit platforms always supports full-lightuserdata.
		return true;

	if (!checked)
	{
		lua_pushcclosure(L, [](lua_State* L) -> int
		{
			// Try to push pointer with all bits set.
			lua_pushlightuserdata(L, (void*)(~((size_t)0)));
			return 1;
		}, 0);

		supported = lua_pcall(L, 0, 1, 0) == 0;
		checked = true;

		lua_pop(L, 1);
	}

	return supported;
}

static uintptr_t compute_peer_key(lua_State *L, ENetPeer *peer)
{
	// ENet peers are be allocated on the heap in an array. Lua numbers
	// (doubles) can store all possible integers up to 2^53. We can store
	// pointers that use more than 53 bits if their alignment is guaranteed to
	// be more than 1. For example an alignment requirement of 8 means we can
	// shift the pointer's bits by 3.

	// Please see these for the reason of this ternary operator:
	// * https://github.com/love2d/love/issues/1916
	// * https://github.com/love2d/love/commit/4ab9a1ce8c
	const size_t minalign = sizeof(void*) == 8 ? std::min(ENET_ALIGNOF(ENetPeer), ENET_ALIGNOF(std::max_align_t)) : 1;
	uintptr_t key = (uintptr_t) peer;

	if ((key & (minalign - 1)) != 0)
	{
		luaL_error(L, "Cannot push enet peer to Lua: unexpected alignment "
				   "(pointer is %p but alignment should be %d)", peer, minalign);
	}

	static const size_t shift = (size_t) log2((double) minalign);

	return key >> shift;
}

static void push_peer_key(lua_State *L, uintptr_t key)
{
	// If full 64-bit lightuserdata is supported (or it's 32-bit platform),
	// always use that. Otherwise, if the key is smaller than 2^53 (which is
	// integer precision for double datatype) on 64-bit platform, then push
	// number. Otherwise, throw error.
	if (supports_full_lightuserdata(L))
		lua_pushlightuserdata(L, (void*) key);
#if UINTPTR_MAX == 0xffffffffffffffff
	else if (key > 0x20000000000000ULL) // 2^53
		luaL_error(L, "Cannot push enet peer to Lua: pointer value %p is too large", key);
#endif
	else
		lua_pushnumber(L, (lua_Number) key);
}

static void push_peer(lua_State *l, ENetPeer *peer) {
	uintptr_t key = compute_peer_key(l, peer);

	// try to find in peer table
	lua_getfield(l, LUA_REGISTRYINDEX, "enet_peers");
	push_peer_key(l, key);
	lua_gettable(l, -2);

	if (lua_isnil(l, -1)) {
		// printf("creating new peer\n");
		lua_pop(l, 1);

		*(ENetPeer**)lua_newuserdata(l, sizeof(void*)) = peer;
		luaL_getmetatable(l, "enet_peer");
		lua_setmetatable(l, -2);

		push_peer_key(l, key);
		lua_pushvalue(l, -2);

		lua_settable(l, -4);
	}
	lua_remove(l, -2); // remove enet_peers
}

static void push_event(lua_State *l, ENetEvent *event) {
	lua_newtable(l); // event table

	if (event->peer) {
		push_peer(l, event->peer);
		lua_setfield(l, -2, "peer");
	}

	switch (event->type) {
		case ENET_EVENT_TYPE_CONNECT:
			lua_pushinteger(l, event->data);
			lua_setfield(l, -2, "data");

			lua_pushstring(l, "connect");
			break;
		case ENET_EVENT_TYPE_DISCONNECT:
			lua_pushinteger(l, event->data);
			lua_setfield(l, -2, "data");

			lua_pushstring(l, "disconnect");
			break;
		case ENET_EVENT_TYPE_RECEIVE:
			lua_pushlstring(l, (const char *)event->packet->data, event->packet->dataLength);
			lua_setfield(l, -2, "data");

			lua_pushinteger(l, event->channelID);
			lua_setfield(l, -2, "channel");

			lua_pushstring(l, "receive");

			enet_packet_destroy(event->packet);
			break;
		case ENET_EVENT_TYPE_NONE:
			lua_pushstring(l, "none");
			break;
	}

	lua_setfield(l, -2, "type");
}

/**
 * Read a packet off the stack as a string
 * idx is position of string
 */
static ENetPacket *read_packet(lua_State *l, int idx, enet_uint8 *channel_id) {
	size_t size;
	int argc = lua_gettop(l);
	const void *data = luaL_checklstring(l, idx, &size);
	ENetPacket *packet;

	enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE;
	*channel_id = 0;

	if (argc >= idx+2 && !lua_isnil(l, idx+2)) {
		const char *flag_str = luaL_checkstring(l, idx+2);
		if (strcmp("unsequenced", flag_str) == 0) {
			flags = ENET_PACKET_FLAG_UNSEQUENCED;
		} else if (strcmp("reliable", flag_str) == 0) {
			flags = ENET_PACKET_FLAG_RELIABLE;
		} else if (strcmp("unreliable", flag_str) == 0) {
			flags = 0;
		} else {
			luaL_error(l, "Unknown packet flag: %s", flag_str);
		}
	}

	if (argc >= idx+1 && !lua_isnil(l, idx+1)) {
		*channel_id = (int) luaL_checknumber(l, idx+1);
	}

	packet = enet_packet_create(data, size, flags);
	if (packet == NULL) {
		luaL_error(l, "Failed to create packet");
	}

	return packet;
}

/**
 * Create a new host
 * Args:
 *	address (nil for client)
 *	[peer_count = 64]
 *	[channel_count = 1]
 *	[in_bandwidth = 0]
 *	[out_bandwidth = 0]
 */
static int host_create(lua_State *l) {
	ENetHost *host;
	size_t peer_count = 64, channel_count = 1;
	enet_uint32 in_bandwidth = 0, out_bandwidth = 0;

	int have_address = 1;
	ENetAddress address;

	if (lua_gettop(l) == 0 || lua_isnil(l, 1)) {
		have_address = 0;
	} else {
		parse_address(l, luaL_checkstring(l, 1), &address);
	}

	switch (lua_gettop(l)) {
		case 5:
			if (!lua_isnil(l, 5)) out_bandwidth = (int) luaL_checknumber(l, 5);
		case 4:
			if (!lua_isnil(l, 4)) in_bandwidth = (int) luaL_checknumber(l, 4);
		case 3:
			if (!lua_isnil(l, 3)) channel_count = (int) luaL_checknumber(l, 3);
		case 2:
			if (!lua_isnil(l, 2)) peer_count = (int) luaL_checknumber(l, 2);
	}

	// printf("host create, peers=%d, channels=%d, in=%d, out=%d\n",
	//		peer_count, channel_count, in_bandwidth, out_bandwidth);
	host = enet_host_create(have_address ? &address : NULL, peer_count,
			channel_count, in_bandwidth, out_bandwidth);

	if (host == NULL) {
		lua_pushnil (l);
		lua_pushstring(l, "enet: failed to create host (already listening?)");
		return 2;
	}

	*(ENetHost**)lua_newuserdata(l, sizeof(void*)) = host;
	luaL_getmetatable(l, "enet_host");
	lua_setmetatable(l, -2);

	return 1;
}

static int linked_version(lua_State *l) {
	lua_pushfstring(l, "%d.%d.%d",
			ENET_VERSION_GET_MAJOR(enet_linked_version()),
			ENET_VERSION_GET_MINOR(enet_linked_version()),
			ENET_VERSION_GET_PATCH(enet_linked_version()));
	return 1;
}

/**
 * Serice a host
 * Args:
 *	timeout
 *
 * Return
 *	nil on no event
 *	an event table on event
 */
static int host_service(lua_State *l) {
	ENetHost *host = check_host(l, 1);
	if (!host) {
		return luaL_error(l, "Tried to index a nil host!");
	}
	ENetEvent event;
	int timeout = 0, out;

	if (lua_gettop(l) > 1)
		timeout = (int) luaL_checknumber(l, 2);

	out = enet_host_service(host, &event, timeout);
	if (out == 0) return 0;
	if (out < 0) return luaL_error(l, "Error during service");

	push_event(l, &event);
	return 1;
}

/**
 * Dispatch a single event if available
 */
static int host_check_events(lua_State *l) {
	ENetHost *host = check_host(l, 1);
	if (!host) {
		return luaL_error(l, "Tried to index a nil host!");
	}
	ENetEvent event;
	int out = enet_host_check_events(host, &event);
	if (out == 0) return 0;
	if (out < 0) return luaL_error(l, "Error checking event");

	push_event(l, &event);
	return 1;
}

/**
 * Enables an adaptive order-2 PPM range coder for the transmitted data of
 * all peers.
 */
static int host_compress_with_range_coder(lua_State *l) {
	ENetHost *host = check_host(l, 1);
	if (!host) {
		return luaL_error(l, "Tried to index a nil host!");
	}

	int result = enet_host_compress_with_range_coder (host);
	if (result == 0) {
		lua_pushboolean (l, 1);
	} else {
		lua_pushboolean (l, 0);
	}

	return 1;
}

/**
 * Connect a host to an address
 * Args:
 *	the address
 *	[channel_count = 1]
 *	[data = 0]
 */
static int host_connect(lua_State *l) {
	ENetHost *host = check_host(l, 1);
	if (!host) {
		return luaL_error(l, "Tried to index a nil host!");
	}
	ENetAddress address;
	ENetPeer *peer;

	enet_uint32 data = 0;
	size_t channel_count = 1;

	parse_address(l, luaL_checkstring(l, 2), &address);

	switch (lua_gettop(l)) {
		case 4:
			if (!lua_isnil(l, 4)) data = (int) luaL_checknumber(l, 4);
		case 3:
			if (!lua_isnil(l, 3)) channel_count = (int) luaL_checknumber(l, 3);
	}

	// printf("host connect, channels=%d, data=%d\n", channel_count, data);
	peer = enet_host_connect(host, &address, channel_count, data);

	if (peer == NULL) {
		return luaL_error(l, "Failed to create peer");
	}

	push_peer(l, peer);

	return 1;
}

static int host_flush(lua_State *l) {
	ENetHost *host = check_host(l, 1);
	if (!host) {
		return luaL_error(l, "Tried to index a nil host!");
	}
	enet_host_flush(host);
	return 0;
}

static int host_broadcast(lua_State *l) {
	ENetHost *host = check_host(l, 1);
	if (!host) {
		return luaL_error(l, "Tried to index a nil host!");
	}

	enet_uint8 channel_id;
	ENetPacket *packet = read_packet(l, 2, &channel_id);
	enet_host_broadcast(host, channel_id, packet);
	return 0;
}

// Args: limit:number
static int host_channel_limit(lua_State *l) {
	ENetHost *host = check_host(l, 1);
	if (!host) {
		return luaL_error(l, "Tried to index a nil host!");
	}
	int limit = (int) luaL_checknumber(l, 2);
	enet_host_channel_limit(host, limit);
	return 0;
}

static int host_bandwidth_limit(lua_State *l) {
	ENetHost *host = check_host(l, 1);
	if (!host) {
		return luaL_error(l, "Tried to index a nil host!");
	}
	enet_uint32 in_bandwidth = (int) luaL_checknumber(l, 2);
	enet_uint32 out_bandwidth = (int) luaL_checknumber(l, 2);
	enet_host_bandwidth_limit(host, in_bandwidth, out_bandwidth);
	return 0;
}

static int host_get_socket_address(lua_State *l) {
	ENetHost *host = check_host(l, 1);
	if (!host) {
		return luaL_error(l, "Tried to index a nil host!");
	}
	ENetAddress address;
	enet_socket_get_address (host->socket, &address);

	lua_pushfstring(l, "%d.%d.%d.%d:%d",
			((address.host) & 0xFF),
			((address.host >> 8) & 0xFF),
			((address.host >> 16) & 0xFF),
			(address.host >> 24& 0xFF),
			address.port);
	return 1;
}
static int host_total_sent_data(lua_State *l) {
	ENetHost *host = check_host(l, 1);
	if (!host) {
		return luaL_error(l, "Tried to index a nil host!");
	}

	lua_pushinteger (l, host->totalSentData);

	return 1;
}

static int host_total_received_data(lua_State *l) {
	ENetHost *host = check_host(l, 1);
	if (!host) {
		return luaL_error(l, "Tried to index a nil host!");
	}

	lua_pushinteger (l, host->totalReceivedData);

	return 1;
}
static int host_service_time(lua_State *l) {
	ENetHost *host = check_host(l, 1);
	if (!host) {
		return luaL_error(l, "Tried to index a nil host!");
	}

	lua_pushinteger (l, host->serviceTime);

	return 1;
}

static int host_peer_count(lua_State *l) {
	ENetHost *host = check_host(l, 1);
	if (!host) {
		return luaL_error(l, "Tried to index a nil host!");
	}

	lua_pushinteger (l, host->peerCount);

	return 1;
}

static int host_get_peer(lua_State *l) {
	ENetHost *host = check_host(l, 1);
	if (!host) {
		return luaL_error(l, "Tried to index a nil host!");
	}

	int peer_index = (int) luaL_checknumber(l, 2) - 1;

	if (peer_index < 0 || ((size_t) peer_index) >= host->peerCount) {
		luaL_argerror (l, 2, "Invalid peer index");
	}

	ENetPeer *peer = &(host->peers[peer_index]);

	push_peer (l, peer);
	return 1;
}

static int host_gc(lua_State *l) {
	// We have to manually grab the userdata so that we can set it to NULL.
	ENetHost** host = (ENetHost**)luaL_checkudata(l, 1, "enet_host");
	// We don't want to crash by destroying a non-existant host.
	if (*host) {
		enet_host_destroy(*host);
	}
	*host = NULL;
	return 0;
}

static int peer_tostring(lua_State *l) {
	ENetPeer *peer = check_peer(l, 1);
	char host_str[128];
	enet_address_get_host_ip(&peer->address, host_str, 128);

	lua_pushstring(l, host_str);
	lua_pushstring(l, ":");
	lua_pushinteger(l, peer->address.port);
	lua_concat(l, 3);
	return 1;
}

static int peer_ping(lua_State *l) {
	ENetPeer *peer = check_peer(l, 1);
	enet_peer_ping(peer);
	return 0;
}

static int peer_throttle_configure(lua_State *l) {
	ENetPeer *peer = check_peer(l, 1);

	enet_uint32 interval = (int) luaL_checknumber(l, 2);
	enet_uint32 acceleration = (int) luaL_checknumber(l, 3);
	enet_uint32 deceleration = (int) luaL_checknumber(l, 4);

	enet_peer_throttle_configure(peer, interval, acceleration, deceleration);
	return 0;
}

static int peer_round_trip_time(lua_State *l) {
	ENetPeer *peer = check_peer(l, 1);

	if (lua_gettop(l) > 1) {
		enet_uint32 round_trip_time = (int) luaL_checknumber(l, 2);
		peer->roundTripTime = round_trip_time;
	}

	lua_pushinteger (l, peer->roundTripTime);

	return 1;
}

static int peer_last_round_trip_time(lua_State *l) {
	ENetPeer *peer = check_peer(l, 1);

	if (lua_gettop(l) > 1) {
		enet_uint32 round_trip_time = (int) luaL_checknumber(l, 2);
		peer->lastRoundTripTime = round_trip_time;
	}
	lua_pushinteger (l, peer->lastRoundTripTime);

	return 1;
}

static int peer_ping_interval(lua_State *l) {
	ENetPeer *peer = check_peer(l, 1);

	if (lua_gettop(l) > 1) {
		enet_uint32 interval = (int) luaL_checknumber(l, 2);
		enet_peer_ping_interval (peer, interval);
	}

	lua_pushinteger (l, peer->pingInterval);

	return 1;
}

static int peer_timeout(lua_State *l) {
	ENetPeer *peer = check_peer(l, 1);

	enet_uint32 timeout_limit = 0;
	enet_uint32 timeout_minimum = 0;
	enet_uint32 timeout_maximum = 0;

	switch (lua_gettop(l)) {
		case 4:
			if (!lua_isnil(l, 4)) timeout_maximum = (int) luaL_checknumber(l, 4);
		case 3:
			if (!lua_isnil(l, 3)) timeout_minimum = (int) luaL_checknumber(l, 3);
		case 2:
			if (!lua_isnil(l, 2)) timeout_limit = (int) luaL_checknumber(l, 2);
	}

	enet_peer_timeout (peer, timeout_limit, timeout_minimum, timeout_maximum);

	lua_pushinteger (l, peer->timeoutLimit);
	lua_pushinteger (l, peer->timeoutMinimum);
	lua_pushinteger (l, peer->timeoutMaximum);

	return 3;
}

static int peer_disconnect(lua_State *l) {
	ENetPeer *peer = check_peer(l, 1);

	enet_uint32 data = lua_gettop(l) > 1 ? (int) luaL_checknumber(l, 2) : 0;
	enet_peer_disconnect(peer, data);
	return 0;
}

static int peer_disconnect_now(lua_State *l) {
	ENetPeer *peer = check_peer(l, 1);

	enet_uint32 data = lua_gettop(l) > 1 ? (int) luaL_checknumber(l, 2) : 0;
	enet_peer_disconnect_now(peer, data);
	return 0;
}

static int peer_disconnect_later(lua_State *l) {
	ENetPeer *peer = check_peer(l, 1);

	enet_uint32 data = lua_gettop(l) > 1 ? (int) luaL_checknumber(l, 2) : 0;
	enet_peer_disconnect_later(peer, data);
	return 0;
}

static int peer_index(lua_State *l) {
	ENetPeer *peer = check_peer(l, 1);

	size_t peer_index = find_peer_index (l, peer->host, peer);
	lua_pushinteger (l, peer_index + 1);

	return 1;
}

static int peer_state(lua_State *l) {
	ENetPeer *peer = check_peer(l, 1);

	switch (peer->state) {
		case (ENET_PEER_STATE_DISCONNECTED):
			lua_pushstring (l, "disconnected");
			break;
		case (ENET_PEER_STATE_CONNECTING):
			lua_pushstring (l, "connecting");
			break;
		case (ENET_PEER_STATE_ACKNOWLEDGING_CONNECT):
			lua_pushstring (l, "acknowledging_connect");
			break;
		case (ENET_PEER_STATE_CONNECTION_PENDING):
			lua_pushstring (l, "connection_pending");
			break;
		case (ENET_PEER_STATE_CONNECTION_SUCCEEDED):
			lua_pushstring (l, "connection_succeeded");
			break;
		case (ENET_PEER_STATE_CONNECTED):
			lua_pushstring (l, "connected");
			break;
		case (ENET_PEER_STATE_DISCONNECT_LATER):
			lua_pushstring (l, "disconnect_later");
			break;
		case (ENET_PEER_STATE_DISCONNECTING):
			lua_pushstring (l, "disconnecting");
			break;
		case (ENET_PEER_STATE_ACKNOWLEDGING_DISCONNECT):
			lua_pushstring (l, "acknowledging_disconnect");
			break;
		case (ENET_PEER_STATE_ZOMBIE):
			lua_pushstring (l, "zombie");
			break;
		default:
			lua_pushstring (l, "unknown");
	}

	return 1;
}

static int peer_connect_id(lua_State *l) {
	ENetPeer *peer = check_peer(l, 1);

	lua_pushinteger (l, peer->connectID);

	return 1;
}


static int peer_reset(lua_State *l) {
	ENetPeer *peer = check_peer(l, 1);
	enet_peer_reset(peer);
	return 0;
}

static int peer_receive(lua_State *l) {
	ENetPeer *peer = check_peer(l, 1);
	ENetPacket *packet;
	enet_uint8 channel_id = 0;

	if (lua_gettop(l) > 1) {
		channel_id = (int) luaL_checknumber(l, 2);
	}

	packet = enet_peer_receive(peer, &channel_id);
	if (packet == NULL) return 0;

	lua_pushlstring(l, (const char *)packet->data, packet->dataLength);
	lua_pushinteger(l, channel_id);

	enet_packet_destroy(packet);
	return 2;
}


/**
 * Send a lua string to a peer
 * Args:
 *	packet data, string
 *	channel id
 *	flags ["reliable", nil]
 *
 */
static int peer_send(lua_State *l) {
	ENetPeer *peer = check_peer(l, 1);

	enet_uint8 channel_id;
	ENetPacket *packet = read_packet(l, 2, &channel_id);

	// printf("sending, channel_id=%d\n", channel_id);
	int ret = enet_peer_send(peer, channel_id, packet);
	if (ret < 0) {
		enet_packet_destroy(packet);
	}

	lua_pushinteger(l, ret);

	return 1;
}

static const struct luaL_Reg enet_funcs [] = {
	{"host_create", host_create},
	{"linked_version", linked_version},
	{NULL, NULL}
};

static const struct luaL_Reg enet_host_funcs [] = {
	{"service", host_service},
	{"check_events", host_check_events},
	{"compress_with_range_coder", host_compress_with_range_coder},
	{"connect", host_connect},
	{"flush", host_flush},
	{"broadcast", host_broadcast},
	{"channel_limit", host_channel_limit},
	{"bandwidth_limit", host_bandwidth_limit},
	// Since ENetSocket isn't part of enet-lua, we should try to keep
	// naming conventions the same as the rest of the lib.
	{"get_socket_address", host_get_socket_address},
	// We need this function to free up our ports when needed!
	{"destroy", host_gc},

	// additional convenience functions (mostly accessors)
	{"total_sent_data", host_total_sent_data},
	{"total_received_data", host_total_received_data},
	{"service_time", host_service_time},
	{"peer_count", host_peer_count},
	{"get_peer", host_get_peer},
	{NULL, NULL}
};

static const struct luaL_Reg enet_peer_funcs [] = {
	{"disconnect", peer_disconnect},
	{"disconnect_now", peer_disconnect_now},
	{"disconnect_later", peer_disconnect_later},
	{"reset", peer_reset},
	{"ping", peer_ping},
	{"receive", peer_receive},
	{"send", peer_send},
	{"throttle_configure", peer_throttle_configure},
	{"ping_interval", peer_ping_interval},
	{"timeout", peer_timeout},

	// additional convenience functions to member variables
	{"index", peer_index},
	{"state", peer_state},
	{"connect_id", peer_connect_id},
	{"round_trip_time", peer_round_trip_time},
	{"last_round_trip_time", peer_last_round_trip_time},
	{NULL, NULL}
};

extern "C" {
	void luax_register(lua_State *L, const char *name, const luaL_Reg *l);
}

int luaopen_enet(lua_State *l) {
	enet_initialize();
	atexit(enet_deinitialize);

	// create metatables
	luaL_newmetatable(l, "enet_host");
	lua_newtable(l); // index
	luax_register(l, NULL, enet_host_funcs);
	lua_setfield(l, -2, "__index");
	lua_pushcfunction(l, host_gc);
	lua_setfield(l, -2, "__gc");

	luaL_newmetatable(l, "enet_peer");
	lua_newtable(l);
	luax_register(l, NULL, enet_peer_funcs);
	lua_setfield(l, -2, "__index");
	lua_pushcfunction(l, peer_tostring);
	lua_setfield(l, -2, "__tostring");

	// set up peer table
	lua_newtable(l);

	lua_newtable(l); // metatable
	lua_pushstring(l, "v");
	lua_setfield(l, -2, "__mode");
	lua_setmetatable(l, -2);

	lua_setfield(l, LUA_REGISTRYINDEX, "enet_peers");

	luax_register(l, nullptr, enet_funcs);

	// return the enet table created with luaL_register
	return 1;
}
