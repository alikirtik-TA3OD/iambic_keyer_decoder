#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x3f, 20, 4); // ekranda yazı yoksa  buradaki parantez içini (0x27, 20,4) olarak değiştir


// --- PINLER ---
#define DOT_PIN 2
#define DASH_PIN 3
#define SPEAKER_PIN 12

#define WPM_UP_PIN 4
#define WPM_DOWN_PIN 5


// --- WPM ---
int wpm = 25;/// BURADAKİ DEĞERİ DEĞİŞTİREREK BAŞLANGIÇTAKİ WPN HIZINI AYARLIYORUZ

int dotTime;
int dashTime;
int elementGap;
int letterGap;
int wordGap;

// --- MODE ---
bool iambicMode = true;

// --- STATE ---
bool keying = false;
bool lastWasDot = false;
bool squeeze = false;

String currentSymbol = "";
unsigned long lastActivity = 0;

// --- LCD CURSOR TAKİBİ ---
int cursorCol = 0;
int cursorRow = 0;

// --- WORD GAP FLAG ---
bool wordPrinted = false;

// --- MORSE ---
struct MorseMap {
  const char* code;
  char letter;
};

MorseMap morseTable[] = {
  {".-", 'A'}, {"-...", 'B'}, {"-.-.", 'C'}, {"-..", 'D'},
  {".", 'E'}, {"..-.", 'F'}, {"--.", 'G'}, {"....", 'H'},
  {"..", 'I'}, {".---", 'J'}, {"-.-", 'K'}, {".-..", 'L'},
  {"--", 'M'}, {"-.", 'N'}, {"---", 'O'}, {".--.", 'P'},
  {"--.-", 'Q'}, {".-.", 'R'}, {"...", 'S'}, {"-", 'T'},
  {"..-", 'U'}, {"...-", 'V'}, {".--", 'W'}, {"-..-", 'X'},
  {"-.--", 'Y'}, {"--..", 'Z'},
  {"-----",'0'}, {".----",'1'}, {"..---",'2'}, {"...--",'3'},
  {"....-",'4'}, {".....",'5'}, {"-....",'6'}, {"--...",'7'},
  {"---..",'8'}, {"----.",'9'}
};

// --- TIMING ---
void updateTiming() {
  dotTime = 1200 / wpm;
  dashTime = dotTime * 3;
  elementGap = dotTime;
  letterGap = dotTime * 2;
  wordGap = dotTime * 7; //  ilk deger 5 bosluk ince ayar
}

// --- INPUT ---
bool dotPressed() { return digitalRead(DOT_PIN) == LOW; }
bool dashPressed() { return digitalRead(DASH_PIN) == LOW; }

// --- SES ---
void toneOn() { tone(SPEAKER_PIN, 700); }
void toneOff() { noTone(SPEAKER_PIN); }

// --- LCD YAZMA (DÜZELTİLDİ) ---
void lcdPrintChar(char c) {
  lcd.setCursor(cursorCol, cursorRow);
  lcd.print(c);

  cursorCol++;

  if (cursorCol >= 20) {
    cursorCol = 0;
    cursorRow++;

    if (cursorRow >= 4) {
      lcd.clear();
      cursorRow = 0;
    }
  }
}

// --- GÖNDER ---
void sendElement(bool isDot) {
  keying = true;

  toneOn();
  if (isDot) delay(dotTime);
  else delay(dashTime);
  toneOff();

  delay(elementGap);

  if (isDot) {
    currentSymbol += ".";
    lastWasDot = true;
  } else {
    currentSymbol += "-";
    lastWasDot = false;
  }

  lastActivity = millis();
  wordPrinted = false; // yeni harf geldi → kelime flag sıfırla
  keying = false;
}

// --- DECODER ---
char decodeMorse(String code) {
  for (int i = 0; i < sizeof(morseTable)/sizeof(MorseMap); i++) {
    if (code == morseTable[i].code)
      return morseTable[i].letter;
  }
  return '?';
}

void handleDecoder() {

  // HARF ÇÖZME
  if (currentSymbol.length() > 0) {
    if (millis() - lastActivity > letterGap) {
      char c = decodeMorse(currentSymbol);
      lcdPrintChar(c);
      currentSymbol = "";
      wordPrinted = false; // yeni harf → kelime flag sıfırla
    }
  }

  // KELİME BOŞLUĞU (ARTIK ÇALIŞIR)
  if ((millis() - lastActivity > wordGap) && !wordPrinted) {
    lcdPrintChar(' ');
    wordPrinted = true;
  }
}

// --- KEYER ---
void handleKeyer() {
  bool dot = dotPressed();
  bool dash = dashPressed();

  if (!iambicMode) {
    if (dot) toneOn();
    else toneOff();
    return;
  }

  squeeze = dot && dash;

  if (!keying) {
    if (squeeze) {
      if (lastWasDot) sendElement(false);
      else sendElement(true);
    }
    else if (dot) sendElement(true);
    else if (dash) sendElement(false);
  }
}

// --- BUTON ---
void handleButtons() {
  static unsigned long lastDebounce = 0;
  if (millis() - lastDebounce < 200) return;

  if (!digitalRead(WPM_UP_PIN)) {
    wpm++;
    if (wpm > 40) wpm = 40;
    updateTiming();

    lcd.setCursor(0,3);
    lcd.print("WPM: ");
    lcd.print(wpm);
    lcd.print("   ");

    lastDebounce = millis();
  }

  if (!digitalRead(WPM_DOWN_PIN)) {
    wpm--;
    if (wpm < 5) wpm = 5;
    updateTiming();

    lcd.setCursor(0,3);
    lcd.print("WPM: ");
    lcd.print(wpm);
    lcd.print("   ");

    lastDebounce = millis();
  }


}

// --- SETUP ---
void setup() {
  pinMode(DOT_PIN, INPUT_PULLUP);
  pinMode(DASH_PIN, INPUT_PULLUP);
  pinMode(SPEAKER_PIN, OUTPUT);

  pinMode(WPM_UP_PIN, INPUT_PULLUP);
  pinMode(WPM_DOWN_PIN, INPUT_PULLUP);
  

  lcd.init();
  lcd.backlight();

  updateTiming();

  lcd.setCursor(0,0);
  lcd.print("   TA3OD");
lcd.setCursor(0,1);
  lcd.print("Cift Kol Maniple");

  lcd.setCursor(0,2);
  lcd.print("iambic Key");

  lcd.setCursor(0,3);
  lcd.print("WPM: ");
  lcd.print(wpm);
delay(3000);
lcd.clear();

}

// --- LOOP ---
void loop() {
  handleButtons();
  handleKeyer();
  handleDecoder();
}
