#ifndef RATE_LIMITER_H
#define RATE_LIMITER_H

#include <functional>
#include "SFML/System.hpp"

/**
 * @brief Limits the execution into "frequency" per second
 */
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

  /**
   * @brief Sleeps to limit the rate
   * 
   * @return float the time passed between the last call and when this call returns
   */
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

  /**
   * @return float the calculated Frames Per Second, averaged
   */
  float rate() {
    float rate = count / lastRateCount.getElapsedTime().asSeconds();
    count = 0;
    lastRateCount.restart();
    return rate;
  }
};

#endif /* !RATE_LIMITER_H */