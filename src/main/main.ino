void setup() {
  //start serial connection
  Serial.begin(9600);
  //pins 2, 3, 4, 5, 6, 7 correspond to columns for the holf with the mcu.
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);
  pinMode(7, INPUT_PULLUP);
  //pins 8, 9, 10 correspond to rows for the half with the mcu.
  pinMode(8, OUTPUT_OPENDRAIN);
  pinMode(9, OUTPUT_OPENDRAIN);
  pinMode(10, OUTPUT_OPENDRAIN);
}

const int ROW_PIN_START = 8;
const int ROW_PIN_END = 9;//10;
const int COL_PIN_START = 2;
const int COL_PIN_END = 3;//7;

typedef struct{
  //val == LOW means the key is pressed
  //val == HIGH means the key is pressed
  int val;
  int modifier;
  int normal_key;
} key_type;


key_type key_map[12*3*4] =
{
  //Left half, level 0
  {HIGH, 0, KEY_Q}, {HIGH, 0, KEY_W}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0},
  {HIGH, 0, KEY_A}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0},
  {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0},
  //Right half, level 0
  {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0},
  {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0},
  {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0},
  //Left half, level 1
  {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0},
  {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0},
  {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0},
  //Right half, level 1
  {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0},
  {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0},
  {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0},
  //Left half, level 2
  {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0},
  {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0},
  {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0},
  //Right half, level 2
  {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0},
  {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0},
  {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0},
  //Left half, level 3
  {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0},
  {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0},
  {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, {HIGH, 0, 0}, 
}

//fn levels 0, 1, 2, 3 (3 is only for left half)
int fn_l = 0;
int fn_r = 0;

unsigned int key_addr(int x, int y, int l){
  int offs = l*(12*3);
  
  return offs+12*y+x
}

key_type get_key_left(int row_pin, int col_pin, int fn_l){
  unsigned int addr = key_left_addr(
                        row_pin - ROW_PIN_START,
                        col_pin - COL_PIN_START,
                        fn_l);  
  return key_map[addr];
}

void set_key_val_left(int row_pin, int col_pin, int fn_l, int val){
  unsigned int addr = key_left_addr(
                        row_pin - ROW_PIN_START,
                        col_pin - COL_PIN_START,
                        fn_l);
  key_map[addr].val = val; 
}

int key_cnt = 0;
int modifier = 0;

void reset_usb_buff(){
  key_cnt = 0;
  modifier = 0;
  /*
  Keyboard.set_key1(0);
  Keyboard.set_key2(0);
  Keyboard.set_key3(0);
  Keyboard.set_key4(0);
  Keyboard.set_key5(0);
  Keyboard.set_key6(0);
  Keyboard.set_modifier(0);
  */
}

void set_usb(key_type k){
  Serial.println("SET");
  Serial.println(k.val);
  Serial.println(k.normal_key);
  
  //usb hid keyboard only allows 6 keypresses at once
  if (key_cnt >= 6)
    return;
  
  modifier |= k.modifier;
  //Keyboard.set_modifier(modifier);

  switch(key_cnt)
  {
    case 0:
      //Keyboard.set_key1(k.normal_key);
      break;
    case 1:
      //Keyboard.set_key2(k.normal_key);
      break;
    case 2:
      //Keyboard.set_key3(k.normal_key);
      break;
    case 3:
      //Keyboard.set_key4(k.normal_key);
      break;
    case 4:
      //Keyboard.set_key5(k.normal_key);
      break;
    case 5:
      //Keyboard.set_key6(k.normal_key);
      break;
    default:
      ;
  }

  key_cnt++;
}

void loop() {
  reset_usb_buff();
  
  int row_pin = ROW_PIN_START;
  for (;row_pin<=ROW_PIN_END;++row_pin){
    digitalWrite(9, HIGH);
    digitalWrite(10, HIGH);
    digitalWrite(11, HIGH);
    digitalWrite(row_pin, LOW);
    //wait for the current row to sink to low
    delayMicroseconds(50);
    
    int col_pin = COL_PIN_START;
    for (;col_pin<=COL_PIN_END;++col_pin){
      int key_val = digitalRead(col_pin);
      key_type k = get_key_left(row_pin, col_pin, fn_l);

      if (key_val != k.val){
        Serial.println("DIFF");
        set_key_val_left(row_pin, col_pin, fn_l, key_val);
        set_usb(k);
      }
      else{
        //nothing has changed, do nothing.
        ;
      }
    }
  }

  //Keyboard.send_now();
  delay(100);
}