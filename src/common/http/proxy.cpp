#include "proxy.hpp"
#include "httpc.hpp"

namespace http {

namespace {

    class ConnectRequest final : public Request {
    private:
        const char *upstream;

    public:
        explicit ConnectRequest(const char *upstream)
            : upstream(upstream) {}
        virtual const char *url() const override {
            return upstream;
        }
        virtual Method method() const override {
            return Method::Connect;
        }
        virtual ContentType content_type() const override {
            // Actually, not used with this request, just filling in an
            // abstract method.
            return ContentType::ApplicationOctetStream;
        }
    };

    class DumbFactory final : public ConnectionFactory {
    private:
        Connection &premade_connection;
        const char *upstream_host;

    public:
        DumbFactory(Connection &premade_connection, const char *upstream_host)
            : premade_connection(premade_connection)
            , upstream_host(upstream_host) {}
        virtual std::variant<Connection *, Error> connection() override {
            return &premade_connection;
        }
        const char *host() override {
            return upstream_host;
        }
        virtual void invalidate() override {
            // No way to do it and we will return an error from the
            // proxy_connect anyway.
        }
    };

} // namespace

std::optional<Error> proxy_connect(Connection &connection, const char *host, uint16_t port) {
    // 35 for connect hostname, 5 for max port, 1 for :, 1 for \0
    constexpr size_t max_len = 35 + 5 + 1 + 1;
    char upstream[max_len];
    snprintf(upstream, sizeof upstream, "%s:%" PRIu16, host, port);
    DumbFactory factory(connection, host);
    HttpClient client(factory);
    ConnectRequest request(upstream);
    auto result = client.send(request, nullptr);
    if (std::holds_alternative<Error>(result)) {
        return std::get<Error>(result);
    } else {
        auto response = std::move(std::get<Response>(result));
        if (response.status >= 200 && response.status < 300) {
            return std::nullopt;
        } else {
            return Error::Proxy;
        }
    }
}

} // namespace http
