#ifndef Sugarbox_h
#define Sugarbox_h

#include <Dexcom.h>

class Sugarbox {
public:
  using OnTooHigh = std::function<void(int)>;
  using OnInRange = std::function<void(int)>;
  using OnTooLow = std::function<void(int)>;
  using OnAttentionRequired = std::function<void(int)>;

  using OnDoubleUp = std::function<void()>;
  using OnSingleUp = std::function<void()>;
  using OnFortyFiveUp = std::function<void()>;
  using OnSteady = std::function<void()>;
  using OnFortyFiveDown = std::function<void()>;
  using OnSingleDown = std::function<void()>;
  using OnDoubleDown = std::function<void()>;
  using OnError = std::function<void()>;

  Sugarbox(Dexcom *dexcom, int updateFreq = 300000, int alarmFreq = 300000);
  void run(unsigned long millis,
           OnTooHigh onTooHigh,
           OnInRange onInRange,
           OnTooLow onTooLow,
           OnAttentionRequired onAttentionRequired,

           OnDoubleUp onDoubleUp,
           OnSingleUp onSingleUp,
           OnFortyFiveUp onFortyFiveUp,
           OnSteady onSteady,
           OnFortyFiveUp onFortyFiveDown,
           OnSingleDown onSingleDown,
           OnDoubleDown onDoubleDown,
           OnError onError);

  unsigned long LAST_ALARM_SOUND = alarmFreq;
  unsigned long LAST_GLUCOSE_CHECK = updateFreq;

  int updateFreq;
  int alarmFreq;
private:
  Dexcom *dexcom;
};

#endif