#ifndef EASINGSMOVEMENT_H
#define EASINGSMOVEMENT_H

#include <itu_common.hpp>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif



static vec2f  newEasingPosition(vec2f Start,vec2f End,float t){
		float newY = Start.y + (End.y - Start.y) * t;
		float newX = Start.x + (End.x - Start.x) * t;
  return(vec2f{newX,newY});
}
  static float easeInSine(float t)
  {
    return 1.0 - std::cos((t * M_PI) / 2.0);
  }
  static float easeOutSine(float t)
  {
    return std::cos((t * M_PI) / 2);
  }
  static float easeInOutSine(float t)
  {
    return -(std::cos(M_PI * t) - 1) / 2;
  }
  static float easeInQuad(float t)
  {
    return t * t;
  }
  static float easeOutQuad(float t)
  {
    return 1 - (1 - t) * (1 - t);
  }
  static float easeInOutQuad(float t)
  {
    return t < 0.5 ? 2 * t * t : 1 - std::pow(-2 * t + 2, 2) / 2;
  }
  static float easeInCubic(float t)
  {
    return t * t * t;
  }
  static float easeOutCubic(float t)
  {
    return 1 - std::pow(1 - t, 3);
  }
  static float easeInOutCubic(float t)
  {
    return t < 0.5 ? 4 * t * t * t : 1 - std::pow(-2 * t + 2, 3) / 2;
  }
  static float easeInQuart(float t)
  {
    return t * t * t * t;
  }
  static float easeOutQuart(float t)
  {
    return 1 - std::pow(1 - t, 4);
  }
  static float easeInOutQuart(float t)
  {
    return t < 0.5 ? 8 * t * t * t * t : 1 - std::pow(-2 * t + 2, 4) / 2;
  }
  static float easeInQuint(float t)
  {
    return t * t * t * t * t;
  }
  static float easeOutQuint(float t)
  {
    return 1 - std::pow(1 - t, 5);
  }
  static float easeInOutQuint(float t)
  {
    return t < 0.5 ? 16 * t * t * t * t * t : 1 - std::pow(-2 * t + 2, 5) / 2;
  }
  static float easeInExpo(float t)
  {
    return (t == 0.0) ? 0.0 : std::pow(2.0, 10.0 * t - 10.0);
  }
  static float easeOutExpo(float t)
  {
    return (t == 1.0f) ? 1.0f : 1.0f - std::pow(2.0f, -10.0f * t);
  }
  static float easeOutCirc(float t)
  {
    return std::sqrt(1 - std::pow(t - 1, 2));
  }
  static float easeInOutCirc(float t)
  {
    return t < 0.5
               ? (1 - std::sqrt(1 - std::pow(2 * t, 2))) / 2
               : (std::sqrt(1 - std::pow(-2 * t + 2, 2)) + 1) / 2;
  }
  static float easeInBack(float t)
  {
    float c1 = 1.70158;
    float c3 = c1 + 1;

    return c3 * t * t * t - c1 * t * t;
  }
  static float easeOutBack(float t)
  {
    float c1 = 1.70158;
    float c3 = c1 + 1;

    return 1 + c3 * std::pow(t - 1, 3) + c1 * std::pow(t - 1, 2);
  }
  static float easeInOutExpo(float t)
  {
    return (t == 0.0f)   ? 0.0f
           : (t == 1.0f) ? 1.0f
           : (t < 0.5f)  ? std::pow(2.0f, 20.0f * t - 10.0f) / 2.0f
                         : (2.0f - std::pow(2.0f, -20.0f * t + 10.0f)) / 2.0f;
  }
  static float easeInCirc(float t)
  {
    return 1 - std::sqrt(1 - std::pow(t, 2));
  }
  static float easeInOutBack(float t)
  {
    float c1 = 1.70158;
    float c2 = c1 * 1.525;

    return t < 0.5
               ? (std::pow(2 * t, 2) * ((c2 + 1) * 2 * t - c2)) / 2
               : (std::pow(2 * t - 2, 2) * ((c2 + 1) * (t * 2 - 2) + c2) + 2) / 2;
  }
  static float easeInElastic(float t)
  {
    const float c4 = (2.0f * M_PI) / 3.0f;

    if (t == 0.0f)
      return 0.0f;
    if (t == 1.0f)
      return 1.0f;

    return -std::pow(2.0f, 10.0f * t - 10.0f) *
           std::sin((t * 10.0f - 10.75f) * c4);
  }
  static float easeOutElastic(float t)
  {
    const float c4 = (2.0f * M_PI) / 3.0f;

    if (t == 0.0f)
      return 0.0f;
    if (t == 1.0f)
      return 1.0f;

    return std::pow(2.0f, -10.0f * t) *
               std::sin((t * 10.0f - 0.75f) * c4) +
           1.0f;
  }
  static float easeInOutElastic(float t)
  {
    const float c5 = (2.0f * M_PI) / 4.5f;

    if (t == 0.0f)
      return 0.0f;
    if (t == 1.0f)
      return 1.0f;

    if (t < 0.5f)
    {
      return -(std::pow(2.0f, 20.0f * t - 10.0f) *
               std::sin((20.0f * t - 11.125f) * c5)) /
             2.0f;
    }
    else
    {
      return (std::pow(2.0f, -20.0f * t + 10.0f) *
              std::sin((20.0f * t - 11.125f) * c5)) /
                 2.0f +
             1.0f;
    }
  }
 
  static float easeOutBounce(float t)
  {
    float n1 = 7.5625;
    float d1 = 2.75;

    if (t < 1 / d1)
    {
      return n1 * t * t;
    }
    else if (t < 2 / d1)
    {
      return n1 * (t -= 1.5 / d1) * t + 0.75;
    }
    else if (t < 2.5 / d1)
    {
      return n1 * (t -= 2.25 / d1) * t + 0.9375;
    }
    else
    {
      return n1 * (t -= 2.625 / d1) * t + 0.984375;
    }
  }
   static float easeInBounce(float t)
  {
    return 1 - easeOutBounce(1 - t);
  }
  static float easeInOutBounce(float t)
  {
    return t < 0.5
               ? (1 - easeOutBounce(1 - 2 * t)) / 2
               : (1 + easeOutBounce(2 * t - 1)) / 2;
  }

#endif // EASINGSMOVEMENT_H