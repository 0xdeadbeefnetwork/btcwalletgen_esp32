#pragma GCC optimize("Ofast")

#include <Wire.h>
#include <TFT_eSPI.h>
#include <SparkFun_ATECCX08a_Arduino_Library.h>
#include <Bitcoin.h>
#include <Hash.h>
#include <Conversion.h>
#include "BitcoinLogo.h"
#include <DigitalRainAnimation.hpp>

// ==== PIN DEFINITIONS ====
#define SDA_PIN 27
#define SCL_PIN 22
#define BOOT_BUTTON_PIN 0  // GPIO0 — BOOT button

#define BACKGROUND_COLOR 0x0000
#define TEXT_COLOR       0x07E0
#define LOGO_COLOR       0x07E0

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

// ==== GLOBAL OBJECTS ====
TFT_eSPI tft = TFT_eSPI();
DigitalRainAnimation<TFT_eSPI> digitalRain;
ATECCX08A atecc;

// ==== GLOBAL STATE ====
String btcAddress = "";
String seedPhrase = "";
HDPrivateKey hdMasterKey;
int statusLine = 0;

// ==== FUNCTION DECLARATIONS ====
void initializeDisplay();
void drawBitcoinLogo();
void simulateCypherpunkBoot();
void drawMatrixEffect();
void statusMsg(const char* msg);
bool validateEntropyFromATECC(uint8_t* buffer);
bool generateEntropy(uint8_t* entropy, size_t size);
void generateBitcoinWallet(uint8_t* entropy);
void displayWalletInfo();
void secureWipe(uint8_t* buffer, size_t size);

// ==== SETUP ====
void setup() {
  Serial.begin(115200);
  pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);

  initializeDisplay();
  drawBitcoinLogo();
  delay(3000);
  tft.fillScreen(BACKGROUND_COLOR);
  simulateCypherpunkBoot();
  Wire.begin(SDA_PIN, SCL_PIN);

  statusMsg("[ 0.2000 ] Probing ATECC508A...");
  if (!atecc.begin()) {
    statusMsg("ERROR: ATECC508A init failed!");
    while (1);
  }
  statusMsg("Press BOOT to generate your stateless wallet...");
}

// ==== MAIN LOOP ====
void loop() {
  static bool buttonPreviouslyPressed = false;

  if (digitalRead(BOOT_BUTTON_PIN) == LOW && !buttonPreviouslyPressed) {
    buttonPreviouslyPressed = true;

    tft.fillScreen(BACKGROUND_COLOR);
    statusLine = 0;

    statusMsg("[ 1.0000 ] Gathering entropy...");
    uint8_t entropy[32];
    if (!generateEntropy(entropy, 32)) {
      statusMsg("ERROR: entropy failed.");
      delay(5000);
      return;
    }

    statusMsg("[ 1.1000 ] Validating entropy...");
    if (!validateEntropyFromATECC(entropy)) {
      statusMsg("FAIL: entropy invalid.");
      delay(5000);
      return;
    }

    statusMsg("[ 1.2000 ] Generating wallet...");
    generateBitcoinWallet(entropy);
    secureWipe(entropy, 32);

    drawMatrixEffect();
    displayWalletInfo();

  } else if (digitalRead(BOOT_BUTTON_PIN) == HIGH) {
    buttonPreviouslyPressed = false;
  }

  delay(100); // debounce
}

// ==== DISPLAY ====
void initializeDisplay() {
  tft.begin();
  tft.setRotation(1);
  tft.setTextFont(1);
  tft.setTextSize(1);
  tft.fillScreen(BACKGROUND_COLOR);
  digitalRain.init(&tft, true, false);
}

void drawBitcoinLogo() {
  tft.fillScreen(BACKGROUND_COLOR);
  tft.drawBitmap(
    (SCREEN_WIDTH - bitcoinLogoWidth) / 2,
    (SCREEN_HEIGHT - bitcoinLogoHeight) / 2,
    bitcoinLogo,
    bitcoinLogoWidth,
    bitcoinLogoHeight,
    LOGO_COLOR
  );
}

void simulateCypherpunkBoot() {
  tft.setTextColor(TEXT_COLOR);
  tft.setTextFont(1);
  tft.setTextSize(1);
  statusLine = 0;

  const char* boot[] = {
    "[ 0.0137 ] Purr.",
    "[ 0.0420 ] Checking for live ATECC508A.",
    "[ 0.0808 ] Yes!",
    "[ 0.1337 ] System Ready.",
    ">> Meow."
  };

  for (int i = 0; i < 5; i++) {
    statusMsg(boot[i]);
    delay(1000);
  }
  delay(1000);
}

void drawMatrixEffect() {
  unsigned long startTime = millis();
  while (millis() - startTime < 5000) digitalRain.loop();
}

void statusMsg(const char* msg) {
  tft.setCursor(10, 10 + (statusLine * 15));
  tft.println(msg);
  statusLine++;
}

// ==== WALLET ====
void generateBitcoinWallet(uint8_t* entropy) {
  seedPhrase = generateMnemonic(24, entropy, 32);
  hdMasterKey = HDPrivateKey(seedPhrase.c_str(), "");
  HDPrivateKey account = hdMasterKey.derive("m/84'/0'/0'/");
  HDPublicKey xpub = account.xpub();
  btcAddress = xpub.derive("m/0/0").address();
}

void secureWipe(uint8_t* buffer, size_t size) {
  for (size_t i = 0; i < size; i++) buffer[i] = 0;
}

bool generateEntropy(uint8_t* entropy, size_t size) {
  size_t written = 0;
  while (written < size) {
    if (!atecc.updateRandom32Bytes()) return false;
    size_t chunk = min(size - written, (size_t)32);
    memcpy(&entropy[written], atecc.random32Bytes, chunk);
    written += chunk;
  }
  return true;
}

bool validateEntropyFromATECC(uint8_t* buffer) {
  const size_t size = 32;
  int ones = 0;
  for (int i = 0; i < size; i++)
    for (int b = 0; b < 8; b++)
      if (buffer[i] & (1 << b)) ones++;
  float ratio = (float)ones / (size * 8);
  if (ratio < 0.40 || ratio > 0.60) return false;

  uint8_t prev = buffer[0];
  int repeat = 1;
  for (size_t i = 1; i < size; i++) {
    if (buffer[i] == prev) {
      if (++repeat > 8) return false;
    } else {
      repeat = 1;
      prev = buffer[i];
    }
  }

  int currentBit = (buffer[0] >> 7) & 1;
  int runLength = 1;
  for (size_t i = 0; i < size; i++) {
    for (int b = 6; b >= 0; b--) {
      int bit = (buffer[i] >> b) & 1;
      if (bit == currentBit) {
        if (++runLength > 26) return false;
      } else {
        currentBit = bit;
        runLength = 1;
      }
    }
  }

  return true;
}

// ==== DISPLAY RESULT ====
void displayWalletInfo() {
  tft.fillScreen(BACKGROUND_COLOR);
  tft.setTextColor(TEXT_COLOR);
  tft.setTextFont(1);
  tft.setTextSize(1);
  tft.setCursor(10, 10);

  tft.println("=== BITCOIN WALLET ===\n");

  tft.println("Address:");
  tft.println();
  tft.println(btcAddress);
  tft.println();
  tft.println("Seed Phrase:");
  tft.println();

  std::vector<String> words;
  int start = 0;
  for (int i = 0; i <= seedPhrase.length(); i++) {
    if (i == seedPhrase.length() || seedPhrase[i] == ' ') {
      words.push_back(seedPhrase.substring(start, i));
      start = i + 1;
    }
  }

  for (int i = 0; i < 24; i++) {
    tft.print(words[i] + " ");
    if ((i + 1) % 6 == 0) tft.println();
  }
}
