#pragma once

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <string>

namespace pke
{
    namespace engine
    {
        /**
         * @brief File-based logger. Writes tagged, timestamped entries to a per-day
         * log file (pikselite-YYYY-MM-DD.log). One file per day, no terminal output.
         * On Windows the logs go to %APPDATA%\PikseliteEngine\logs so that installed
         * builds always have a writable, stable location regardless of how the app
         * is launched.
         */
        class Log
        {
        public:
            /**
             * @brief Overrides the directory where log files are written.
             * @param logDir Directory for log files (default %APPDATA%\PikseliteEngine\logs).
             */
            static void init(const std::filesystem::path& logDir = defaultDir())
            {
                std::lock_guard<std::mutex> lock(Mutex());
                Dir() = logDir;
            }

            static void info(const std::string& msg)
            {
                write("LOG", msg);
            }

            static void warning(const std::string& msg)
            {
                write("WARNING", msg);
            }

            static void error(const std::string& msg)
            {
                write("ERROR", msg);
            }

        private:
            Log() = default;

            static std::mutex& Mutex()
            {
                static std::mutex mu;
                return mu;
            }

            static std::filesystem::path defaultDir()
            {
    #ifdef _WIN32
                char* appData = nullptr;
                size_t len = 0;
                if (_dupenv_s(&appData, &len, "APPDATA") == 0 && appData && *appData)
                {
                    std::filesystem::path dir =
                        std::filesystem::path(appData) / "PikseliteEngine" / "logs";
                    free(appData);
                    return dir;
                }
    #else
                const char* appData = std::getenv("APPDATA");
                if (appData && *appData)
                    return std::filesystem::path(appData) / "PikseliteEngine" / "logs";
    #endif
                return std::filesystem::path("logs");
            }

            static std::filesystem::path& Dir()
            {
                static std::filesystem::path dir = defaultDir();
                return dir;
            }

            static std::ofstream& Stream()
            {
                static std::ofstream out;
                return out;
            }

            static std::string& OpenDate()
            {
                static std::string date;
                return date;
            }

            static std::string dateString()
            {
                std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
                std::time_t tt = std::chrono::system_clock::to_time_t(now);
                std::tm tm = toLocalTime(tt);
                std::ostringstream ss;
                ss << std::put_time(&tm, "%Y-%m-%d");
                return ss.str();
            }

            static std::string timestampString()
            {
                std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
                std::time_t tt = std::chrono::system_clock::to_time_t(now);
                std::tm tm = toLocalTime(tt);
                std::ostringstream ss;
                ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
                return ss.str();
            }

            static std::tm toLocalTime(const std::time_t& tt)
            {
                std::tm tm{};
    #ifdef _WIN32
                localtime_s(&tm, &tt);
    #else
                localtime_r(&tt, &tm);
    #endif
                return tm;
            }

            static void write(const std::string& tag, const std::string& msg)
            {
                std::lock_guard<std::mutex> lock(Mutex());

                try
                {
                    std::string date = dateString();

                    if (!Stream().is_open() || date != OpenDate())
                    {
                        std::filesystem::path dir = Dir();
                        std::error_code ec;
                        std::filesystem::create_directories(dir, ec);
                        Stream().open(dir / ("pikselite-" + date + ".log"),
                                    std::ios::out | std::ios::app);
                        OpenDate() = date;
                    }

                    if (Stream().is_open())
                    {
                        Stream() << "[" << timestampString() << "] [" << tag << "] " << msg << '\n';
                        Stream().flush();
                    }
                }
                catch (...)
                {
                }
            }
        };
    } // namespace engine
}