#ifndef RATE_LIMITER_H
#define RATE_LIMITER_H

#include <functional>
#include "SFML/System.hpp"

class RateLimiter {
private:
  const float frequency;
  const float delay;
  int count;
  sf::Clock lastRateCount;
  sf::Clock lastRun;
public:
  RateLimiter(float frequency)
  : frequency(frequency), delay(1 / frequency), lastRun(), count(0), lastRateCount() {}
  float operator()() {
    float remainingSeconds = delay - lastRun.getElapsedTime().asSeconds();
    sf::sleep(sf::seconds(remainingSeconds));
    lastRun.restart();
    count++;
    if (remainingSeconds >= 0) {
      return delay;
    } else {
      return delay - remainingSeconds;
    }
  }
  float rate() {
    float rate = count / lastRateCount.getElapsedTime().asSeconds();
    count = 0;
    lastRateCount.restart();
    return rate;
  }
};

#endif /* !RATE_LIMITER_H */