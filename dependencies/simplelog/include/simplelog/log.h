#pragma once

#include <cstddef>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>
#include <iostream>

#define LOG_DEBUG(msg, ...) Log::getInstance().log(LogLevel::DEBUG, msg, ##__VA_ARGS__)
#define LOG_INFO(msg, ...) Log::getInstance().log(LogLevel::INFO, msg, ##__VA_ARGS__)
#define LOG_SUCCESS(msg, ...) Log::getInstance().log(LogLevel::SUCCESS, msg, ##__VA_ARGS__)
#define LOG_WARNING(msg, ...) Log::getInstance().log(LogLevel::WARNING, msg, ##__VA_ARGS__)
#define LOG_ERROR(msg, ...) Log::getInstance().log(LogLevel::ERROR, msg, ##__VA_ARGS__)

constexpr const char* DEBUG = "\033[38;5;223m[DEBUG] ";
constexpr const char* INFO = "\033[38;5;223m[INFO] ";
constexpr const char* SUCCESS = "\033[38;5;142m[SUCCESS] ";
constexpr const char* WARNING = "\033[38;5;208m[WARNING] ";
constexpr const char* ERROR = "\033[38;5;124m[ERROR] ";
constexpr const char* RESET = "\033[0m";




enum class LogLevel {
  DEBUG,
  INFO,
  SUCCESS,
  WARNING,
  ERROR,
};


class Log {
public:
  Log(const Log&) = delete;
  Log& operator=(const Log&) = delete;

private:    
  LogLevel currentLevel;
  Log();

public:
  static Log& getInstance();
  void setLevel(LogLevel level);
  
  template<typename... Args>
  void log(LogLevel maxLevel, std::string msg, Args... args) {
    if(maxLevel < currentLevel)
      return;

    std::vector<char*> strs = split(msg);
    size_t argc = sizeof...(args);
    if(argc != strs.size()-1) {
      throw std::runtime_error("Invalid log argument count");
      return;
    }

    auto argsTuple = std::make_tuple(args...);
    std::stringstream ss;
    setPreview(ss, maxLevel);
    joinArg<0>(ss, strs, argsTuple);
    ss << RESET;

    std::cout << ss.str() << std::endl;

  }



private:
  std::vector<char*> split(std::string& msgStr);
  void setPreview(std::stringstream& ss, LogLevel level);
  
  template<int index, typename... Args>
  typename std::enable_if<index < sizeof...(Args)>::type
  joinArg(std::stringstream& ss, 
    const std::vector<char *>& strs,
    const std::tuple<Args...>& args) {

      ss << strs[index] << std::get<index>(args);
      joinArg<index+1>(ss, strs, args);
  }

  template<int index, typename... Args>
  typename std::enable_if<index == sizeof...(Args)>::type
  joinArg(std::stringstream& ss, 
    const std::vector<char *>& strs,
    const std::tuple<Args...>& args) {

      ss << strs[index];
  }

};
