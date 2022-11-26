#pragma once

#include <spdlog/spdlog.h>

namespace Piccolo
{
    class LogSystem final
    {
    public:
        enum class LogLevel : uint8_t
        {
            debug,
            info,
            warn,
            error,
            fatal
        };

        LogSystem();
        ~LogSystem();

        template<typename... TARGS>
        void log(LogLevel level, TARGS&&... args)
        {
            switch (level)
            {
                case LogLevel::debug:
                    m_logger->debug(std::forward<TARGS>(args)...);
                    break;

                case LogLevel::info:
                    m_logger->info(std::forward<TARGS>(args)...);
                    break;

                case LogLevel::warn:
                    m_logger->warn(std::forward<TARGS>(args)...);
                    break;

                case LogLevel::error:
                    m_logger->error(std::forward<TARGS>(args)...);
                    break;

                case LogLevel::fatal:
                    m_logger->critical(std::forward<TARGS>(args)...);
                    fatalCallback(std::forward<TARGS>(args)...);
                    break;
            }
        }

        template<typename... TARGS>
        void fatalCallback(TARGS&&... args)
        {
            const std::string format_str = fmt::format(std::forward<TARGS>(args)...);
            throw std::runtime_error(format_str);
        }

    private:
        std::shared_ptr<spdlog::logger> m_logger;
    };
} // namespace Piccolo