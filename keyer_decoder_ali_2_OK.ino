#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x3f, 20, 4);

// Pinler
#define DOT_PIN 2
#define DASH_PIN 3
#define SIDETONE 12


// WPM 20
#define DOT_TIME 55  // BU DEĞER 60 İDİ

#define DASH_TIME (DOT_TIME * 3)
#define ELEMENT_GAP DOT_TIME
#define LETTER_GAP (DOT_TIME * 2)///  buradaki değer 3 idi, kelimeyi anlamıyordu 2 tam oldu

#define WORD_GAP (DOT_TIME * 5) // BURADAKİ DEĞER 6 İDİ 5 DAHA İYİ uyum sağladı
//////////buraları satırlar sırasıyla yazılsın diye ekledik///////////
int col = 0;
int row = 0;
//////////////////////////


// State
bool keying = false;
bool lastWasDot = false;
bool squeeze = false;

unsigned long timer = 0;
String morseBuffer = "";
unsigned long lastActivity = 0;

// Morse tablo
struct MorseMap {
  const char* code;
  char letter;
};

MorseMap table[] = {
  {".-", 'A'}, {"-...", 'B'}, {"-.-.", 'C'}, {"-..", 'D'},
  {".", 'E'}, {"..-.", 'F'}, {"--.", 'G'}, {"....", 'H'},
  {"..", 'I'}, {".---", 'J'}, {"-.-", 'K'}, {".-..", 'L'},
  {"--", 'M'}, {"-.", 'N'}, {"---", 'O'}, {".--.", 'P'},
  {"--.-", 'Q'}, {".-.", 'R'}, {"...", 'S'}, {"-", 'T'},
  {"..-", 'U'}, {"...-", 'V'}, {".--", 'W'}, {"-..-", 'X'},
  {"-.--", 'Y'}, {"--..", 'Z'},
  {"-----",'0'},{".----",'1'},{"..---",'2'},{"...--",'3'},
  {"....-",'4'},{".....",'5'},{"-....",'6'},{"--...",'7'},
  {"---..",'8'},{"----.",'9'},
};

char decode(String code) {
  for (int i = 0; i < sizeof(table)/sizeof(MorseMap); i++) {
    if (code == table[i].code) return table[i].letter;
  }
  return '?';
}

// Sidetone
void toneOn() {
  tone(SIDETONE, 800);
}

void toneOff() {
  noTone(SIDETONE);
}

// Paddle okuma
bool dotPressed() { return !digitalRead(DOT_PIN); }
bool dashPressed() { return !digitalRead(DASH_PIN); }

// Eleman üretimi
void sendElement(bool isDot) {
  keying = true;
  lastWasDot = isDot;

  if (isDot) {
    morseBuffer += ".";
    toneOn();
    delay(DOT_TIME);
  } else {
    morseBuffer += "-";
    toneOn();
    delay(DASH_TIME);
  }

  toneOff();
  delay(ELEMENT_GAP);

  keying = false;
  lastActivity = millis();
}

// IAMBIC MODE B ENGINE
void handleKeyer() {
  bool dot = dotPressed();
  bool dash = dashPressed();

  // squeeze detection
  squeeze = dot && dash;

  if (!keying) {

    if (squeeze) {
      // Alternating sequence
      if (lastWasDot) {
        sendElement(false);
      } else {
        sendElement(true);
      }
    }
    else if (dot) {
      sendElement(true);
    }
    else if (dash) {
      sendElement(false);
    }
  }
}

// Decoder kontrol
void handleDecoder() {
  if (morseBuffer.length() > 0 &&
      millis() - lastActivity > LETTER_GAP) {

    char c = decode(morseBuffer);
   // lcd.print(c); //  bunu değiştirdik alttakiye
    lcdWriteChar(c);
    morseBuffer = "";
  }

  if (millis() - lastActivity > WORD_GAP) {
    //lcd.print(" ");
    lcdWriteChar(' ');
    lastActivity = millis();
  }
}
/////////lcd satırlar  1 2 3 4 sırasıyla görünsün diye///////////// 

void lcdWriteChar(char c) {
  lcd.setCursor(col, row);
  lcd.print(c);

  col++;

  if (col >= 20) {
    col = 0;
    row++;

    if (row >= 4) {
      row = 0;
      lcd.clear();
    }
  }
}
void setup() {
  pinMode(DOT_PIN, INPUT_PULLUP);
  pinMode(DASH_PIN, INPUT_PULLUP);
  pinMode(SIDETONE, OUTPUT);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("TA3OD Iambic Keyer B");
  lcd.setCursor(0,1);
  lcd.print("WPM: 25");

  delay(1500);
  lcd.clear();
}

void loop() {
  handleKeyer();
  handleDecoder();
}