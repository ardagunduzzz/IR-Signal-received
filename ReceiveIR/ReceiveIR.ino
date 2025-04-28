#include <Arduino.h>

#include "PinDefinitionsAndMore.h" // Define macros for input and output pin etc.

#if !defined(RAW_BUFFER_LENGTH)
// For air condition remotes it requires 750. Default is 200.
#  if !((defined(RAMEND) && RAMEND <= 0x4FF) || (defined(RAMSIZE) && RAMSIZE < 0x4FF))
#define RAW_BUFFER_LENGTH  730 // this allows usage of 16 bit raw buffer, for RECORD_GAP_MICROS > 20000
#  endif
#endif

#define MARK_EXCESS_MICROS    20    // Adapt it to your IR receiver module. 20 is recommended for the cheap VS1838 modules.

//#define RECORD_GAP_MICROS 12000 
//#define DEBUG // Activate this for lots of lovely debug output from the decoders.

#include <IRremote.hpp>

// Kod sayacı - her alınan IR sinyali için artan bir numara
unsigned int codeNumber = 0;

//+=============================================================================
// Configure the Arduino
//
void setup() {
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(115200);   // Status message will be sent to PC at 115200 baud
    while (!Serial)
        ; // Wait for Serial to become available. Is optimized away for some cores.

#if defined(__AVR_ATmega32U4__) || defined(SERIAL_PORT_USBVIRTUAL) || defined(SERIAL_USB) /*stm32duino*/|| defined(USBCON) /*STM32_stm32*/ \
    || defined(SERIALUSB_PID)  || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_attiny3217)
    delay(4000); // To be able to connect Serial monitor after reset or power up and before first print out. Do not wait for an attached Serial Monitor!
#endif
    
    // Start the receiver and if not 3. parameter specified, take LED_BUILTIN pin from the internal boards definition as default feedback LED
    IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);

}

//+=============================================================================
// The repeating section of the code
//
void loop() {
    if (IrReceiver.decode()) {  // Grab an IR code
        // Kod sayacını artır
        codeNumber++;
        
        // Geçerli zamanı saniye cinsinden hesapla (Arduino açıldıktan sonra geçen süre)
        float currentTime = millis() / 1000.0;
        
        // Sadece istenen çıktı formatı
        Serial.print(F("@ "));
        Serial.print(currentTime, 2); // 2 ondalık basamakla zamanı yazdır
        Serial.print(F(", Code number: "));
        Serial.print(codeNumber);
        Serial.print(F(", Length: "));
        Serial.println(IrReceiver.decodedIRData.rawDataPtr->rawlen - 1);
        
        Serial.print(F("{"));
        
        for (uint16_t i = 1; i < IrReceiver.decodedIRData.rawDataPtr->rawlen; i++) {
            // MICROS_PER_TICK değeri ile çarparak mikrosaniye değerlerini elde et
            uint32_t duration = IrReceiver.decodedIRData.rawDataPtr->rawbuf[i] * MICROS_PER_TICK;
            
            Serial.print(duration);
            
            if (i < IrReceiver.decodedIRData.rawDataPtr->rawlen - 1) {
                Serial.print(F(", "));
            }
        }
        
        Serial.println(F("}"));
        Serial.println(); // Boş satır
        
        IrReceiver.resume();
    }
}