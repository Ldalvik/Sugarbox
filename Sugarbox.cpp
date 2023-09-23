#include "Sugarbox.h"

Sugarbox::Sugarbox(Dexcom *dexcom, int updateFreq, int alarmFreq)
  : dexcom(dexcom), updateFreq(updateFreq), alarmFreq(alarmFreq) {
}

void Sugarbox::run(unsigned long millis,
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
                   OnError onError) {
  if (millis - LAST_GLUCOSE_CHECK >= updateFreq) {
    if (dexcom->currentStatus != LOGGED_IN) return;
    if (dexcom->update()) {
      int gluc = dexcom->currentGlucose;
      int highThres = dexcom->highThreshold;
      int lowThres = dexcom->lowThreshold;
      int warnThres = dexcom->warningThreshold;

      if (gluc > highThres + warnThres || gluc < lowThres - warnThres) {  // above or below the warning threshold
        onAttentionRequired(gluc);
      } else if (gluc > highThres) {  // above high threshold
        onTooHigh(gluc);
      } else if (gluc > lowThres && gluc < highThres) {  // in-between high and low threshold
        onInRange(gluc);
      } else if (gluc < lowThres) {  // below threshold
        onTooLow(gluc);
      }

      switch (dexcom->currentTrend) {
        case DOUBLE_UP: onDoubleUp(); break;
        case SINGLE_UP: onSingleUp(); break;
        case FORTYFIVE_UP: onFortyFiveUp(); break;
        case FLAT: onSteady(); break;
        case FORTYFIVE_DOWN: onFortyFiveDown(); break;
        case SINGLE_DOWN: onSingleDown(); break;
        case DOUBLE_DOWN: onDoubleDown(); break;
        case NOT_COMPUTABLE:
        case RATE_OUT_OF_RANGE:
        default: onError(); break;
      }
    } else {
      
    }
    LAST_GLUCOSE_CHECK = millis;
  }
}
