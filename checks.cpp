/*
 *  Copyright 2006 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

// Most of this was borrowed (with minor modifications) from V8's and Chromium's
// src/base/logging.cc.

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
//#include "pch.h"

#if defined(WEBRTC_ANDROID)
#define RTC_LOG_TAG_ANDROID "rtc"
#include <android/log.h>  // NOLINT
#endif

#if defined(WEBRTC_WIN)
#include <windows.h>
#endif

#if defined(WEBRTC_WIN)
#define LAST_SYSTEM_ERROR (::GetLastError())
#elif defined(__native_client__) && __native_client__
#define LAST_SYSTEM_ERROR (0)
#elif defined(WEBRTC_POSIX)
#include <errno.h>
#define LAST_SYSTEM_ERROR (errno)
#endif  // WEBRTC_WIN

#include "checks.h"

#if defined(_MSC_VER)
// Warning C4722: destructor never returns, potential memory leak.
// FatalMessage's dtor very intentionally aborts.
#pragma warning(disable:4722)
#endif

namespace rtc {
namespace {

void VPrintError(const char* format, va_list args) {
#if defined(WEBRTC_ANDROID)
  __android_log_vprint(ANDROID_LOG_ERROR, RTC_LOG_TAG_ANDROID, format, args);
#else
  vfprintf(stderr, format, args);
#endif
}

#if defined(__GNUC__)
void PrintError(const char* format, ...)
    __attribute__((__format__(__printf__, 1, 2)));
#endif

void PrintError(const char* format, ...) {
  va_list args;
  va_start(args, format);
  VPrintError(format, args);
  va_end(args);
}

}  // namespace

FatalMessage::FatalMessage(const char* file, int line) {
  Init(file, line);
}

FatalMessage::FatalMessage(const char* file, int line, std::string* result) {
  Init(file, line);
  stream_ << "Check failed: " << *result << std::endl << "# ";
  delete result;
}

RTC_NORETURN FatalMessage::~FatalMessage() {
  fflush(stdout);
  fflush(stderr);
  stream_ << std::endl << "#" << std::endl;
  PrintError("%s", stream_.str().c_str());
  fflush(stderr);
  abort();
}

void FatalMessage::Init(const char* file, int line) {
  stream_ << std::endl
          << std::endl
          << "#" << std::endl
          << "# Fatal error in " << file << ", line " << line << std::endl
          << "# last system error: " << LAST_SYSTEM_ERROR << std::endl
          << "# ";
}

// MSVC doesn't like complex extern templates and DLLs.
#if !defined(COMPILER_MSVC)
// Explicit instantiations for commonly used comparisons.
template std::string* MakeCheckOpString<int, int>(
    const int&, const int&, const char* names);
template std::string* MakeCheckOpString<unsigned long, unsigned long>(
    const unsigned long&, const unsigned long&, const char* names);
template std::string* MakeCheckOpString<unsigned long, unsigned int>(
    const unsigned long&, const unsigned int&, const char* names);
template std::string* MakeCheckOpString<unsigned int, unsigned long>(
    const unsigned int&, const unsigned long&, const char* names);
template std::string* MakeCheckOpString<std::string, std::string>(
    const std::string&, const std::string&, const char* name);
#endif

}  // namespace rtc

// Function to call from the C version of the RTC_CHECK and RTC_DCHECK macros.
RTC_NORETURN void rtc_FatalMessage(const char* file, int line,
                                   const char* msg) {
  rtc::FatalMessage(file, line).stream() << msg;
}
