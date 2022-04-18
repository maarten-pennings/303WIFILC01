// clocktest.ino - tests the LED and switches on the board
// board: Generic ESP8266 module

#define D1_PIN 2

#define S1_PIN 0
#define S2_PIN 4
#define S3_PIN 15

#define getS1() (digitalRead(S1_PIN)==0) // low active
#define getS2() (digitalRead(S2_PIN)==0) // low active
#define getS3() (digitalRead(S3_PIN)!=0) // high active

void setup() {
  Serial.begin(115200);
  Serial.printf("\n\nclocktest.ino\n\n");

  // configure LEDs (D1)
  digitalWrite(D1_PIN, HIGH); // low active 
  pinMode(D1_PIN, OUTPUT);

  // Configure switches (S1, S2, S3)
  pinMode(S1_PIN, INPUT_PULLUP );
  pinMode(S2_PIN, INPUT_PULLUP );
  pinMode(S3_PIN, INPUT );
}

int count;
void loop() {
  digitalWrite(D1_PIN, LOW); // low active
  delay(200);
  digitalWrite(D1_PIN, HIGH); // low active 
  delay(800); 
  Serial.printf("%04d %d %d %d\n",count++,getS1(),getS2(),getS3());
}
