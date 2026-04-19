#include <Arduino.h>

constexpr uint8_t potent = 15;
constexpr uint8_t photoresistor = 14;

constexpr uint8_t greenPin = 13;
constexpr uint8_t redPin = 12;
constexpr uint8_t yellowPin = 11;

constexpr uint8_t resultWinPin = 10;
constexpr uint8_t resultLosePin = 9;

constexpr uint8_t buttonTactRedPin = 8;
constexpr uint8_t buttonYellowPin = 7;
constexpr uint8_t buttonGreenPin = 6;

enum GameState {
  STATE_WAIT = 0,
  STATE_PREPARE = 1,
  STATE_ACTION = 2,
  STATE_RESULT = 3,
  STATE_GAMEOVER = 4,

};
GameState currentState = STATE_WAIT;

uint32_t score = 0;
uint8_t mistakes = 0;
int targetLed = -1;
uint32_t startTime = 0;
uint32_t reactionTimeLimit = 2000;
bool lastWin = false;


// put function declarations here:


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  randomSeed(analogRead(potent));

  pinMode(buttonTactRedPin, INPUT_PULLUP);
  pinMode(buttonYellowPin, INPUT);
  pinMode(buttonGreenPin, INPUT);

  pinMode(redPin, OUTPUT);
  pinMode(yellowPin, OUTPUT);
  pinMode(greenPin, OUTPUT);

  pinMode(resultWinPin, OUTPUT);
  pinMode(resultLosePin, OUTPUT);
}

void loop() {
  uint32_t potentValue = analogRead(potent);
  uint32_t photoValue = analogRead(photoresistor);
  reactionTimeLimit = map(potentValue, 0, 4095, 2000, 200);
  uint32_t brightness = map(photoValue, 0, 4095, 10, 255);

  switch (currentState) {
    case STATE_WAIT: { 
      if (digitalRead(buttonTactRedPin) == LOW) {
        Serial.print("Limit: ");
        Serial.print(reactionTimeLimit);
        Serial.print(" ms | Brightness: ");
        Serial.println(brightness);
        score = 0;
        mistakes = 0;
        Serial.println("Game start!");
        while (digitalRead(buttonTactRedPin) == LOW);
        Serial.println("Get Ready...");
        analogWrite(redPin, brightness); delay(500);
        analogWrite(yellowPin, brightness); delay(500);
        analogWrite(greenPin, brightness); delay(500);
        analogWrite(redPin, 0); analogWrite(yellowPin, 0); analogWrite(greenPin, 0);
        delay(1000);
        currentState = STATE_PREPARE;
      }
      break;
    } 

    case STATE_PREPARE: { 
      uint8_t randomIndex = random(0, 3);
      uint8_t gameleds[] = {redPin, yellowPin, greenPin};
      targetLed = gameleds[randomIndex];
      analogWrite(targetLed, brightness);
      startTime = millis();
      currentState = STATE_ACTION;
      break;
    } 

    case STATE_ACTION: {
      uint32_t currentTime = millis();

      if (currentTime - startTime >= reactionTimeLimit) {
        Serial.println("Too slow!");
        mistakes++;
        analogWrite(targetLed, 0);
        lastWin = false;
        currentState = STATE_RESULT;
        break;
      }

      int pressedPin = -1;
      if (digitalRead(buttonTactRedPin) == LOW) {
        pressedPin = redPin;
      } else if (digitalRead(buttonYellowPin) == HIGH) {
        pressedPin = yellowPin;
      } else if (digitalRead(buttonGreenPin) == HIGH) {
        pressedPin = greenPin;
      }

      if (pressedPin != -1) {
        analogWrite(targetLed, 0);
        if (pressedPin == targetLed) {
          Serial.println("Correct!");
          score++;
          lastWin = true;
        } else {
          Serial.println("Wrong Button!");
          mistakes++;
          lastWin = false;
        }
        while (digitalRead(buttonTactRedPin) == LOW || digitalRead(buttonYellowPin) == HIGH || digitalRead(buttonGreenPin) == HIGH);
        currentState = STATE_RESULT;
      }
      break;
    } 

    case STATE_RESULT: { 
      if (lastWin) {
        digitalWrite(resultWinPin, HIGH);
        delay(300);
        digitalWrite(resultWinPin, LOW);
      } else {
        digitalWrite(resultLosePin, HIGH);
        delay(300);
        digitalWrite(resultLosePin, LOW);
      }

      if (mistakes >= 3) {
        currentState = STATE_GAMEOVER;
      } else {
        currentState = STATE_PREPARE;
      }
      break;
    } 

    case STATE_GAMEOVER: {
      Serial.printf("Game over! Your final score is %d\n", score);
      Serial.println("Press red button to restart");
      delay(1500);
      currentState = STATE_WAIT;
      break;
    }
  }
}