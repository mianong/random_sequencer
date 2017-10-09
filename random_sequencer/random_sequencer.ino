
/*
   random sequencer

   Create sequences, programmed or randomly generated

   output to midi ... in the future gate,cv, velocity(accent) x8?

   note (start and stop), velocity

  The 16x2 Display circuit:
    -
    -
  The Encoder circuit:
    -
    -
  The MIDI circuit:
    - digital in 1 connected to MIDI jack pin 5
    - MIDI jack pin 2 connected to ground
    - MIDI jack pin 4 connected to +5V through 220 ohm resistor
    - Attach a MIDI cable to the jack, then to a MIDI synth, and play music.

  TODO:
   add a keyboard with a multiplexer
   add a with a multiplexer

*/

#include "Display.h"
#include "Encoder.h"
#include <MIDI.h>

const int DEBUG = 0;
const int LED_TEMPO = 13;

const int CHANNELS= 2;
const int MAX_NUMBER_OF_STEPS = 16;

const int midiCommandNoteOff = 0x80;
const int midiCommandNoteOn = 0x90;
const int midiCommandKeyPressure = 0xA0;

int firstNote=1;

//
unsigned int currentStep = 0;

// variable to measure the last  a step
unsigned long lastStepTime = 0 ;

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

  https://www.midikits.net/midi_analyser/midi_note_numbers__octaves.htm

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
  Slendro is a pentatonic scale, About this sound Play (helpÂ·info) the older of the two most common scales (laras) used in Indonesian gamelan music, the other ...
  â€ŽTuning Â· â€ŽNote names Â· â€ŽConnotations Â· â€ŽOrigin
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

/* stepEnabled bit array to enable or disable a step */
int stepEnabled[8];
static char drum[26][26] = {"BD1","RIM","BD2","CLAP","BD3","SD1","CHH","SD2","PHH","SD3","OHH","SD4","TOM1","PERC1","TOM2","PERC2","TOM3","PERC3","CYM1","PERC4","CYM2","PERC5","CYM3","HIT","OTHER1","OTHER2"};

/*
   Grove settings is the percentage delay to apply to a step
*/
long lastRotaryCounter = 0;
int menuMode = 0;
int menuSelection = 0;

static char setting[][8] = {"mode", "tempo", "key", "scale", "drum1", "drum2", "drum3", "drum4"};

static char mode[][10] = {"off", "random", "ascending", "scale", "end", "other"};

int modeSelection = 0;
int modeType = 0;
static char key[][7] = {"C", "C#/Db", "D", "D#/Eb", "E", "F", "F#/Gb", "G", "G#/Ab", "A", "A#/Bb", "B"};
int keySelection = 0;

static char scale[][16] = {"chromatic", "major", "minor", "harmonic major", "harmonic minor", "hungarian", "pentatonic", "ditonic", "persian", "slendro", "pelog", "prometheus"};
int scaleSelection = 0;

/*
  Store all sequencer parameters into a noteArray
  Channel -0-15
  Step - 0-63?
  StepType = Note, Velocity, Length
    Notes = 0-127
    Velocity = 0-127
    Length = 0-127
*/
enum midiParam { NOTE, VELOCITY};

/* the first four channels are melodic the next four are drums */
unsigned int noteArray[CHANNELS][MAX_NUMBER_OF_STEPS][1];

/*
static int scales  [12][12] = {
  { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 } //0 chromatic
  , { 1, 2, 3, 3, 5, 6, 6, 8, 8, 10, 11, 12 } //1 major

};
*/
// tempo in beats per minute
unsigned int tempo = 112;


int swPin = 4;
MIDI_CREATE_DEFAULT_INSTANCE();

void setup() {

  setupDisplay();
  updateDisplay("starting up 1" , ".");
  delay(100);
  setupEncoder();
  updateDisplay("starting up 2" , "..");
  delay(100);

  // Setup the button
  pinMode(swPin , INPUT);
  updateDisplay("starting up 3" , "...");
  delay(100);
  // Activate internal pull-up (optional)
  digitalWrite( swPin , HIGH);
  updateDisplay("starting up 4" , "....");
  delay(100);

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  updateDisplay("starting up 5" , ".....");
  delay(100);
  setRandomSequence();
  updateDisplay("starting up 6" , "......");
  delay(100);
  MIDI.begin(MIDI_CHANNEL_OFF);
  updateDisplay("starting up 7" , ".......");
  delay(100);

  setDefaultDrumSettings();
  updateDisplay("starting up 8" , "........");
  delay(100);
  updateDisplay("starting up 9" , "done");
  delay(100);

}

void setDefaultDrumSettings() {
  for (int i=0;i<MAX_NUMBER_OF_STEPS;i++) {
      noteArray[4][i][NOTE]=64; //BD1
  //    noteArray[4][i][VELOCITY]=70;
      noteArray[5][i][NOTE]=77; //CHH
  //    noteArray[5][i][VELOCITY]=70;
      noteArray[6][i][NOTE]=69; //SND1
  ////    noteArray[6][i][VELOCITY]=70;
      noteArray[7][i][NOTE]=76; //TOM1
  //    noteArray[7][i][VELOCITY]=70;
  }
}

int getDrumNote(int channel) {
  int drumnote;    
  drumnote=  noteArray[channel][1][NOTE] -64;
  return drumnote;
}

void setDrumNote(int channel, int drumnote) {
  for (int i=0;i<MAX_NUMBER_OF_STEPS;i++) {
      noteArray[channel][i][NOTE]=drumnote+64; //BD1
//      noteArray[4][i][VELOCITY]=70;
  }
}

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

// makeup a random sequence
void setRandomSequence() {
  for (int channel = 0; channel < CHANNELS; channel++) {
    for (int istep = 0; istep < MAX_NUMBER_OF_STEPS; istep++) {
      noteArray[channel][istep][NOTE] = random(25, 68);
     // noteArray[channel][istep][VELOCITY] = random(70, 80);
    }
  }
}

// makeup a fith sequence
void setAscendingFifthSequence() {
  for (int channel = 0; channel < CHANNELS; channel++) {
    for (int istep = 0; istep < MAX_NUMBER_OF_STEPS; istep++) {
      noteArray[channel][istep][NOTE] = channel*7+istep+40;
   //   noteArray[channel][istep][VELOCITY] = 70;
    }
  }
}

unsigned int  editMode = 0;
int lastPush = 1;
void checkInputs() {
  if ( digitalRead(swPin) == 0 && editMode == 0 && lastPush == 1) {
    editMode = 1;
    lastPush = 0;
  }

  if ( digitalRead(swPin) == 0 && editMode == 1 && lastPush == 1) {
    editMode = 0;
    lastPush = 0;
    for (int c = 0;c<CHANNELS;c++)
      MIDI.sendControlChange(123,0,c);
    if (modeSelection==1) setRandomSequence();
    if (modeSelection==2) setAscendingFifthSequence();

  }

  if ( digitalRead(swPin) == 1) {
    lastPush = 1;
  }

  // Edit parameters
  if (editMode == 1) {
    switch (menuSelection) {
      case (0) :
        modeSelection = modeSelection + counterDirection;
        if (modeSelection>5) modeSelection =0;
        if (modeSelection<0)modeSelection=5;
        break;
      case (1) :
        tempo = tempo + counterDirection;
        if (tempo < 0) tempo = 0;
        break;
      case (2) :
        keySelection = keySelection + counterDirection;
        if (keySelection > 11) keySelection = 0;
        if (keySelection < 0) keySelection = 11;
        break;
      case (3) :
        scaleSelection = scaleSelection + counterDirection;
        if (scaleSelection > 11) scaleSelection = 0;
        if (scaleSelection < 0) scaleSelection = 11;
        break;
      case (4) :
      case (5) :
      case (6) :
      case (7) :
        int drumNote = getDrumNote(menuSelection);
        drumNote = drumNote + counterDirection;
        if (drumNote > 25) drumNote = 0;
        if (drumNote < 0) drumNote = 25;
        setDrumNote(menuSelection,drumNote);        
        break;
    }
  } else {
    menuSelection = menuSelection + counterDirection;
    if (menuSelection > 7) menuSelection = 0;
    if (menuSelection < 0) menuSelection = 7;
  }
  counterDirection = 0;

}

// the sequencer will play notes  the current beat  each channel
void playNotes() {
  for (int i = 0; i < CHANNELS; i++) {
  if (((stepEnabled[i] & ( 1 << currentStep )) >> currentStep)==1) {
    //NOTE, VELOCITY, LENGTH};
    //TODO: do something with note length. currently all notes end with each step
    int channel=1;
    if (i>3)channel=10;
//    MIDI.sendNoteOn(noteArray[i][currentStep][NOTE],noteArray[i][currentStep][VELOCITY],channel);  
      MIDI.sendNoteOn(noteArray[i][currentStep][NOTE],70,channel);  
    }                
  }
  firstNote=0;
}



void disableNotes() {
  //are there any notes to disable?
  if (firstNote==0) {
    for (int i = 0;  i < CHANNELS; i++) {
      //NOTE, VELOCITY, LENGTH}; /TODO do something with note length!!!
      int previousStep=currentStep-1;
      if (previousStep<0)  previousStep = MAX_NUMBER_OF_STEPS-1;
      int channel=1;
      if (i>3)channel=10;
      MIDI.sendNoteOff(noteArray[i][previousStep][NOTE],0,channel);  
    }
  }
}

/*
 * waitForNextStep
 * 
 * pausing the process between steps
 * convert tempo (in BPM) to steps
 * 
 * say each beat is a quarter note and each step is an sixteenth beat
 * 100 BPM = 400 Steps per Minute
 * the wait time between steps would be
 * 1/400 minutes or 60,000,000 / 400 microseconds or 15,000,000 microseconds
 * 60,000,000/(4*BPM)
 * 15,000,000/(BPM)
 */
boolean waitForNextStep() {  
  // TODO put in delays for groove settings
  // TODO micros() overflows ever 70 or so minutes.
  unsigned long timeSinceLastStep = micros() - lastStepTime;
  unsigned long delayTime = 0;
  unsigned long beatDelay = 15000000 / tempo;
  
  if (beatDelay > timeSinceLastStep) {
    delayTime = beatDelay - timeSinceLastStep;
  } else {
    delayTime = 0;
    lastStepTime = micros();
  }
  return (delayTime > 0);
}

//TODO write to the LCD screen
//  update with every ste.
void updateUI() {
  //TODO flash an LED to show each step?
  char line1[16];
  char line2[16];
  
  switch (menuSelection) {
    case (0) :
      sprintf(line2,"mode: %s",mode[modeSelection]);
      break;
    case (1) :
      sprintf(line2,"tempo: %d",tempo);
      break;
    case (2) :
      sprintf(line2,"key: %s",key[keySelection]);
      break;
    case (3) : 
      sprintf(line2,"%s",scale[scaleSelection]);
      break;
    case (4) :
    case (5) :
    case (6) :
    case (7) :
      sprintf(line2,"drum: %s", drum[getDrumNote(menuSelection)]);
      break;
  }
  if (editMode == 1 ) {
    sprintf(line1,"%s edit",setting[menuSelection]);
  } else {
    strcpy(line1,setting[menuSelection]);
  }
  updateDisplay(line1, line2);
  lastRotaryCounter = rotaryCounter;
}


void loop() {
  if (waitForNextStep() || modeSelection == 0) {
    checkInputs();
    updateUI();  
  } else {
    if (currentStep % MAX_NUMBER_OF_STEPS == 0);
    disableNotes();
    playNotes();
    currentStep++;
    if (currentStep >= MAX_NUMBER_OF_STEPS) currentStep = 0;
  }
}

