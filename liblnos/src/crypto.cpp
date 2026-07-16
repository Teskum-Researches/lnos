#include <sodium.h>
#include <fstream>
#include <stdexcept>

#include <lnos/crypto.h>


namespace lnos {


    bool signPacket(
        Packet& packet,
        const std::array<uint8_t, PRIVATE_KEY_SIZE>& privateKey)
    {
        Blob data = encode(packet, false);

        unsigned long long len;

        crypto_sign_detached(
            packet.signature.data(),
            &len,
            data.data(),
            data.size(),
            privateKey.data()
        );

        return len == SIGNATURE_SIZE;
    }

    std::array<std::uint8_t, PUBLIC_KEY_SIZE> loadPublicKey() {
        std::array<std::uint8_t, PUBLIC_KEY_SIZE> publicKey{};

        std::ifstream publicKeyFile("/etc/lnos/public.key", std::ios::binary);
        if (!publicKeyFile.is_open())
            throw std::runtime_error("Failed to open /etc/lnos/public.key");

        publicKeyFile.read(reinterpret_cast<char*>(publicKey.data()),
                           publicKey.size());

        if (publicKeyFile.gcount() != static_cast<std::streamsize>(publicKey.size()))
            throw std::runtime_error("Invalid public key file");

        return publicKey;
    }

    std::array<std::uint8_t, PRIVATE_KEY_SIZE> loadPrivateKey() {
        std::array<std::uint8_t, PRIVATE_KEY_SIZE> privateKey{};

        std::ifstream file("/etc/lnos/private.key", std::ios::binary);
        if (!file)
            throw std::runtime_error("Failed to open /etc/lnos/private.key");

        file.read(reinterpret_cast<char*>(privateKey.data()), privateKey.size());

        if (file.gcount() != static_cast<std::streamsize>(privateKey.size()))
            throw std::runtime_error("Invalid private.key");

        return privateKey;
    }


}
