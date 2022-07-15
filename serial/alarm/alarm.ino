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
  //Serial.print("Hello, world!\n");
  c = Serial.read();
  if(c == 'A' || c == 'B' || c == 'C' || c == 'D' || c == 'E' || c == 'F' || c == 'X' || c == 'Y' || c == ' '){
    if (stat != c){
      if (c == 'X'){
        if (stat != ' ' && delay_time > 100){
          delay_time -= 80;
          stat += 1;
        }else if (stat == ' '){
          delay_time = 500;
          stat = 'A';
        }
      }else if(c == 'Y'){
        if (stat != ' ' && delay_time < 500){
          delay_time += 80;
          stat -= 1;
        }else if (stat == 'A'){
          noTone(buzzerPin);
          digitalWrite(redPin, LOW);
          stat = ' ';
        }
      }else if(c == ' '){
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
    tone(buzzerPin, 880, 100);
    delay(delay_time);
  }else{
    delay(500);
  }
}
