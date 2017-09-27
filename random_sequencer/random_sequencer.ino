/*
   random sequencer

   Create sequences, programmed or randomly generated

   output to midi / cv?

   note (start and stop), velocity


  If this circuit is connected to a MIDI synth, it will play the notes
  F#-0 (0x1E) to F#-5 (0x5A) in sequence.

  The circuit:
  - digital in 1 connected to MIDI jack pin 5
  - MIDI jack pin 2 connected to ground
  - MIDI jack pin 4 connected to +5V through 220 ohm resistor
  - Attach a MIDI cable to the jack, then to a MIDI synth, and play music.


   TODO:

   a keyboard with a multiplexer

*/

const int LED_TEMPO=13;

const int channels = 16;

const int midiCommandNoteOff = 0x80;
const int midiCommandNoteOn = 0x90;
const int midiCommandKeyPressure = 0xA0;

// 
unsigned int currentStep = 0; 
unsigned int maxNumberOfSteps = 64; 
// variable to measure the last for a step
unsigned long lastStepTime =0 ;
/*
  Value (decimal)  Value (Hex) Command Data bytes
  128-143 80-8F Note off  2 (note, velocity)
  144-159 90-9F Note on 2 (note, velocity)
  160-175 A0-AF Key Pressure  2 (note, key pressure)
  176-191 B0-BF Control Change  2 (controller no., value)
  192-207 C0-CF Program Change  1 (program no.)
  208-223 D0-DF Channel Pressure  1 (pressure)
  224-239 E0-EF Pitch Bend  2 (least significant byte, most significant byte)
*/

/*

  https://www.midikits.net/midi_analyser/midi_note_numbers_for_octaves.htm

  Octave  Note Numbers
  C   C#  D   D#  E   F   F#  G   G#  A   A#  B
  -1  0   1   2   3   4   5   6   7   8   9   10  11
  0   12  13  14  15  16  17  18  19  20  21  22  23
  1   24  25  26  27  28  29  30  31  32  33  34  35
  2   36  37  38  39  40  41  42  43  44  45  46  47
  3   48  49  50  51  52  53  54  55  56  57  58  59
  4   60  61  62  63  64  65  66  67  68  69  70  71
  5   72  73  74  75  76  77  78  79  80  81  82  83
  6   84  85  86  87  88  89  90  91  92  93  94  95
  7   96  97  98  99  100 101 102 103 104 105 106 107
  8   108 109 110 111 112 113 114 115 116 117 118 119
  9   120 121 122 123 124 125 126 127
*/
/*
  https://en.wikipedia.org/wiki/Persian_scale

  Slendro - Wikipedia
  https://en.wikipedia.org/wiki/Slendro
  Slendro is a pentatonic scale, About this sound Play (help·info) the older of the two most common scales (laras) used in Indonesian gamelan music, the other ...
  ‎Tuning · ‎Note names · ‎Connotations · ‎Origin
  Pelog - Wikipedia
  https://en.wikipedia.org/wiki/Pelog
  Pelog is one of the two essential scales of gamelan music native to Bali and Java, in Indonesia. In Javanese the term is said to be a variant of the word pelag meaning "fine" or "beautiful". The other, older, scale commonly used is called slendro.
  Prometheus scale/Mystic chord - Wikipedia
  https://en.wikipedia.org/wiki/Mystic_chord
  The Mystic is an example of a synthetic chord, and the scale from which it derives its notes, sometimes called the Prometheus scale, is an example of a synthetic scale.

  Locrian
  Ionian
  Neapolitan major
  Neapolitan minor
  Hungarian major
  Hungarian minor
  Lydian
  Mixolydian
  Dorian
  Aeolian
  Phrygian

  maybe hemitonia and tritonia of the perfect-fifth projection??
*/


/*
 * Grove settings is the percentage delay to apply to a step
 */
int grooveSettingStep[16] = {0,0,0,0
                            ,0,0,0,0
                            ,0,0,0,0
                            ,0,0,0,0};

static char scaleNames[16][16] = {"chromatic", "major", "minor", "harmonic major", "harmonic minor", "hungarian", "pentatonic", "ditonic", "persian", "slendro", "pelog", "prometheus"};

int scales  [12][12] = {
  { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 } //0 chromatic
  , { 1, 2, 3, 3, 5, 6, 6, 8, 8, 10, 11, 12 } //1 major

};

// tempo in beats per minute
unsigned int tempo = 120;

/*
Store all sequencer parameters into a noteArray
  Channel -0-15
  Step - 0-63?
  StepType = Note, Velocity, Length
    Notes = 0-127
    Velocity = 0-127
    Length = 0-127
*/
enum midiParam { NOTE, VELOCITY, LENGTH};

int  noteArray[8][64][4];

void setup() {
  // Set MIDI baud rate:
  Serial.begin(31250);

}


char trackLabels[8][32]  = {
  "synth_voice1"
  , "synth_voice2"
  , "synth_voice3"
  , "synth_voice4"
  , "synth_voice8"
  , "synth_voice6"
  , "synth_voice7"
  , "synth_voice8"
};

/*
   seq/rand//
*/

enum applicationMode { STARTUP, STEP, EDIT, PLAY, RECORD};
void setMode() {
  //Edit modes
  // Edit

  //  random
  //      range
  //
  //  input
  //  base {C-B}
  //  scale


  //Play mode
}

// TODO read tempo from analog in
void setTempo() {
  // set tempo
}

// TODO read tempo from analog in
void setScale() {

}

void checkInputs() {

}

// the sequencer will play notes for the current beat for each channel
void playNotes() {
  for (int i; i++; i < channels) {
    //NOTE, VELOCITY, LENGTH}; 
    //TODO: do something with note length. currently all notes end with each step
    Serial.write(i | midiCommandNoteOn); //bitwise OR
    Serial.write(noteArray[i][currentStep][NOTE]);
    Serial.write(noteArray[i][currentStep][VELOCITY]);
  }
}



void disableNotes() {
  //are there any notes to disable?
  for (int i; i++; i < channels) {
    //NOTE, VELOCITY, LENGTH}; /TODO do something with note length!!!
    Serial.write(i| midiCommandNoteOff); //bitwise OR
    Serial.write(noteArray[i][currentStep][NOTE]);
    Serial.write(noteArray[i][currentStep][VELOCITY]);
  }
}

// pausing the process between steps
void waitForNextStep(){
    // TODO put in delays for groove settings
    // TODO micros() overflows ever 70 or so minutes.
    unsigned long timeSinceLastStep = micros()-lastStepTime;
    //convert tempo (in BPM) to steps
    // say each beat is a quarter note and each step is an sixteenth beat
    // 100 BPM = 400 Steps per Minute
    // the wait time between steps would be 
    // 1/400 minutes or 60,000,000 / 400 microseconds or 150,000 microseconds
    // 60,000,0000/(4*BPM)
    // 15,000,0000/(BPM)
    
    delayMicroseconds((150000000/tempo)-timeSinceLastStep);
    lastStepTime=micros();
}

//TODO write to the LCD screen 
//  update with every ste

void updateUI() {
  //TODO flash an LED to show each step?
  digitalWrite(LED_TEMPO, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(10);
  digitalWrite(LED_TEMPO, LOW);
}

void loop() {
  
  checkInputs();

  updateUI();

  disableNotes();

  playNotes();

  
  currentStep++;
  if (currentStep > maxNumberOfSteps) currentStep = 0;

  waitForNextStep();
}
