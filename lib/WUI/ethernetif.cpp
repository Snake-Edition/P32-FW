#include "ethernetif.h"
#include "otp.hpp"
#include <logging/log.hpp>

LOG_COMPONENT_REF(Network);

uint8_t *ethernetif_get_mac() {
    return const_cast<uint8_t *>(otp_get_mac_address()->mac);
}

extern "C" void log_dropped_packet_rx(size_t len) {
    log_warning(Network, "pbuf_alloc_rx(%zu) failed on eth, dropping packet", len);
}
