#pragma once

#if (defined ARCH_WINDOWS && defined RENDERER_NATIVE)

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>

#include "helpers/json.h"

namespace tpp {

    /** A same-user, local named-pipe endpoint for reading and sending terminal text. */
    class TabControlServer {
    public:
        using RequestHandler = std::function<JSON(JSON const &)>;
        using Dispatcher = std::function<void(std::function<void()>)>;

        TabControlServer(Dispatcher dispatcher, RequestHandler handler);
        ~TabControlServer();

        TabControlServer(TabControlServer const &) = delete;
        TabControlServer & operator = (TabControlServer const &) = delete;

    private:
        struct RequestState;
        void run();

        Dispatcher dispatcher_;
        RequestHandler handler_;
        std::shared_ptr<std::atomic_bool> alive_;
        std::atomic_bool stopping_{false};
        std::mutex requestMutex_;
        std::weak_ptr<RequestState> pendingRequest_;
        std::thread thread_;
    };

} // namespace tpp

#endif
