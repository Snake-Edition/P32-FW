#pragma once

#include "connect_error.h"
#include <optional>

namespace http {

class Connection;

/**
 * Negotiates the CONNECT method.
 *
 * Takes a connection already established to a proxy server and asks it to
 * CONNECT (eg. the http method, not our service) to the provided host and
 * port.
 *
 * Returns nullopt for success (and further communication can happen over the
 * connection), or specific error if it failed.
 */
std::optional<Error> proxy_connect(Connection &connection, const char *host, uint16_t port);

} // namespace http
