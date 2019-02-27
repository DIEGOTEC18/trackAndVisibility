void setup() {
  char input[] = "84698384";
  char temp[3];
  char c;
  int index;
  int i;
  Serial.begin(9600);
  
  for (i = 0; i < sizeof(input); i += 2) {
    temp[0] = input[i];
    temp[1] = input[i + 1];
    temp[2] = '\0';
    index = atoi(temp);
    c = toascii(index);
    Serial.print(c);
  }
}

void loop() {

}
