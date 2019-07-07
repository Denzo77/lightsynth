#include <array>

#include <MIDI.h>
#include <Bounce.h>
#include "actuators.h"

// Setup USB MIDI if available.
#if defined(USBCON)
#include <midi_UsbTransport.h>

static const unsigned sUsbTransportBufferSize = 16;
typedef midi::UsbTransport<sUsbTransportBufferSize> UsbTransport;

UsbTransport sUsbTransport;

MIDI_CREATE_INSTANCE(UsbTransport, sUsbTransport, MIDI);

#else // No USB available, fallback to Serial
MIDI_CREATE_DEFAULT_INSTANCE();
#endif


// Default midi channel.
static constexpr auto defaultChannel {2};
// Button bounce time.
static constexpr auto bounceTime {5};

// LED Pins
static constexpr uint_fast8_t LED_RED {15};
static constexpr uint_fast8_t LED_GREEN {16};


//// Inputs

// Selector - Choose between 6 voices
// range of values [0:5[
constexpr auto selectorSwitchPositions {6};
const std::array<std::pair<Bounce, uint_fast8_t>, selectorSwitchPositions> selectorSwitch {
    std::make_pair(Bounce(2, bounceTime), 0),
    {Bounce(3, bounceTime), 1},
    {Bounce(4, bounceTime), 2},
    {Bounce(5, bounceTime), 3},
    {Bounce(6, bounceTime), 4},
    {Bounce(7, bounceTime), 5},
};
MidiSwitch<decltype(usbMIDI), usbMIDI, selectorSwitchPositions, true> selector(defaultChannel, 0, selectorSwitch);

// Auto Play button
MidiButton<decltype(usbMIDI), usbMIDI> autoPlay(defaultChannel, 1, 8, bounceTime);


// Arp on/off
MidiButton<decltype(usbMIDI), usbMIDI> arpOnOff(defaultChannel, 2, 9, bounceTime);

// Arp rate pot
MidiPot<decltype(usbMIDI), usbMIDI> arpRate(defaultChannel, 3, 9);

// Scale on/off
MidiButton<decltype(usbMIDI), usbMIDI> scaleOnOff(defaultChannel, 4, 10, bounceTime);



// Onset pot
MidiPot<decltype(usbMIDI), usbMIDI> onsetAmount(defaultChannel, 5, 7);

// Length pot
MidiPot<decltype(usbMIDI), usbMIDI> lengthAmount(defaultChannel, 6, 8);

// Sustain on/half/off
constexpr auto sustainAmountPositions {2};
const std::array<std::pair<Bounce, uint_fast8_t>, sustainAmountPositions> sustainAmount {
    std::make_pair(Bounce(11, bounceTime), 127),
    {Bounce(12, bounceTime), 0},
};
MidiSwitch<decltype(usbMIDI), usbMIDI, sustainAmountPositions, true, 63> sustain(defaultChannel, 7, sustainAmount);


// FX on/off
MidiButton<decltype(usbMIDI), usbMIDI> fxOnOff(defaultChannel, 8, 14, bounceTime);

// Reverb Dry/Wet pot
MidiPot<decltype(usbMIDI), usbMIDI> fxReverbDryWet(defaultChannel, 9, 6);

// Echo Dry/Wet pot
MidiPot<decltype(usbMIDI), usbMIDI> fxEchoDryWet(defaultChannel, 10, 5);

// FX Fade pot
MidiPot<decltype(usbMIDI), usbMIDI> fxFade(defaultChannel, 11, 4);

// FX Rate pot
MidiPot<decltype(usbMIDI), usbMIDI> fxRate(defaultChannel, 12, 3);


//// Outputs

// LED indicator
// MidiCC autoPlayLED(defaultChannel, 12);
constexpr uint8_t controlRedLED {20};
constexpr uint8_t controlGreenLED {21};

void controlChangeHandler(const byte channel, const byte control, const byte value)
{
    if (channel != defaultChannel) { return; }

    switch (control) {
        case (controlRedLED): { digitalWrite(LED_RED, (value != 0)); break; }
        case (controlGreenLED): { digitalWrite(LED_GREEN, (value != 0)); break; }
        default: break;
    }
}

void setup() {
    Serial.begin(115200);
    while (!Serial);
    MIDI.begin();
    Serial.println("Arduino ready.");

    for (auto i = 2; i != 15; ++i) { pinMode(i, INPUT_PULLUP); }

    pinMode(LED_RED, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);

    usbMIDI.setHandleControlChange(controlChangeHandler);
}

void loop() {
    selector.readAndSend();
    autoPlay.readAndSend();

    // if (autoPlay.read()) {
    //     digitalWrite(LED_RED, LOW);
    //     digitalWrite(LED_GREEN, HIGH);
    // } else {
    //     digitalWrite(LED_RED, HIGH);
    //     digitalWrite(LED_GREEN, LOW);        
    // }

    arpRate.readAndSend();
    arpOnOff.readAndSend();
    scaleOnOff.readAndSend();

    onsetAmount.readAndSend();
    lengthAmount.readAndSend();
    sustain.readAndSend();

    fxOnOff.readAndSend();
    fxReverbDryWet.readAndSend();
    fxEchoDryWet.readAndSend();
    fxFade.readAndSend();
    fxRate.readAndSend();

    while (usbMIDI.read()) {}
}
