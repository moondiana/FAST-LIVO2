#ifndef LOG_INIT_H
#define LOG_INIT_H

#include <string>

void initFastLivoLogging(const char* argv0);
void shutdownFastLivoLogging();
void appendLoopLog(const std::string& message);

/** Current session log directory, e.g. Log/2026-07-03_17-16-30 */
std::string getFastLivoLogDir();

/** Base Log directory under project root */
std::string getFastLivoLogBaseDir();

/** Resolve output path into current session dir (strips optional "Log/" prefix) */
std::string resolveFastLivoLogPath(const std::string& path);

/** Resolve path relative to project root (for loading maps from previous runs) */
std::string resolveRootRelativePath(const std::string& path);

#endif  // LOG_INIT_H
