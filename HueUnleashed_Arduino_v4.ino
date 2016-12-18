#include <TimerOne.h>
#include <PacketSerial.h>
#include <Aldi_NeoPixel.h>

#define NUMBER_OF_LEDS 50
#define DEFAULT_TRANSITION_SPEED 20

#define SET_ALL_LED_COLOR_COMMAND 0x01
#define SET_SINGLE_LED_COLOR_COMMAND 0x02
#define SET_PART_LED_COLOR_COMMAND 0x03

//All Fade commands are deprecated because of their inefficient and blocking code
//Use the Transition commands instead
//These shouldn't work anymore
#define FADE_ALL_LED_COLOR_COMMAND 0x04
#define FADE_SINGLE_LED_COLOR_COMMAND 0x05
#define FADE_PART_LED_COLOR_COMMAND 0x06

#define GET_ALL_LED_COLOR_COMMAND 0x07
#define GET_SINGLE_LED_COLOR_COMMAND 0x08
#define GET_PART_LED_COLOR_COMMAND 0x09

#define SET_ALL_LED_COLOR_TRANSITION_COMMAND 0x0a
#define SET_SINGLE_LED_COLOR_TRANSITION_COMMAND 0x0b
#define SET_PART_LED_COLOR_TRANSITION_COMMAND 0x0c

#define SET_GAMMA_CORRECTION_STATE_COMMAND 0x0d

PacketSerial serial;

Aldi_NeoPixel ledStrip = Aldi_NeoPixel(NUMBER_OF_LEDS, 6, NEO_BRG + NEO_KHZ800);
uint8_t ledState[NUMBER_OF_LEDS][3];

uint8_t transitionTarget[NUMBER_OF_LEDS][3];
uint8_t transitionSteps[NUMBER_OF_LEDS];

bool colorFadeDone[3];
bool gammaCorrectionEnabled;

const uint8_t PROGMEM gamma[] = {
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
	1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  3,  3,  3,  3,  4,  4,
	4,  4,  5,  5,  5,  5,  6,  6,  6,  7,  7,  7,  8,  8,  8,  9,
	9,  9, 10, 10, 11, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16,
	16, 17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 23, 23, 24, 24,
	25, 26, 26, 27, 28, 28, 29, 30, 30, 31, 32, 32, 33, 34, 35, 35,
	36, 37, 38, 38, 39, 40, 41, 42, 42, 43, 44, 45, 46, 47, 47, 48,
	49, 50, 51, 52, 53, 54, 55, 56, 56, 57, 58, 59, 60, 61, 62, 63,
	64, 65, 66, 67, 68, 69, 70, 71, 73, 74, 75, 76, 77, 78, 79, 80,
	81, 82, 84, 85, 86, 87, 88, 89, 91, 92, 93, 94, 95, 97, 98, 99,
	100,102,103,104,105,107,108,109,111,112,113,115,116,117,119,120,
	121,123,124,126,127,128,130,131,133,134,136,137,139,140,142,143,
	145,146,148,149,151,152,154,155,157,158,160,162,163,165,166,168,
	170,171,173,175,176,178,180,181,183,185,186,188,190,192,193,195,
	197,199,200,202,204,206,207,209,211,213,215,217,218,220,222,224,
	226,228,230,232,233,235,237,239,241,243,245,247,249,251,253,255
};

void calculateTransitions()
{
	for(uint8_t ledIndex = 0; ledIndex < NUMBER_OF_LEDS; ledIndex++)
	{
		if (ledState[ledIndex][0] != transitionTarget[ledIndex][0]) {
			if (ledState[ledIndex][0] < transitionTarget[ledIndex][0] - transitionSteps[ledIndex])
				ledState[ledIndex][0] += transitionSteps[ledIndex];
			else if (ledState[ledIndex][0] > transitionTarget[ledIndex][0] + transitionSteps[ledIndex])
				ledState[ledIndex][0] -= transitionSteps[ledIndex];
			else
				ledState[ledIndex][0] = transitionTarget[ledIndex][0];
		}

		if (ledState[ledIndex][1] != transitionTarget[ledIndex][1]) {
			if (ledState[ledIndex][1] < transitionTarget[ledIndex][1] - transitionSteps[ledIndex])
				ledState[ledIndex][1] += transitionSteps[ledIndex];
			else if (ledState[ledIndex][1] > transitionTarget[ledIndex][1] + transitionSteps[ledIndex])
				ledState[ledIndex][1] -= transitionSteps[ledIndex];
			else
				ledState[ledIndex][1] = transitionTarget[ledIndex][1];
		}

		if (ledState[ledIndex][2] != transitionTarget[ledIndex][2]) {
			if (ledState[ledIndex][2] < transitionTarget[ledIndex][2] - transitionSteps[ledIndex])
				ledState[ledIndex][2] += transitionSteps[ledIndex];
			else if (ledState[ledIndex][2] > transitionTarget[ledIndex][2] + transitionSteps[ledIndex])
				ledState[ledIndex][2] -= transitionSteps[ledIndex];
			else
				ledState[ledIndex][2] = transitionTarget[ledIndex][2];
		}
	}
}
void applyAllPixelColors()
{
	calculateTransitions();

	for (uint8_t ledNumber = 0; ledNumber<NUMBER_OF_LEDS; ledNumber++)
	{
		if (gammaCorrectionEnabled)
			ledStrip.setPixelColor(ledNumber, pgm_read_byte(&gamma[ledState[ledNumber][0]]), pgm_read_byte(&gamma[ledState[ledNumber][1]]), pgm_read_byte(&gamma[ledState[ledNumber][2]]));
		else
			ledStrip.setPixelColor(ledNumber, ledState[ledNumber][0], ledState[ledNumber][1], ledState[ledNumber][2]);
	}
	ledStrip.show();
}
void fadeColor(uint8_t endR, uint8_t endG, uint8_t endB, uint8_t ledIndex, uint8_t fadeWaitSteps, bool enableFadeWaitSteps = true)
{
	if (abs(endR - ledState[ledIndex][0]) > 0) {
		if (ledState[ledIndex][0] < endR)
			ledState[ledIndex][0]++;
		else
			ledState[ledIndex][0]--;
	}
	else
	{
		colorFadeDone[0] = true;
	}

	if (abs(endG - ledState[ledIndex][1]) > 0) {
		if (ledState[ledIndex][1] < endG)
			ledState[ledIndex][1]++;
		else
			ledState[ledIndex][1]--;
	}
	else
	{
		colorFadeDone[1] = true;
	}

	if (abs(endB - ledState[ledIndex][2]) > 0) {
		if (ledState[ledIndex][2] < endB)
			ledState[ledIndex][2]++;
		else
			ledState[ledIndex][2]--;
	}
	else
	{
		colorFadeDone[2] = true;
	}

	if (enableFadeWaitSteps)
		for (uint32_t i = 0; i < fadeWaitSteps*8; i++)
			loop();
}
void sendPixelColor(uint8_t ledIndex, byte answerCommand)
{
	uint8_t answerArray[5] = { answerCommand, ledIndex, ledState[ledIndex][0], ledState[ledIndex][1], ledState[ledIndex][2] };
	serial.send(answerArray, 5);
}

void setAllPixelColor(uint8_t r, uint8_t g, uint8_t b)
{
	for (uint8_t ledNumber = 0; ledNumber < NUMBER_OF_LEDS; ledNumber++)
	{
		ledState[ledNumber][0] = r;	ledState[ledNumber][1] = g;	ledState[ledNumber][2] = b;
		transitionTarget[ledNumber][0] = r;	transitionTarget[ledNumber][1] = g;	transitionTarget[ledNumber][2] = b;
		transitionSteps[ledNumber] = DEFAULT_TRANSITION_SPEED;
	}
}
void setSinglePixelColor(uint8_t ledIndex, uint8_t r, uint8_t g, uint8_t b)
{
	ledState[ledIndex][0] = r;	ledState[ledIndex][1] = g;	ledState[ledIndex][2] = b;
	transitionTarget[ledIndex][0] = r;	transitionTarget[ledIndex][1] = g;	transitionTarget[ledIndex][2] = b;
	transitionSteps[ledIndex] = DEFAULT_TRANSITION_SPEED;
}
void setPartPixelColor(uint8_t startLedIndex, uint8_t endLedIndex, uint8_t r, uint8_t g, uint8_t b)
{
	for (uint8_t ledNumber = startLedIndex; ledNumber <= endLedIndex; ledNumber++)
	{
		ledState[ledNumber][0] = r;	ledState[ledNumber][1] = g;	ledState[ledNumber][2] = b;
		transitionTarget[ledNumber][0] = r;	transitionTarget[ledNumber][1] = g;	transitionTarget[ledNumber][2] = b;
		transitionSteps[ledNumber] = DEFAULT_TRANSITION_SPEED;
	}
}
void fadeAllPixelColor(uint8_t fadeWaitSteps, uint8_t endR, uint8_t endG, uint8_t endB)
{
	colorFadeDone[0] = false;	colorFadeDone[1] = false;	colorFadeDone[2] = false;

	while (colorFadeDone[0] == false || colorFadeDone[1] == false || colorFadeDone[2] == false)
	{
		for (uint8_t ledIndex = 0; ledIndex < NUMBER_OF_LEDS; ledIndex++) {
			fadeColor(endR, endG, endB, ledIndex, fadeWaitSteps, false);
		}

		for (uint32_t i = 0; i<fadeWaitSteps*8; i++)
		{
			loop();
		}
	}
}
void fadeSinglePixelColor(uint8_t ledIndex, uint8_t fadeWaitSteps, uint8_t endR, uint8_t endG, uint8_t endB)
{
	colorFadeDone[0] = false;	colorFadeDone[1] = false;	colorFadeDone[2] = false;

	while (colorFadeDone[0] == false || colorFadeDone[1] == false || colorFadeDone[2] == false)
	{
		fadeColor(endR, endG, endB, ledIndex, fadeWaitSteps);
	}
}
void fadePartColor(uint8_t startLedIndex, uint8_t endLedIndex, uint8_t fadeWaitSteps, uint8_t endR, uint8_t endG, uint8_t endB)
{
	colorFadeDone[0] = false;	colorFadeDone[1] = false;	colorFadeDone[2] = false;

	while (colorFadeDone[0] == false || colorFadeDone[1] == false || colorFadeDone[2] == false)
	{
		for (uint8_t ledIndex = startLedIndex; ledIndex < endLedIndex; ledIndex++) {
			fadeColor(endR, endG, endB, ledIndex, fadeWaitSteps, false);
		}

		for (uint32_t i = 0; i < fadeWaitSteps * 8; i++) {
			loop();
		}
	}
}
void getAllPixelColor()
{
	for (uint8_t ledIndex = 0; ledIndex < NUMBER_OF_LEDS; ledIndex++)
	{
		sendPixelColor(ledIndex, GET_ALL_LED_COLOR_COMMAND);
	}
}
void getSingelPixelColor(uint8_t ledIndex)
{
	sendPixelColor(ledIndex, GET_SINGLE_LED_COLOR_COMMAND);
}
void getPartColor(uint8_t startLedIndex, uint8_t endLedIndex)
{
	for (uint8_t ledIndex = startLedIndex; ledIndex < endLedIndex; ledIndex++)
	{
		sendPixelColor(ledIndex, GET_PART_LED_COLOR_COMMAND);
	}
}
void setAllPixelColorTransition(uint8_t transitionStepsArg, uint8_t r, uint8_t g, uint8_t b)
{
	for(uint8_t ledIndex = 0; ledIndex < NUMBER_OF_LEDS; ledIndex++)
	{
		transitionSteps[ledIndex] = transitionStepsArg;
		transitionTarget[ledIndex][0] = r;	transitionTarget[ledIndex][1] = g;	transitionTarget[ledIndex][2] = b;
	}
}
void setSinglePixelColorTransition(uint8_t transitionStepsArg, uint8_t ledIndex, uint8_t r, uint8_t g, uint8_t b)
{
	transitionSteps[ledIndex] = transitionStepsArg;
	transitionTarget[ledIndex][0] = r;	transitionTarget[ledIndex][1] = g;	transitionTarget[ledIndex][2] = b;
}
void setPartPixelColorTransition(uint8_t transitionStepsArg, uint8_t startLedIndex, uint8_t endLedIndex, uint8_t r, uint8_t g, uint8_t b)
{
	for(uint8_t ledIndex = startLedIndex; ledIndex <= endLedIndex; ledIndex++)
	{
		transitionSteps[ledIndex] = transitionStepsArg;
		transitionTarget[ledIndex][0] = r;	transitionTarget[ledIndex][1] = g;	transitionTarget[ledIndex][2] = b;
	}
}
void setGammaCorrectionState(bool newGammaCorrectionEnabled)
{
	gammaCorrectionEnabled = newGammaCorrectionEnabled;
}

void onSerialPacket(const uint8_t* buffer, size_t size)
{
	switch (buffer[0])
	{
		case SET_ALL_LED_COLOR_COMMAND:
			if (size == 4)
				setAllPixelColor(buffer[1], buffer[2], buffer[3]);
			break;
		case SET_SINGLE_LED_COLOR_COMMAND:
			if (size == 5)
				setSinglePixelColor(buffer[1], buffer[2], buffer[3], buffer[4]);
			break;
		case SET_PART_LED_COLOR_COMMAND:
			if (size == 6)
				setPartPixelColor(buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);
			break;


		case FADE_ALL_LED_COLOR_COMMAND:
			if (size == 5)
				fadeAllPixelColor(buffer[1], buffer[2], buffer[3], buffer[4]);
			break;
		case FADE_SINGLE_LED_COLOR_COMMAND:
			if (size == 6)
				fadeSinglePixelColor(buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);
			break;
		case FADE_PART_LED_COLOR_COMMAND:
			if (size == 7)
				fadePartColor(buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6]);
			break;


		case GET_ALL_LED_COLOR_COMMAND:
			if (size == 1)
				getAllPixelColor();
			break;
		case GET_SINGLE_LED_COLOR_COMMAND:
			if (size == 2)
				getSingelPixelColor(buffer[1]);
			break;
		case GET_PART_LED_COLOR_COMMAND:
			if (size == 3)
				getPartColor(buffer[1], buffer[2]);
			break;


		case SET_ALL_LED_COLOR_TRANSITION_COMMAND:
			if (size == 5)
				setAllPixelColorTransition(buffer[1], buffer[2], buffer[3], buffer[4]);
			break;
		case SET_SINGLE_LED_COLOR_TRANSITION_COMMAND:
			if(size == 6)
				setSinglePixelColorTransition(buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);
			break;
		case SET_PART_LED_COLOR_TRANSITION_COMMAND:
			if(size == 7)
				setPartPixelColorTransition(buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6]);
			break;


		case SET_GAMMA_CORRECTION_STATE_COMMAND:
			if (size == 2)
				setGammaCorrectionState(buffer[1]);
			break;
	}
}



void setup()
{
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, LOW);
	serial.setPacketHandler(&onSerialPacket);
	serial.begin(921600);

	for(uint8_t ledIndex=0; ledIndex<NUMBER_OF_LEDS; ledIndex++)
	{
		transitionSteps[ledIndex] = DEFAULT_TRANSITION_SPEED;
	}

	ledStrip.begin();

	Timer1.initialize(16666);
	Timer1.attachInterrupt(applyAllPixelColors);
}

void loop()
{
	serial.update();
}