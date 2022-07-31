const int redPin = 5;
const int yellowPin = 6;
const int greenPin = 7;
const int buzzerPin = 10;

char c;
char stat = ' ';
int delay_time = 500;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(redPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  c = Serial.read();
  if(c == 'A' || c == 'B' || c == 'C' || c == 'D' || c == 'E' || c == 'F' || c == ' '){
    if (stat != c){
      if(c == ' '){
        stat = c;
        noTone(buzzerPin);
        digitalWrite(redPin, LOW);
      }else{
        stat = c;
        delay_time = 500 - (c - 'A') * 80;
        digitalWrite(redPin, HIGH);
      }
    }
  }
  if (stat != ' '){
    tone(buzzerPin, 880, 99);
    delay(delay_time);
  }else{
    delay(500);
  }
}
