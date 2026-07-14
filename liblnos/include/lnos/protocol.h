#pragma once
#include <array>
#include <string>
#include <vector>
#include <cstdint>
#include <cstring>

constexpr std::size_t PUBLIC_KEY_SIZE = 32;
constexpr std::size_t PRIVATE_KEY_SIZE = 64;
constexpr std::size_t SIGNATURE_SIZE = 64;
constexpr int PROTOCOL_VERSION = 2;

/*
+----------------+
| version        |
+----------------+
| type           |
+----------------+
| public key     |  <-- who
+----------------+
| name           |
+----------------+
| services       |
+----------------+
| signature      |  <-- proof (r.i.p)
+----------------+ */

namespace lnos {

    enum class PacketType {
        Announce
    };

    struct Service {
        std::string name;
        uint16_t port;
    };

    struct Packet {
        std::string version;
        PacketType type;
        std::array<std::uint8_t, PUBLIC_KEY_SIZE> publicKey;
        std::string name;
        std::vector<Service> services;
        std::array<std::uint8_t, SIGNATURE_SIZE> signature;
    };

    using Blob = std::vector<uint8_t>;

    Blob encodeWithoutSignature(const Packet& p);

    struct EncodedPacket {
        uint8_t *data;
        uint64_t len;
    };

    inline void blobPush(Blob& blob, uint64_t data) {
        uint8_t *ptr = (uint8_t *) &data;
        for (uint64_t i = 0; i < sizeof(data); ++i)
            blob.push_back(ptr[i]);
    }

    inline void blobPush(Blob& blob, uint16_t data) {
        uint8_t *ptr = (uint8_t *) &data;
        for (uint64_t i = 0; i < sizeof(data); ++i)
            blob.push_back(ptr[i]);
    }

    inline void blobPush(Blob& blob, const std::string& data) {
        uint64_t len = data.length();
        blobPush(blob, len);
        for (uint64_t i = 0; i < len; ++i)
            blob.push_back((uint8_t) data.data()[i]);
    }

    template <std::size_t N>
    inline void blobPush(Blob& blob, const std::array<uint8_t, N>& data) {
        for (uint8_t b : data)
            blob.push_back(b);
    }

    inline bool encodedPacketConsumeImpl(EncodedPacket& packet, uint64_t& data) {
        if (packet.len < sizeof(uint64_t))
            return false;
        data = *(uint64_t *) packet.data;
        packet.data += sizeof(uint64_t);
        packet.len -= sizeof(uint64_t);
        return true;
    }

    inline bool encodedPacketConsumeImpl(EncodedPacket& packet, uint16_t& data) {
        if (packet.len < sizeof(uint16_t))
            return false;
        data = *(uint16_t *) packet.data;
        packet.data += sizeof(uint16_t);
        packet.len -= sizeof(uint16_t);
        return true;
    }

    inline bool encodedPacketConsumeImpl(EncodedPacket& packet, std::string& data) {
        uint64_t len = 0;
        if (!encodedPacketConsumeImpl(packet, len))
            return false;
        if (packet.len < len)
            return false;
        data = std::string((char *) packet.data, len);
        packet.data += len;
        packet.len -= len;
        return true;
    }

    template <std::size_t N>
    inline bool encodedPacketConsumeImpl(EncodedPacket& packet,
                            std::array<uint8_t, N>& data) {
        if (packet.len < N)
            return false;

        std::memcpy(data.data(), packet.data, N);

        packet.data += N;
        packet.len -= N;

        return true;
    }

    inline Blob encode(const Packet& p) {
        Blob blob;

        blobPush(blob, p.version);
        blobPush(blob, (uint16_t) p.type);
        blobPush(blob, p.publicKey);
        blobPush(blob, p.name);

        uint64_t len = p.services.size();
        blobPush(blob, len);

        for (const auto& s : p.services)
        {
            blobPush(blob, s.name);
            blobPush(blob, s.port);
        }

        blobPush(blob, p.signature);

        return blob;
    }

#define encodedPacketConsume(blob, data)       \
  do {                                         \
    if (!encodedPacketConsumeImpl(blob, data)) \
      return false;                            \
  } while (0)

    inline bool decode(EncodedPacket& packet, Packet& result) {
        encodedPacketConsume(packet, result.version);
        uint16_t type = 0;
        encodedPacketConsume(packet, type);
        result.type = (PacketType) type;

        encodedPacketConsume(packet, result.publicKey);
        encodedPacketConsume(packet, result.name);

        uint64_t len = 0;
        encodedPacketConsume(packet, len);
        result.services.reserve(len);

        for (uint64_t i = 0; i < len; ++i)
        {
            Service service;
            encodedPacketConsume(packet, service.name);
            encodedPacketConsume(packet, service.port);
            result.services.push_back(service);
        }

        encodedPacketConsume(packet, result.signature);

        return true;
    }

#undef encodedPacketConsume

}
