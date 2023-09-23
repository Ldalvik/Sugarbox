#include "Sugarbox.h"
#include "Free_Fonts.h"

// WiFi network credentials
#define SSID "YourSSID"
#define SSID_PASSWORD "YourSSIDPassword"

// Note: Dexcom FOLLOW has a different login portal and account than your normal dexcom account.
#define DEXCOM_USERNAME "YourUsername"
#define DEXCOM_PASSWORD "YourPassword"

Dexcom dexcom = Dexcom(DEXCOM_USERNAME, DEXCOM_PASSWORD);
Sugarbox sugarbox = Sugarbox(&dexcom);

// This will change depending on the driver
#include "SPI_9488.h"
LGFX lcd;

void setup() {
  Serial.begin(115200);

  lcd.init();
  lcd.fillScreen(TFT_BLACK);
  lcd.setRotation(1);

  // testValuesAndTrends();

  startup();

  dexcom.setLowThreshold(90);
  dexcom.setHighThreshold(250);

  // Initial run to display a value on boot up
  sugarbox.run(sugarbox.updateFreq,
               onTooHigh,
               onInRange,
               onTooLow,
               onAttentionRequired,
               onDoubleUp,
               onSingleUp,
               onFortyFiveUp,
               onSteady,
               onFortyFiveDown,
               onSingleDown,
               onDoubleDown,
               onError);
}

void onTooHigh(int currentGlucose) {
  lcd.setTextColor(TFT_ORANGE);
  setGlucose(currentGlucose);
}

void onInRange(int currentGlucose) {
  lcd.setTextColor(TFT_GREEN);
  setGlucose(currentGlucose);
}

void onTooLow(int currentGlucose) {
  lcd.setTextColor(TFT_RED);
  setGlucose(currentGlucose);
}

void onAttentionRequired(int currentGlucose) {
  lcd.setTextColor(TFT_CYAN);
  setGlucose(currentGlucose);
}

void onDoubleUp() {
  drawTrend("RISING FAST", TFT_RED, 7);
}

void onSingleUp() {
  drawTrend("RISING", TFT_ORANGE, 7);
}

void onFortyFiveUp() {
  drawTrend("RISING SLIGHTLY", TFT_ORANGE, 5);
}

void onSteady() {
  drawTrend("STEADY :)", TFT_GREEN, 7);
}

void onFortyFiveDown() {
  drawTrend("FALLING SLIGHTLY", TFT_YELLOW, 5);
}

void onSingleDown() {
  drawTrend("FALLING", TFT_RED, 7);
}

void onDoubleDown() {
  drawTrend("FALLING FAST", TFT_RED, 6);
}

void onError() {
  drawTrend("Error :(", TFT_WHITE, 7);
}

void drawTrend(String text, const int color, int fontSize) {
  int width = lcd.width() / 2 + 5;
  int height = lcd.height();
  int font = 0;

  lcd.setTextDatum(BC_DATUM);
  lcd.setTextFont(font);
  lcd.setTextSize(fontSize);
  lcd.setTextColor(color);

  lcd.drawString(text, width, height);
}

int lastGlucoseReading = 0;
void setGlucose(int glucose) {
  lcd.fillScreen(TFT_BLACK);  // first thing called on glucose check, clear screen here
  {
    int width = lcd.width() / 2 + 20;
    int height = 10;
    int font = 1;
    int textSize = 27;
    if (glucose > 99 && glucose < 200 && glucose % 10 != 1) width -= 20;  // numbers ending or starting with 1 need to be adjusted with this font

    lcd.setTextDatum(TC_DATUM);
    lcd.setTextFont(font);
    lcd.setTextSize(textSize);

    lcd.drawNumber(glucose, width, height);
  }
  {
    int width = lcd.width() - 20;
    int height = 240;
    int font = 1;
    int textSize = 3;

    lcd.setTextDatum(MR_DATUM);
    lcd.setTextFont(font);
    lcd.setTextSize(textSize);

    int gluc = dexcom.currentGlucose;
    int highThres = dexcom.highThreshold;
    int lowThres = dexcom.lowThreshold;
    int warnThres = dexcom.warningThreshold;

    uint16_t color;
    if (gluc > (highThres + warnThres) || gluc < (lowThres - warnThres)) {
      color = TFT_CYAN;   // above or below warning threshold
    } else if (gluc > highThres) {
      color = TFT_ORANGE; // above high threshold
    } else if (gluc > lowThres && gluc < highThres) {
      color = TFT_GREEN;  // in-between high and low threshold
    } else if (gluc < lowThres) {
      color = TFT_RED;    // below low threshold
    }
    lcd.setTextColor(color, TFT_BLACK);
    lcd.drawNumber(lastGlucoseReading, width, height);

    lastGlucoseReading = glucose;
  }
}

// Countdown timer for the next blood sugar reading
unsigned long lastMillis = 0;
void nextReading() {
  if (millis() - lastMillis >= 1000) {
    int width = 20;
    int height = 240;
    int font = 1;
    int textSize = 3;

    lcd.setTextDatum(ML_DATUM);
    lcd.setTextFont(font);
    lcd.setTextSize(textSize);
    lcd.setTextColor(TFT_WHITE, TFT_BLACK);

    unsigned long nextCheck = sugarbox.updateFreq - (millis() - sugarbox.LAST_GLUCOSE_CHECK);
    String seconds = (nextCheck % 60000 / 1000 < 10 ? "0" : "") + String(nextCheck % 60000 / 1000);
    String minutes = String(nextCheck / 60000);

    lcd.drawString(minutes + ":" + seconds, width, height);

    lastMillis = millis();
  }
}

void loop() {
  nextReading();
  sugarbox.run(millis(),
               onTooHigh,
               onInRange,
               onTooLow,
               onAttentionRequired,

               onDoubleUp,
               onSingleUp,
               onFortyFiveUp,
               onSteady,
               onFortyFiveDown,
               onSingleDown,
               onDoubleDown,
               onError);
}

// Error response types for failed Dexcom::createSession();
void printError() {
  switch (dexcom.currentStatus) {
    case SESSION_NOT_VALID: Serial.println("Session ID invalid"); break;
    case SESSION_NOT_FOUND: Serial.println("Session not found"); break;
    case ACCOUNT_NOT_FOUND: Serial.println("Account not found"); break;
    case PASSWORD_INVALID: Serial.println("Password invalid"); break;
    case MAX_ATTEMPTS: Serial.println("Maximum authentication attempts exceeded"); break;
    case USERNAME_NULL_EMPTY: Serial.println("Username NULL or empty"); break;
    case PASSWORD_NULL_EMPTY: Serial.println("Password NULL or empty"); break;
    default: Serial.println("Unknown error"); break;
  }
}

void startup() {
  lcd.setTextDatum(MC_DATUM);
  lcd.setTextSize(9);

  char characters[] = { 'S', 'u', 'g', 'a', 'r', 'b', 'o', 'x' };
  uint16_t colors[] = { TFT_RED, TFT_ORANGE, TFT_YELLOW, TFT_GREEN, TFT_BLUE, TFT_MAGENTA, TFT_VIOLET, TFT_CYAN };
  int padding = 50;

  for (int i = 0; i < sizeof(characters) / sizeof(characters[0]); i++) {
    lcd.setTextColor(colors[i]);
    lcd.drawString(String(characters[i]), 70 + (i * padding), 80);
    delay(100);
  }

  {
    int width = lcd.width() / 2;
    int height = lcd.height() / 2 - 10;

    lcd.setTextSize(4);
    lcd.setTextColor(TFT_WHITE);
    lcd.drawString("Blood glucose", width, height);
    lcd.drawString("monitor", width, height + 35);
  }

  WiFi.mode(WIFI_OFF);
  WiFi.begin(SSID, SSID_PASSWORD);

  {
    int height = lcd.height() - 50;
    int width = 10;

    lcd.setTextDatum(BL_DATUM);
    lcd.setTextSize(2);
    lcd.drawString("Connecting to WIFI...", width, height);

    unsigned long timeout = 10000, startMillis = millis();
    while (WiFi.status() != WL_CONNECTED) {
      if (millis() - startMillis >= timeout) {
        lcd.setTextColor(TFT_RED);
        lcd.drawString("failed :(", width + 280, height);
      }
    }

    lcd.setTextColor(TFT_GREEN);
    lcd.drawString("connected!", width + 280, height);

    lcd.setTextColor(TFT_WHITE);
    lcd.drawString("Logging into dexcom...", width, height + 25);

    if (dexcom.createSession()) {
      lcd.setTextColor(TFT_GREEN);
      lcd.drawString("logged in!", width + 280, height + 25);
    } else {
      printError();
    }
  }
  delay(500);
  lcd.fillScreen(TFT_BLACK);
}
