// File M32_Test
//
// A Arduino program to test the operation of pressure sensors in the TE Connectivity M32 series (I2C versions only)
//
#include <Wire.h> // Arduino I2C library
#include <stdint.h> // Standard C, Allows explicit data type declaration.

//////////////////////////////////////////////////////////////////////////////////////
// M32 sensor characteristics (from the 4/2019 version of the data sheet)
//////////////////////////////////////////////////////////////////////////////////////

// M32 sensor I2C address (uncomment the Interface Type of the device you are using)
// Interface Type I
const uint8_t M32Address = 0x28;
// Interface Type J
//const uint8_t M32Address = 0x36;
// Interface Type K
//const uint8_t M32Address = 0x46;
// Interface Type 0
//const uint8_t M32Address = 0x48;

// M32 sensor full scale range and units
//const int16_t M32FullScaleRange = 1; // 1 psi
const int16_t M32FullScaleRange = 6.89476; // 100 psi in bar
//const int16_t M32FullScaleRange = 0.0689476; // 1 psi in Bar
//const int16_t M32FullScaleRange = 6895; // 1 psi in Pascal
//const int16_t M32FullScaleRange = 2; // 2 psi
//const int16_t M32FullScaleRange = 5; // 5 psi

// M32 Sensor type (A or B) comment out the wrong type assignments
// Type A (10% to 90%)
//const int16_t M32MinScaleCounts = 1638;
//const int16_t M32FullScaleCounts = 14746;
// Type B (5% to 95%)
//const int16_t M32MinScaleCounts = 819;
//const int16_t M32FullScaleCounts = 15563;
// M32 ??
const int16_t M32MinScaleCounts = 1000;
const int16_t M32FullScaleCounts = 15000;
const int16_t M32Span=M32FullScaleCounts-M32MinScaleCounts;

//M32 sensor pressure style, differential or otherwise. Comment out the wrong one.
//Differential
//const int16_t M32ZeroCounts=(M32MinScaleCounts+M32FullScaleCounts)/2;
// Absolute, Gauge
const int16_t M32ZeroCounts=M32MinScaleCounts;

//////////////////////////////////////////////////////////////////////////////////////
// end of M32 sensor characteristics
//////////////////////////////////////////////////////////////////////////////////////


void printBinary(int number, uint8_t Length){
  static int bits;
  // debug
  /*Serial.flush();
  Serial.println("");
  Serial.print("starting loop: ");
  Serial.println(number);
  Serial.print("bits = ");
  Serial.println(bits);*/
  if ( bits > 64 ) {
    // bits cant reach 64
    Serial.print(F("64 loops done, failed somewhere. Debug: bits:"));
    Serial.print(bits);
    Serial.print(F(" lenght: "));
    Serial.print(Length);
    Serial.print(F(" number: "));
    Serial.println(number);
    bits=0;
    number=0;
    return;
  }
  Serial.flush();
  if (number) { //The remaining bits have a value greater than zero continue
    bits++; // Count the number of bits so we know how many leading zeros to print first
    printBinary(number >> 1, Length); // Remove a bit and do a recursive call this function.
    if (bits) for (uint8_t x = (Length - bits); x; x--)Serial.write('0'); // Add the leading zeros
    bits = 0; // clear no need for this any more
    // debug
    //Serial.flush();
    Serial.write((number & 1) ? '1' : '0'); // print the bits in reverse order as we depart the recursive function
  }
}


char* tempToAscii(float temp)
// convert long to type char and store in variable array ascii
{
  char ascii[20];// needs to be this big to hold a type float

  int frac;
  int rnd;

    rnd = (unsigned int)(temp*1000)%10;
    frac=(unsigned int)(temp*100)%100;  //get three numbers to the right of the deciaml point.
    if (rnd>=5) frac=frac+1;
    itoa((int)temp,ascii,10);           // I changed it to 2 decimal places
    strcat(ascii,".");

  if (frac<10)  {itoa(0,&ascii[strlen(ascii)],10); }   // if fract < 10 should print .0 fract ie if fract=6 print .06

    itoa(frac,&ascii[strlen(ascii)],10); //put the frac after the deciaml
    
  return ascii;
}

// fetch_i2cdata is a function to do the I2C read and extraction of the three data fields
//
byte fetch_i2cdata(double &pressure, double &temperature)
{
byte _status;
byte Press_H;
byte Press_L;
byte Temp_H;
byte Temp_L;

uint16_t P_dat; // 14 bit pressure data
uint16_t T_dat; // 11 bit temperature data

Wire.requestFrom(M32Address, static_cast<uint8_t>(4), static_cast<uint8_t>(true)); //Request 4 bytes, 2 pressure/status and 2 temperature
Press_H = Wire.read();
Press_L = Wire.read();
Temp_H = Wire.read();
Temp_L = Wire.read();

// print raw data
printBinary(Press_H, 8);
Serial.println("\n");
printBinary(Press_L, 8);
Serial.println("\n");
printBinary(Temp_H, 8);
Serial.println("\n");
printBinary(Temp_L, 8);
Serial.println("\n");

_status = (Press_H >> 6) & 0x03;
Press_H = Press_H & 0x3f;
P_dat = (((uint16_t)Press_H) << 8 ) | Press_L; // space added after ‘8’ so code can be uploaded without emoji conversion!

// the low part of the temperature must remove the last 5 bits
Temp_L = (Temp_L >> 5);
// the correct temperature must take the high part, adding 3 bit on the end, and then the remaining 3 bit on Temp_L
T_dat = (((uint16_t)Temp_H) << 3) | Temp_L;

// debug
Serial.print("pressure (binary): ");
printBinary(P_dat, 14);
Serial.println("\n");

Serial.print("pressure (dec): ");
Serial.print(P_dat);
Serial.println("\n");

Serial.print("temperature (binary): ");
printBinary(T_dat, 11);
Serial.println("\n");


Serial.print("temperature (dec): ");
Serial.print(T_dat);
Serial.println("\n");

pressure=(static_cast<double>(static_cast<int16_t>(P_dat)-M32ZeroCounts))/static_cast<int16_t>(M32Span)*static_cast<int16_t>(M32FullScaleRange);

// output (decimal counts) = ( output °C + 50°C ) * 2048 / (150°C - (-50°C))
/* output °C	Digital counts (Dec)	Digital counts
   0		 512			0x200
  10		 614			0x266
  25		 767			0x2FF
  40		 921			0x399
  55		1075			0x433 */
//temperature=(T_dat+50)*2048/200;
// reversed formula from datasheet as we have count and want temperature
temperature = ( static_cast<int16_t>(T_dat) * 200 / 2048 ) - 50;
temperature = T_dat + 50;
Serial.println(T_dat);
temperature = (double)798*200;
Serial.println(temperature);

temperature = 798*200/2048;
Serial.println(temperature);


return _status;

}

// setup is the main function to setup the diagnostic serial port and the I2C port
void setup()
{

Serial.begin(115200);
Wire.begin();
// wait until serial port opens for native USB devices
while (! Serial)
{
delay(1);
}

Serial.println("M32 test");

}

void loop()
{

byte _status; // A two bit field indicating the status of the I2C read

double pressure;
double temperature;

_status = fetch_i2cdata(pressure, temperature);

switch (_status)
{
case 0:
//Serial.println("Ok ");
break;
case 1:
Serial.println("Busy");
break;
case 2:
Serial.println("Slate");
break;
default:
Serial.println("Error");
break;
}

Serial.print("pressure:");
Serial.println(pressure);

Serial.print("°C:");
Serial.println(temperature);

delay(1000);

}
