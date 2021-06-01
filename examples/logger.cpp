#include "effortless/logger.hpp"

#include <catch2/catch.hpp>
#include <chrono>
#include <cmath>
#include <thread>

#include "effortless/throttler.hpp"
#include "effortless/timer.hpp"

using namespace effortless;

TEST_CASE("Logger: Simple Logging", "[logger]") {
  Logger logger("Test");
  Timer timer("Printing");
  timer.tic();

  logger << "This is a text stream log." << std::endl;

  logger.info("This is an info log.");
  logger.warn("This could be a warning, but just for demo.");
  logger.error("This could be an error, but just for demo.");

  logger.info(
    "You can print strings like %%s as \"%s\", and formatted numbers like "
    "%%1.3f as %1.3f.",
    "text", M_PI);

  logger
    << "You can use the stream operator \'<<\' just like with \'std::cout\'."
    << std::endl;
  logger << "This can be helpul for printing complex objects like matrices "
            "from the Eigen library."
         << std::endl;

  timer.toc();
  logger << "Or also effortless objects, like a timer:" << std::endl
         << timer << std::endl;
}

TEST_CASE("Logger: Colorless Logging", "[logger]") {
  Logger logger("Test", false);

  logger << "This is a text stream log without colors but with indicators."
         << std::endl;
  logger.info("This is an info log.");
  logger.warn("This could be a warning, but just for demo.");
  logger.error("This could be an error, but just for demo.");
}

TEST_CASE("Logger: Fatal Logging", "[logger]") {
  Logger logger("Test");

  try {
    logger.fatal(
      "Fatal messages can also throw a runtime error to abort a program.");
    FAIL_CHECK("Expected logger to throw exception.");
  } catch (const std::exception&) {
  }
}

struct LoggerConstCaller {
  Logger logger{"LoggerConstCaller"};
  void printInfo(const std::string& info) const { logger.info(info.c_str()); }
};

TEST_CASE("Logger: Logging from constant functions", "[logger]") {
  const LoggerConstCaller caller;
  caller.printInfo("This should log just fine from a const call!");
}


TEST_CASE("Logger: Throttled Logging", "[logger]") {
  Logger logger("ThrottledLogger");
  Throttler throttler(logger, 1.0);

  for (int i = 0; i < 101; ++i) {
    throttler(&Logger::info, "This should only appear %d times.", 2);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}

TEST_CASE("Logger: Debug Logging", "[logger]") {
  Logger logger("DebugLogger");

  logger.debug("This only prints with %s", "debug logging enabled!");
  logger.debug() << "It also provides a stream device to log debug info!"
                 << std::endl;
  logger.debug([&]() {
    logger.info(
      "Or a lambda that only counts to 10 with debug logging enabled");
    for (int i = 1; i <= 10; ++i) logger << i << std::endl;
  });
#ifndef DEBUG_LOG
  logger.debug(
    "And by the way, these debug messages are completely optimized away if not "
    "enabled through a compiler flag.\n%s",
    std::string("Otherwise this would generate a segmentation fault!").data() +
      (2 << 16));
#endif
}