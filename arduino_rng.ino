#include <Base64.h>


/********************************/
/*  Rob Seward 2008-2009        */
/*  v1.0                        */
/*  4/20/2009                   */
/********************************/

#define BINS_SIZE 256
#define CALIBRATION_SIZE 50000

#define NO_BIAS_REMOVAL 0
#define EXCLUSIVE_OR 1
#define VON_NEUMANN 2

#define ASCII_BYTE 0
#define BINARY 1
#define ASCII_BOOL 2
#define KB_EMULATOR 3

#define KB_RAW_LEN 36
#define KB_B64_LEN 48

/***  Configure the RNG **************/
int bias_removal = VON_NEUMANN;
int output_format = KB_EMULATOR;
int baud_rate = 19200;
/*************************************/

unsigned int bins[BINS_SIZE];
int adc_pin = A0;
int button_pin = A1;
int led_pin = 13;
boolean initializing = true;

int kb_buff_index = 0;
boolean kb_buff_ready = false;
boolean kb_debug = false;
char kb_buff_raw[KB_RAW_LEN];
char kb_buff_b64[KB_B64_LEN];
int button_current = LOW;
int button_previous = LOW;

unsigned int calibration_counter = 0;


void setup() {
  pinMode(led_pin, OUTPUT);
  Serial.begin(baud_rate);
  for (int i = 0; i < BINS_SIZE; i++) {
    bins[i] = 0;
  }
}

void loop() {
  byte threshold;
  if (kb_buff_ready) {
    button_previous = button_current;
    button_current = digitalRead(button_pin);
    if (button_current == HIGH && button_previous == LOW) {
      Keyboard.begin();
      Keyboard.print(kb_buff_b64);
      Keyboard.end();
    }
  } else {
    int adc_value = analogRead(adc_pin);
    byte adc_byte = adc_value >> 2;
    if (calibration_counter >= CALIBRATION_SIZE) {
      threshold = findThreshold();
      // Keeping the button pressed during callibration enters debug mode
      // where we constantly generate new buffers and send them via keyboard
      // emulation (to enable sending the result to rngtest).
      if (initializing && output_format == KB_EMULATOR && digitalRead(button_pin) == HIGH) kb_debug = true;
      initializing = false;
    }
    if (initializing) {
      calibrate(adc_byte);
      calibration_counter++;
    } else {
      processInput(adc_byte, threshold);
    }
  }
}

void processInput(byte adc_byte, byte threshold) {
  boolean input_bool;
  input_bool = (adc_byte < threshold) ? 1 : 0;
  switch (bias_removal) {
    case VON_NEUMANN:
      vonNeumann(input_bool);
      break;
    case EXCLUSIVE_OR:
      exclusiveOr(input_bool);
      break;
    case NO_BIAS_REMOVAL:
      buildByte(input_bool);
      break;
  }
}

void exclusiveOr(byte input) {
  static boolean flip_flop = 0;
  flip_flop = !flip_flop;
  buildByte(flip_flop ^ input);
}

void vonNeumann(byte input) {
  static int count = 1;
  static boolean previous = 0;
  static boolean flip_flop = 0;

  flip_flop = !flip_flop;

  if (flip_flop) {
    if (input == 1 && previous == 0) {
      buildByte(0);
    }
    else if (input == 0 && previous == 1) {
      buildByte(1);
    }
  }
  previous = input;
}

void buildByte(boolean input) {
  static int byte_counter = 0;
  static byte out = 0;

  if (input == 1) {
    out = (out << 1) | 0x01;
  }
  else {
    out = (out << 1);
  }
  byte_counter++;
  byte_counter %= 8;
  if (byte_counter == 0) {
    blinkLed(10);
    if (output_format == ASCII_BYTE) Serial.println(out, DEC);
    if (output_format == BINARY) Serial.print(out, BIN);
    if (output_format == KB_EMULATOR) {
      kb_buff_raw[kb_buff_index++] = out;
      if (kb_buff_index >= KB_RAW_LEN) {
        base64_encode(kb_buff_b64, kb_buff_raw, KB_RAW_LEN);
        for (int i = 0; i < KB_B64_LEN; i++) {
          if (!kb_debug) {
            // make it url-safe
            if (kb_buff_b64[i] == '/') kb_buff_b64[i] = '_';
            if (kb_buff_b64[i] == '+') kb_buff_b64[i] = '-';
          }
        }
        if (kb_debug) {
          Keyboard.begin();
          Keyboard.println(kb_buff_b64);
          Keyboard.end();
          kb_buff_index = 0;
        } else {
          kb_buff_ready = true;
          digitalWrite(led_pin, HIGH);
        }
      }
    }
    out = 0;
  }
  if (output_format == ASCII_BOOL) Serial.print(input, DEC);
}


void calibrate(byte adc_byte) {
  bins[adc_byte]++;
  printStatus();
}

unsigned int findThreshold() {
  unsigned long half;
  unsigned long total = 0;
  int i;

  for (i = 0; i < BINS_SIZE; i++) {
    total += bins[i];
  }

  half = total >> 1;
  total = 0;
  for (i = 0; i < BINS_SIZE; i++) {
    total += bins[i];
    if (total > half) {
      break;
    }
  }
  return i;
}

//Blinks an LED after each 10th of the calibration completes
void printStatus() {
  unsigned int increment = CALIBRATION_SIZE / 10;
  static unsigned int num_increments = 0; //progress units so far
  unsigned int threshold;

  threshold = (num_increments + 1) * increment;
  if (calibration_counter > threshold) {
    num_increments++;
    blinkLed(30);
  }
}

void blinkLed(int duration) {
  digitalWrite(led_pin, HIGH);
  delay(duration);
  digitalWrite(led_pin, LOW);
}
