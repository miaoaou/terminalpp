#include "tab_control_server.h"

#if (defined ARCH_WINDOWS && defined RENDERER_NATIVE)

#include <sddl.h>
#include <algorithm>
#include <chrono>
#include <cstring>
#include <future>
#include <iomanip>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#pragma comment(lib, "advapi32.lib")

namespace tpp {

    namespace {
        wchar_t const * const PIPE_NAME = L"\\\\.\\pipe\\terminalpp-control";
        DWORD const MAX_REQUEST_SIZE = 1024 * 1024;
        DWORD const PIPE_BUFFER_SIZE = 64 * 1024;

        struct PipeSecurity {
            SECURITY_ATTRIBUTES attributes{};
            PSECURITY_DESCRIPTOR descriptor = nullptr;

            ~PipeSecurity() {
                if (descriptor != nullptr)
                    LocalFree(descriptor);
            }

            bool initialize() {
                HANDLE token = nullptr;
                if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token))
                    return false;

                DWORD required = 0;
                GetTokenInformation(token, TokenUser, nullptr, 0, &required);
                if (required == 0) {
                    CloseHandle(token);
                    return false;
                }
                std::vector<unsigned char> tokenData(required);
                bool const gotToken = GetTokenInformation(token, TokenUser, tokenData.data(), required, &required) != FALSE;
                CloseHandle(token);
                if (!gotToken)
                    return false;

                LPWSTR sid = nullptr;
                if (!ConvertSidToStringSidW(reinterpret_cast<TOKEN_USER *>(tokenData.data())->User.Sid, &sid))
                    return false;
                std::wstring sddl = L"D:P(A;;GA;;;SY)(A;;GA;;;";
                sddl += sid;
                sddl += L")";
                LocalFree(sid);

                if (!ConvertStringSecurityDescriptorToSecurityDescriptorW(sddl.c_str(), SDDL_REVISION_1, &descriptor, nullptr))
                    return false;
                attributes.nLength = sizeof(attributes);
                attributes.lpSecurityDescriptor = descriptor;
                attributes.bInheritHandle = FALSE;
                return true;
            }
        };

        void WriteJSONString(std::ostream & stream, std::string const & value) {
            static char const HEX[] = "0123456789abcdef";
            stream.put('"');
            for (unsigned char c : value) {
                switch (c) {
                    case '"': stream << "\\\""; break;
                    case '\\': stream << "\\\\"; break;
                    case '\b': stream << "\\b"; break;
                    case '\f': stream << "\\f"; break;
                    case '\n': stream << "\\n"; break;
                    case '\r': stream << "\\r"; break;
                    case '\t': stream << "\\t"; break;
                    default:
                        if (c < 0x20) {
                            stream << "\\u00" << HEX[c >> 4] << HEX[c & 0x0f];
                        } else {
                            stream.put(static_cast<char>(c));
                        }
                        break;
                }
            }
            stream.put('"');
        }

        void WriteJSON(std::ostream & stream, JSON const & value) {
            switch (value.kind()) {
                case JSON::Kind::Null:
                    stream << "null";
                    break;
                case JSON::Kind::Boolean:
                    stream << (value.toBool() ? "true" : "false");
                    break;
                case JSON::Kind::Integer:
                    stream << value.toInt();
                    break;
                case JSON::Kind::Double:
                    stream << std::setprecision(std::numeric_limits<double>::max_digits10) << value.toDouble();
                    break;
                case JSON::Kind::String:
                    WriteJSONString(stream, value.toString());
                    break;
                case JSON::Kind::Array: {
                    stream.put('[');
                    bool first = true;
                    for (auto item = value.begin(); item != value.end(); ++item) {
                        if (!first)
                            stream.put(',');
                        WriteJSON(stream, *item);
                        first = false;
                    }
                    stream.put(']');
                    break;
                }
                case JSON::Kind::Object: {
                    stream.put('{');
                    bool first = true;
                    for (auto item = value.begin(); item != value.end(); ++item) {
                        if (!first)
                            stream.put(',');
                        WriteJSONString(stream, item.name());
                        stream.put(':');
                        WriteJSON(stream, *item);
                        first = false;
                    }
                    stream.put('}');
                    break;
                }
            }
        }

        std::string ToJson(JSON const & value) {
            std::stringstream stream;
            WriteJSON(stream, value);
            return stream.str();
        }

        JSON ErrorResponse(std::string const & message) {
            JSON result = JSON::Object();
            result.add("error", JSON{message});
            return result;
        }

        bool ReadRequest(HANDLE pipe, std::atomic_bool const & stopping, std::string & request) {
            char buffer[4096];
            while (!stopping.load()) {
                DWORD bytesRead = 0;
                if (ReadFile(pipe, buffer, sizeof(buffer), &bytesRead, nullptr)) {
                    if (bytesRead == 0)
                        return false;
                    char const * newline = static_cast<char const *>(std::memchr(buffer, '\n', bytesRead));
                    size_t const count = newline == nullptr ? bytesRead : static_cast<size_t>(newline - buffer);
                    if (request.size() + count > MAX_REQUEST_SIZE)
                        return false;
                    request.append(buffer, count);
                    if (newline != nullptr)
                        return true;
                    continue;
                }
                DWORD const error = GetLastError();
                if (error == ERROR_NO_DATA || error == ERROR_PIPE_LISTENING) {
                    Sleep(10);
                    continue;
                }
                return false;
            }
            return false;
        }

        bool WriteResponse(HANDLE pipe, std::atomic_bool const & stopping, std::string const & response) {
            std::string const data = response + "\n";
            size_t offset = 0;
            while (offset < data.size() && !stopping.load()) {
                DWORD bytesWritten = 0;
                DWORD const chunk = static_cast<DWORD>(std::min<size_t>(data.size() - offset, PIPE_BUFFER_SIZE));
                if (WriteFile(pipe, data.data() + offset, chunk, &bytesWritten, nullptr)) {
                    offset += bytesWritten;
                    continue;
                }
                DWORD const error = GetLastError();
                if (error == ERROR_NO_DATA || error == ERROR_PIPE_LISTENING) {
                    Sleep(10);
                    continue;
                }
                return false;
            }
            return offset == data.size();
        }

        void WaitForClientClose(HANDLE pipe, std::atomic_bool const & stopping) {
            char discard[256];
            while (!stopping.load()) {
                DWORD bytesRead = 0;
                if (ReadFile(pipe, discard, sizeof(discard), &bytesRead, nullptr)) {
                    if (bytesRead == 0)
                        return;
                    continue;
                }
                DWORD const error = GetLastError();
                if (error == ERROR_BROKEN_PIPE || error == ERROR_PIPE_NOT_CONNECTED)
                    return;
                if (error == ERROR_NO_DATA || error == ERROR_PIPE_LISTENING) {
                    Sleep(10);
                    continue;
                }
                return;
            }
        }
    }

    struct TabControlServer::RequestState {
        std::promise<std::string> response;
        std::atomic_bool completed{false};

        void complete(std::string value) {
            if (!completed.exchange(true))
                response.set_value(std::move(value));
        }
    };

    TabControlServer::TabControlServer(Dispatcher dispatcher, RequestHandler handler):
        dispatcher_{std::move(dispatcher)},
        handler_{std::move(handler)},
        alive_{std::make_shared<std::atomic_bool>(true)},
        thread_{[this]() { run(); }} {
    }

    TabControlServer::~TabControlServer() {
        alive_->store(false);
        stopping_.store(true);
        {
            std::lock_guard<std::mutex> lock{requestMutex_};
            if (std::shared_ptr<RequestState> request = pendingRequest_.lock())
                request->complete(ToJson(ErrorResponse("Terminal++ is shutting down")));
        }
        if (thread_.joinable())
            thread_.join();
    }

    void TabControlServer::run() {
        PipeSecurity security;
        if (!security.initialize())
            return;

        while (!stopping_.load()) {
            HANDLE pipe = CreateNamedPipeW(
                PIPE_NAME,
                PIPE_ACCESS_DUPLEX,
                PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_NOWAIT | PIPE_REJECT_REMOTE_CLIENTS,
                1,
                PIPE_BUFFER_SIZE,
                PIPE_BUFFER_SIZE,
                0,
                &security.attributes);
            if (pipe == INVALID_HANDLE_VALUE)
                return;

            bool connected = false;
            while (!stopping_.load()) {
                if (ConnectNamedPipe(pipe, nullptr)) {
                    connected = true;
                    break;
                }
                DWORD const error = GetLastError();
                if (error == ERROR_PIPE_CONNECTED) {
                    connected = true;
                    break;
                }
                if (error != ERROR_PIPE_LISTENING)
                    break;
                Sleep(10);
            }

            if (connected && !stopping_.load()) {
                std::string line;
                if (ReadRequest(pipe, stopping_, line)) {
                    std::shared_ptr<RequestState> request = std::make_shared<RequestState>();
                    std::future<std::string> response = request->response.get_future();
                    {
                        std::lock_guard<std::mutex> lock{requestMutex_};
                        pendingRequest_ = request;
                    }
                    try {
                        JSON parsed = JSON::Parse(line);
                        if (parsed.kind() != JSON::Kind::Object)
                            throw std::runtime_error("Request must be a JSON object");
                        std::weak_ptr<std::atomic_bool> alive = alive_;
                        RequestHandler handler = handler_;
                        dispatcher_([alive, request, handler, parsed = std::move(parsed)]() mutable {
                            std::shared_ptr<std::atomic_bool> lifetime = alive.lock();
                            if (!lifetime || !lifetime->load()) {
                                request->complete(ToJson(ErrorResponse("Terminal++ is not available")));
                                return;
                            }
                            try {
                                request->complete(ToJson(handler(parsed)));
                            } catch (std::exception const & error) {
                                request->complete(ToJson(ErrorResponse(error.what())));
                            } catch (...) {
                                request->complete(ToJson(ErrorResponse("Unknown request error")));
                            }
                        });
                    } catch (std::exception const & error) {
                        request->complete(ToJson(ErrorResponse(error.what())));
                    }

                    while (!stopping_.load() && response.wait_for(std::chrono::milliseconds(50)) != std::future_status::ready) {
                    }
                    if (response.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
                        if (WriteResponse(pipe, stopping_, response.get()))
                            WaitForClientClose(pipe, stopping_);
                    }
                    {
                        std::lock_guard<std::mutex> lock{requestMutex_};
                        pendingRequest_.reset();
                    }
                }
            }

            DisconnectNamedPipe(pipe);
            CloseHandle(pipe);
        }
    }

} // namespace tpp

#endif
