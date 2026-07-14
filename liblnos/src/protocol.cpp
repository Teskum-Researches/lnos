#include "lnos/protocol.h"

namespace lnos {
    Blob encodeWithoutSignature(const Packet& p)
    {
        Blob blob;

        blobPush(blob, p.version);
        blobPush(blob, (uint16_t)p.type);
        blobPush(blob, p.publicKey);
        blobPush(blob, p.name);

        uint64_t len = p.services.size();
        blobPush(blob, len);

        for (const auto& s : p.services)
        {
            blobPush(blob, s.name);
            blobPush(blob, s.port);
        }

        return blob;
    }
}